/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "profilewidget.h"
#include <stdexcept>
#include <QtDebug>
#include <QMessageBox>
#include <util/util.h>
#include "ljprofile.h"
#include "ljaccount.h"
#include "ljfriendentry.h"
#include "frienditemdelegate.h"
#include "xmlsettingsmanager.h"
#include "addeditentrydialog.h"
#include "friendsmodel.h"
#include "friendsproxymodel.h"
#include "core.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	ProfileWidget::ProfileWidget (LJProfile *profile, QWidget *parent)
	: QWidget (parent)
	, Profile_ (profile)
	, FriendsModel_ (new FriendsModel (this))
	, FriendsProxyModel_(new FriendsProxyModel (this))
	, CommunitiesModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		FriendsProxyModel_->setSourceModel (FriendsModel_);
		Ui_.FriendsView_->setModel (FriendsProxyModel_);
		Ui_.FriendsView_->setDropIndicatorShown (true);
		FriendsModel_->setHorizontalHeaderLabels ({ tr ("UserName"),
				tr ("Status"), tr ("FullName"), tr ("Birthday") });
		CommunitiesModel_->setHorizontalHeaderLabels ({ tr ("Name") });
		
		connect (FriendsModel_,
				SIGNAL (userGroupChanged (const QString&, const QString&,
						const QString&, int)),
				this,
				SLOT (handleUserGroupChanged (const QString&, const QString&,
						const QString&, int)));

		FriendItemDelegate *friendDelegate = new FriendItemDelegate (Ui_.FriendsView_);
		connect (this,
				SIGNAL (coloringItemChanged ()),
				friendDelegate,
				SLOT (handleColoringItemChanged ()));
		Ui_.FriendsView_->setItemDelegate (friendDelegate);

		Ui_.CommunitiesView_->setModel (CommunitiesModel_);

		Ui_.ColoringFriendsList_->setChecked (XmlSettingsManager::Instance ()
				.Property ("ColoringFriendsList", true).toBool ());

		updateProfile ();
		
		connect (Ui_.Filter_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (handleFriendFilterTextChanged (QString)));
	}

	void ProfileWidget::RereadProfileData ()
	{
		//TODO
		const LJProfileData& data = Profile_->GetProfileData ();

		Ui_.JournalName_->setText (data.FullName_);
		IAccount *acc = qobject_cast<IAccount*> (Profile_->GetParentAccount ());
		const QString& path = Util::CreateIfNotExists ("blogique/metida/avatars")
				.absoluteFilePath (acc->GetAccountID ().toBase64 ().replace ('/', '_'));
		Ui_.JournalPic_->setPixmap (QPixmap (path));
		ReFillModels ();
	}

	void ProfileWidget::FillFriends (const QList<LJFriendEntry_ptr>& friends)
	{
		for (auto fr : friends)
		{
			
			QStandardItem *item = new QStandardItem (fr->GetUserName ());
			QStandardItem *itemName = new QStandardItem (fr->GetFullName ());

			QIcon icon;
			FriendsModel::FriendStatus status = FriendsModel::FSFriendOf;
			if (fr->GetFriendOf () &&
					fr->GetMyFriend ())
			{
				icon = Core::Instance ().GetCoreProxy ()->GetIcon ("im-msn");
				status = FriendsModel::FSBothFriends;
			}
			else if (fr->GetFriendOf ())
			{
				icon = Core::Instance ().GetCoreProxy ()->GetIcon ("im-user-offline");
				status = FriendsModel::FSFriendOf;
			}
			else if (fr->GetMyFriend ())
			{
				icon = Core::Instance ().GetCoreProxy ()->GetIcon ("im-user");
				status = FriendsModel::FSMyFriend;
			}
			QStandardItem *itemStatus = new QStandardItem (icon, QString ());
			itemStatus->setData (status, FriendsModel::FRFriendStatus);
			QStandardItem *itemBirthday = new QStandardItem (fr->GetBirthday ());
			Item2Friend_ [item] = fr;

			item->setData (fr->GetBGColor ().name (), ItemColorRoles::BackgroundColor);
			item->setData (fr->GetFGColor ().name (), ItemColorRoles::ForegroundColor);
			
			FriendsModel_->appendRow ({ item, itemStatus, itemName, itemBirthday });
		}

		Ui_.FriendsView_->header ()->setResizeMode (QHeaderView::ResizeToContents);
	}

	void ProfileWidget::FillCommunities (const QStringList& communities)
	{
		const QIcon& icon = Core::Instance ().GetCoreProxy ()->GetIcon ("system-users");
		for (const auto& community : communities)
		{
			QStandardItem *item = new QStandardItem (icon, community);
			item->setEditable (false);
			CommunitiesModel_->appendRow (item);
		}
	}

	void ProfileWidget::ReFillModels ()
	{
		const LJProfileData& data = Profile_->GetProfileData ();
		FriendsModel_->removeRows (0, FriendsModel_->rowCount ());
		Item2Friend_.clear ();
		Item2FriendGroup_.clear ();
		FillFriends (data.Friends_);
		
		CommunitiesModel_->removeRows (0, CommunitiesModel_->rowCount());
		FillCommunities (data.Communities_);
	}

	void ProfileWidget::updateProfile ()
	{
		if (Profile_)
			RereadProfileData ();
		else
			qWarning () << Q_FUNC_INFO
					<< "Profile is set to 0";
	}

	void ProfileWidget::on_ColoringFriendsList__toggled (bool toggle)
	{
		XmlSettingsManager::Instance ().setProperty ("ColoringFriendsList", toggle);
		emit coloringItemChanged ();
	}

	void ProfileWidget::on_Add__released ()
	{
		AddEditEntryDialog *aeed = new AddEditEntryDialog (Profile_, ATEFriend, this);
		aeed->setAttribute (Qt::WA_DeleteOnClose);
		if (aeed->exec () == QDialog::Rejected)
			return;

		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		const QString& userName = aeed->GetUserName ();
		const QString& bgcolor = aeed->GetBackgroundColorName ();
		const QString& fgcolor = aeed->GetForegroundColorName ();
		uint realId =  aeed->GetGroupRealId ();

		account->AddNewFriend (userName, bgcolor, fgcolor, realId);
	}

	void ProfileWidget::on_Edit__released ()
	{
		auto index = Ui_.FriendsView_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		QString msg;
		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		AddEditEntryDialog *dlg = new AddEditEntryDialog (Profile_, ATENone, this);
		dlg->setAttribute (Qt::WA_DeleteOnClose);
		dlg->ShowAddTypePossibility (false);
		dlg->SetCurrentAddTypeEntry (ATEFriend);
		LJFriendEntry_ptr entry = Item2Friend_ [FriendsModel_->itemFromIndex (FriendsProxyModel_->mapToSource (index))];
		dlg->SetUserName (entry->GetUserName ());
		dlg->SetBackgroundColor (entry->GetBGColor ());
		dlg->SetForegroundColor (entry->GetFGColor ());
		dlg->SetGroup (entry->GetGroupMask ());

		if (dlg->exec () == QDialog::Rejected)
			return;

		const QString& userName = dlg->GetUserName ();
		const QString& bgcolor = dlg->GetBackgroundColorName ();
		const QString& fgcolor = dlg->GetForegroundColorName ();
		uint realId = dlg->GetGroupRealId ();

		account->AddNewFriend (userName, bgcolor, fgcolor, realId);
	}

	void ProfileWidget::on_Delete__released ()
	{
		auto index = Ui_.FriendsView_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), Columns::Name);
		if (!index.isValid ())
			return;

		const QString& msg = tr ("Are you sure to delete user <b>%1</b> from your friends.")
				.arg (index.data ().toString ());
		int res = QMessageBox::question (this,
				tr ("Change friendslist"),
				msg,
				QMessageBox::Ok | QMessageBox::Cancel,
				QMessageBox::Cancel);

		if (res == QMessageBox::Ok)
		{
			LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
			if (!account)
				return;
			account->DeleteFriend (index.data ().toString ());
		}
	}

	void ProfileWidget::on_UpdateProfile__released ()
	{
		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;
		account->updateProfile ();
	}

	void ProfileWidget::handleUserGroupChanged (const QString& username,
			const QString& bgColor, const QString& fgColor, int groupId)
	{
		LJAccount *account = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		if (!account)
			return;

		account->AddNewFriend (username, bgColor, fgColor, groupId);
	}
	
	void ProfileWidget::handleFriendFilterTextChanged (const QString& text)
	{
		FriendsProxyModel_->setFilterFixedString (text);
	}

}
}
}


