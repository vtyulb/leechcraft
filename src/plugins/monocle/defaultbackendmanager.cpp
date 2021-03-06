/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "defaultbackendmanager.h"
#include <memory>
#include <numeric>
#include <QStandardItemModel>
#include <QSettings>
#include <QApplication>
#include <interfaces/iinfo.h>
#include <interfaces/core/ipluginsmanager.h>
#include "choosebackenddialog.h"
#include "core.h"

namespace LeechCraft
{
namespace Monocle
{
	DefaultBackendManager::DefaultBackendManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Backends")) << tr ("Choice"));
	}

	void DefaultBackendManager::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		settings.beginGroup ("BackendChoices");
		Q_FOREACH (const auto& key, settings.childKeys ())
		{
			const auto& utf8key = key.toUtf8 ();
			AddToModel (utf8key, settings.value (utf8key).toByteArray ());
		}
		settings.endGroup ();
	}

	QAbstractItemModel* DefaultBackendManager::GetModel () const
	{
		return Model_;
	}

	QObject* DefaultBackendManager::GetBackend (const QList<QObject*>& loaders)
	{
		QList<QByteArray> ids;
		Q_FOREACH (auto backend, loaders)
			ids << qobject_cast<IInfo*> (backend)->GetUniqueID ();
		std::sort (ids.begin (), ids.end ());
		const auto& key = std::accumulate (ids.begin (), ids.end (), QByteArray (),
				[] (const QByteArray& left, const QByteArray& right)
					{ return left + '|' + right; });

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		settings.beginGroup ("BackendChoices");
		std::shared_ptr<void> guard (static_cast<void*> (0),
				[&settings] (void*) { settings.endGroup (); });

		if (ids.contains (settings.value (key).toByteArray ()))
		{
			const auto& id = settings.value (key).toByteArray ();
			Q_FOREACH (auto backend, loaders)
				if (qobject_cast<IInfo*> (backend)->GetUniqueID () == id)
					return backend;
			return 0;
		}

		ChooseBackendDialog dia (loaders);
		if (dia.exec () != QDialog::Accepted)
			return 0;

		auto backend = dia.GetSelectedBackend ();
		if (dia.GetRememberChoice ())
		{
			const auto& selectedId = qobject_cast<IInfo*> (backend)->GetUniqueID ();
			settings.setValue (key, selectedId);
			AddToModel (key, selectedId);
		}

		return backend;
	}

	void DefaultBackendManager::AddToModel (const QByteArray& key, const QByteArray& choice)
	{
		QList<QByteArray> set = key.split ('|');
		set.removeAll (QByteArray ());

		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		auto getName = [pm] (const QByteArray& id) -> QString
		{
			auto plugin = pm->GetPluginByID (id);
			return plugin ? qobject_cast<IInfo*> (plugin)->GetName () : QString ();
		};
		QStringList names;
		std::transform (set.begin (), set.end (), std::back_inserter (names), getName);

		QList<QStandardItem*> row;
		row << new QStandardItem (names.join ("; "));
		row << new QStandardItem (getName (choice));
		Model_->appendRow (row);

		row.first ()->setData (key, Roles::KeyID);
	}

	void DefaultBackendManager::removeRequested (const QString&, const QModelIndexList& indices)
	{
		QList<QPersistentModelIndex> pidxs;
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		settings.beginGroup ("BackendChoices");
		Q_FOREACH (const auto& idx, indices)
		{
			settings.remove (idx.sibling (idx.row (), 0).data (Roles::KeyID).toByteArray ());
			pidxs << idx;
		}
		settings.endGroup ();

		Q_FOREACH (const auto& pidx, pidxs)
			Model_->removeRow (pidx.row ());
	}
}
}
