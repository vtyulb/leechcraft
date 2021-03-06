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

#include "addcontactdialog.h"
#include <QStringListModel>
#include <util/tags/tagscompleter.h>
#include "interfaces/azoth/iprotocol.h"
#include "interfaces/azoth/iaccount.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	AddContactDialog::AddContactDialog (IAccount *focusAcc, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Util::TagsCompleter *tc = new Util::TagsCompleter (Ui_.Groups_, this);
		tc->OverrideModel (new QStringListModel (Core::Instance ().GetChatGroups (), this));
		Ui_.Groups_->AddSelector ();

		Q_FOREACH (IProtocol *proto, Core::Instance ().GetProtocols ())
			Ui_.Protocol_->addItem (proto->GetProtocolName (),
					QVariant::fromValue<IProtocol*> (proto));

		if (focusAcc)
			FocusAccount (focusAcc);

		checkComplete ();
		connect (Ui_.ContactID_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (checkComplete ()));
	}

	void AddContactDialog::SetContactID (const QString& id)
	{
		Ui_.ContactID_->setText (id);
	}

	void AddContactDialog::SetNick (const QString& nick)
	{
		Ui_.Nick_->setText (nick);
	}

	IAccount* AddContactDialog::GetSelectedAccount () const
	{
		int idx = Ui_.Account_->currentIndex ();
		return idx >= 0 ?
			Ui_.Account_->itemData (idx).value<IAccount*> () :
			0;
	}

	QString AddContactDialog::GetContactID () const
	{
		return Ui_.ContactID_->text ();
	}

	QString AddContactDialog::GetNick () const
	{
		return Ui_.Nick_->text ();
	}

	QString AddContactDialog::GetReason () const
	{
		return Ui_.Reason_->toPlainText ();
	}

	QStringList AddContactDialog::GetGroups () const
	{
		QStringList result;
		Q_FOREACH (const QString& str, Ui_.Groups_->text ().split (';'))
			result << str.trimmed ();
		return result;
	}

	void AddContactDialog::on_Protocol__currentIndexChanged (int idx)
	{
		Ui_.Account_->clear ();
		if (idx < 0)
			return;

		IProtocol *proto = Ui_.Protocol_->
				itemData (idx).value<IProtocol*> ();
		Q_FOREACH (QObject *accObj, proto->GetRegisteredAccounts ())
		{
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< accObj
						<< "to IAccount";
				continue;
			}

			if (!acc->IsShownInRoster ())
				continue;

			if (acc->GetState ().State_ == SOffline &&
					!(acc->GetAccountFeatures () & IAccount::FCanAddContactsInOffline))
				continue;

			Ui_.Account_->addItem (QString ("%1 (%2)")
						.arg (acc->GetAccountName ())
						.arg (acc->GetOurNick ()),
					QVariant::fromValue<IAccount*> (acc));
		}
	}

	void AddContactDialog::FocusAccount (IAccount *focusAcc)
	{
		QObject *protoObj = focusAcc->GetParentProtocol ();
		IProtocol *focusProto = qobject_cast<IProtocol*> (protoObj);
		if (!focusProto)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< protoObj
					<< "to IProtocol";
			return;
		}

		for (int i = 0; i < Ui_.Protocol_->count (); ++i)
			if (Ui_.Protocol_->itemData (i).value<IProtocol*> () == focusProto)
			{
				Ui_.Protocol_->setCurrentIndex (i);
				break;
			}

		for (int i = 0; i < Ui_.Account_->count (); ++i)
			if (Ui_.Account_->itemData (i).value<IAccount*> () == focusAcc)
			{
				Ui_.Account_->setCurrentIndex (i);
				break;
			}
	}

	void AddContactDialog::checkComplete ()
	{
		const bool isComplete = GetSelectedAccount () &&
				!Ui_.ContactID_->text ().isEmpty ();
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (isComplete);
	}
}
}
