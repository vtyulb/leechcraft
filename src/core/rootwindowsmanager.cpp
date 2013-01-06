/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "rootwindowsmanager.h"
#include <iterator>
#include <algorithm>
#include "core.h"
#include "mainwindow.h"
#include "mwproxy.h"
#include "tabmanager.h"
#include "dockmanager.h"
#include <interfaces/ihavetabs.h>

namespace LeechCraft
{
	RootWindowsManager::RootWindowsManager (QObject *parent)
	: QObject (parent)
	{
	}

	void RootWindowsManager::Release ()
	{
		for (const auto& win : Windows_)
			win.Window_->handleQuit ();
	}

	MainWindow* RootWindowsManager::MakeMainWindow ()
	{
		return CreateWindow ();
	}

	TabManager* RootWindowsManager::GetTabManager (MainWindow *win) const
	{
		return GetTabManager (GetWindowIndex (win));
	}

	TabManager* RootWindowsManager::GetTabManager (int index) const
	{
		return Windows_.value (index).TM_;
	}

	bool RootWindowsManager::WindowCloseRequested (MainWindow *win)
	{
		if (Windows_ [0].Window_ == win)
			return false;

		const int index = GetWindowIndex (win);
		const auto& data = Windows_ [index];
		for (int i = data.TM_->GetWidgetCount () - 1; i >= 0; --i)
			qobject_cast<ITabWidget*> (data.TM_->GetWidget (i))->Remove ();

		emit windowRemoved (index);

		Windows_.removeAt (index);

		win->deleteLater ();

		return true;
	}

	QObject* RootWindowsManager::GetObject ()
	{
		return this;
	}

	int RootWindowsManager::GetWindowsCount () const
	{
		return Windows_.size ();
	}

	int RootWindowsManager::GetPreferredWindowIndex () const
	{
		const auto active = QApplication::activeWindow ();
		if (!active)
			return 0;

		for (int i = 0; i < GetWindowsCount (); ++i)
			if (Windows_ [i].Window_ == active)
				return i;

		return 0;
	}

	int RootWindowsManager::GetWindowForTab (ITabWidget *tab) const
	{
		for (int i = 0; i < GetWindowsCount (); ++i)
		{
			const auto tw = Windows_ [i].Window_->GetTabWidget ();
			if (tw->IndexOf (dynamic_cast<QWidget*> (tab)) >= 0)
				return i;
		}

		return -1;
	}

	int RootWindowsManager::GetWindowIndex (QMainWindow *w) const
	{
		auto pos = std::find_if (Windows_.begin (), Windows_.end (),
				[w] (decltype (Windows_.at (0)) item) { return item.Window_ == w; });
		return pos == Windows_.end () ? -1 : std::distance (Windows_.begin (), pos);
	}

	QMainWindow* RootWindowsManager::GetMainWindow (int index) const
	{
		return Windows_ [index].Window_;
	}

	IMWProxy* RootWindowsManager::GetMWProxy (int index) const
	{
		return Windows_ [index].Proxy_;
	}

	ICoreTabWidget* RootWindowsManager::GetTabWidget (int index) const
	{
		return Windows_ [index].Window_->GetTabWidget ();
	}

	MainWindow* RootWindowsManager::CreateWindow ()
	{
		auto win = new MainWindow;
		auto proxy = new MWProxy (win);
		auto tm = new TabManager (win->GetTabWidget (), win, win->GetTabWidget ());

		connect (tm,
				SIGNAL (currentTabChanged (QWidget*)),
				Core::Instance ().GetDockManager (),
				SLOT (handleTabChanged (QWidget*)));

		Windows_.push_back ({ win, proxy, tm });
		win->Init ();

		emit windowAdded (Windows_.size () - 1);

		return win;
	}

	void RootWindowsManager::PerformWithTab (std::function<void (TabManager*, int)> f, QWidget *w)
	{
		const int idx = GetWindowForTab (qobject_cast<ITabWidget*> (w));
		if (idx < 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "no window for tab"
					<< w;
			return;
		}

		f (Windows_ [idx].TM_, idx);
	}

	void RootWindowsManager::MoveTab (int tabIdx, int fromWin, int toWin)
	{
		auto widget = Windows_ [fromWin].TM_->GetWidget (tabIdx);
		const auto& name = Windows_ [fromWin].Window_->GetTabWidget ()->TabText (tabIdx);

		emit tabIsMoving (fromWin, toWin, tabIdx);

		Windows_ [fromWin].TM_->remove (widget);
		Windows_ [toWin].TM_->add (name, widget);

		emit tabMoved (fromWin, toWin, Windows_ [toWin].TM_->FindTabForWidget (widget));
	}

	void RootWindowsManager::moveTabToNewWindow ()
	{
		CreateWindow ();

		MoveTab (sender ()->property ("TabIndex").toInt (),
				sender ()->property ("FromWindowIndex").toInt (),
				GetWindowsCount () - 1);
	}

	void RootWindowsManager::moveTabToExistingWindow ()
	{
		MoveTab (sender ()->property ("TabIndex").toInt (),
				sender ()->property ("FromWindowIndex").toInt (),
				sender ()->property ("ToWindowIndex").toInt ());
	}

	void RootWindowsManager::add (const QString& name, QWidget *w)
	{
		const int winIdx = GetPreferredWindowIndex ();

		auto itw = qobject_cast<ITabWidget*> (w);
		const int oldWinIdx = GetWindowForTab (itw);
		if (oldWinIdx >= 0 && oldWinIdx != winIdx)
		{
			const auto& oldData = Windows_ [oldWinIdx];
			emit tabIsRemoving (winIdx, oldData.Window_->GetTabWidget ()->IndexOf (w));
			oldData.TM_->remove (w);
		}

		Windows_ [winIdx].TM_->add (name, w);
		emit tabAdded (winIdx, Windows_ [winIdx].Window_->GetTabWidget ()->IndexOf (w));
	}

	void RootWindowsManager::remove (QWidget *w)
	{
		PerformWithTab ([this, w] (TabManager *tm, int winIdx)
			{
				emit tabIsRemoving (winIdx, tm->FindTabForWidget (w));
				tm->remove (w);
			}, w);
	}

	void RootWindowsManager::changeTabName (QWidget *w, const QString& name)
	{
		PerformWithTab ([w, &name] (TabManager *tm, int) { tm->changeTabName (w, name); }, w);
	}

	void RootWindowsManager::changeTabIcon (QWidget *w, const QIcon& icon)
	{
		PerformWithTab ([w, &icon] (TabManager *tm, int) { tm->changeTabIcon (w, icon); }, w);
	}

	void RootWindowsManager::bringToFront (QWidget *w)
	{
		PerformWithTab ([w] (TabManager *tm, int) { tm->bringToFront (w); }, w);
	}
}