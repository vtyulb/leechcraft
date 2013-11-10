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

#include "ljbloggingplatform.h"
#include <QIcon>
#include <QInputDialog>
#include <QSettings>
#include <QtDebug>
#include <QTimer>
#include <QMainWindow>
#include <QDomElement>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/passutils.h>
#include "core.h"
#include "ljaccount.h"
#include "ljaccountconfigurationwidget.h"
#include "postoptionswidget.h"
#include "localstorage.h"
#include "xmlsettingsmanager.h"
#include "polldialog.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJBloggingPlatform::LJBloggingPlatform (QObject *parent)
	: QObject (parent)
	, ParentBlogginPlatfromPlugin_ (parent)
	, PluginProxy_ (0)
	, LJUser_ (new QAction (Core::Instance ().GetCoreProxy ()->GetIcon ("user-properties"),
			tr ("Add LJ user"), this))
	, LJPoll_ (new QAction (Core::Instance ().GetCoreProxy ()->GetIcon ("office-chart-pie"),
			tr ("Create poll"), this))
	, LJCut_ (new QAction (Core::Instance ().GetCoreProxy ()->GetIcon ("user-properties"),
			tr ("Insert LJ cut"), this))
	, FirstSeparator_ (new QAction (this))
	, MessageCheckingTimer_ (new QTimer (this))
	{
		FirstSeparator_->setSeparator (true);

		connect (LJUser_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddLJUser ()));
		connect (LJPoll_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddLJPoll ()));

		connect (MessageCheckingTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkForMessages ()));

		XmlSettingsManager::Instance ().RegisterObject ("CheckingInboxEnabled",
				this, "handleMessageChecking");
		handleMessageChecking ();
	}

	QObject* LJBloggingPlatform::GetQObject ()
	{
		return this;
	}

	IBloggingPlatform::BloggingPlatfromFeatures LJBloggingPlatform::GetFeatures () const
	{
		return BPFSupportsProfiles | BPFSelectablePostDestination | BPFSupportsBackup | BPFSupportComments;
	}

	QObjectList LJBloggingPlatform::GetRegisteredAccounts ()
	{
		QObjectList result;
		Q_FOREACH (auto acc, LJAccounts_)
			result << acc;
		return result;
	}

	QObject* LJBloggingPlatform::GetParentBloggingPlatformPlugin () const
	{
		return ParentBlogginPlatfromPlugin_;
	}

	QString LJBloggingPlatform::GetBloggingPlatformName () const
	{
		return "LiveJournal";
	}

	QIcon LJBloggingPlatform::GetBloggingPlatformIcon () const
	{
		return QIcon ("lcicons:/plugins/blogique/plugins/metida/resources/images/livejournalicon.svg");
	}

	QByteArray LJBloggingPlatform::GetBloggingPlatformID () const
	{
		return "Blogique.Metida.LiveJournal";
	}

	QList<QWidget*> LJBloggingPlatform::GetAccountRegistrationWidgets (IBloggingPlatform::AccountAddOptions, const QString&)
	{
		QList<QWidget*> result;
		result << new LJAccountConfigurationWidget ();
		return result;
	}

	void LJBloggingPlatform::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
		auto w = qobject_cast<LJAccountConfigurationWidget*> (widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}

		LJAccount *account = new LJAccount (name, this);
		account->FillSettings (w);

		const QString& pass = w->GetPassword ();
		if (!pass.isEmpty ())
			Util::SavePassword (pass,
					"org.LeechCraft.Blogique.PassForAccount/" + account->GetAccountID (),
					&Core::Instance ());

		LJAccounts_ << account;
		saveAccounts ();
		emit accountAdded (account);
		account->Init ();
		Core::Instance ().GetLocalStorage ()->AddAccount (account->GetAccountID ());
	}

	void LJBloggingPlatform::RemoveAccount (QObject *account)
	{
		LJAccount *acc = qobject_cast<LJAccount*> (account);
		if (LJAccounts_.removeAll (acc))
		{
			emit accountRemoved (account);
			Core::Instance ().GetLocalStorage ()->RemoveAccount (acc->GetAccountID ());
			account->deleteLater ();
			saveAccounts ();
		}
	}

	QList<QAction*> LJBloggingPlatform::GetEditorActions () const
	{
		return { FirstSeparator_, LJUser_, LJPoll_ };
	}

	QList<InlineTagInserter> LJBloggingPlatform::GetInlineTagInserters () const
	{
		return { InlineTagInserter { "lj-cut", QVariantMap (), [] (QAction *action)
				{
					action->setText ("Insert cut");
					action->setIcon (Core::Instance ().GetCoreProxy ()->
							GetIcon ("distribute-vertical-equal"));
				} } };
	}

	QList<QWidget*> LJBloggingPlatform::GetBlogiqueSideWidgets () const
	{
		return { new PostOptionsWidget };
	}

	void LJBloggingPlatform::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	void LJBloggingPlatform::Prepare ()
	{
		RestoreAccounts ();
	}

	void LJBloggingPlatform::Release ()
	{
		saveAccounts ();
	}

	IAdvancedHTMLEditor::CustomTags_t LJBloggingPlatform::GetCustomTags () const
	{
		IAdvancedHTMLEditor::CustomTags_t tags;

		IAdvancedHTMLEditor::CustomTag ljUserTag;
		ljUserTag.TagName_ = "lj";
		ljUserTag.ToKnown_ = [] (QDomElement& elem) -> void
		{
			const auto& user = elem.attribute ("user");
			elem.setTagName ("span");
			elem.setAttribute ("contenteditable", "false");

			QDomElement linkElem = elem.ownerDocument ().createElement ("a");
			linkElem.setAttribute ("href", QString ("http://%1.livejournal.com/profile").arg (user));
			linkElem.setAttribute ("target", "_blank");

			QDomElement imgElem = elem.ownerDocument ().createElement ("img");
			imgElem.setAttribute ("src", "http://l-stat.livejournal.com/img/userinfo.gif?v=17080");
			linkElem.appendChild (imgElem);

			QDomElement nameElem = elem.ownerDocument ().createElement ("a");
			nameElem.setAttribute ("href", QString ("http://%1.livejournal.com/profile").arg (user));
			nameElem.setAttribute ("target", "_blank");
			nameElem.setAttribute ("id", "nameLink");
			nameElem.setAttribute ("contenteditable", "true");
			nameElem.appendChild (elem.ownerDocument ().createTextNode (user));

			elem.appendChild (linkElem);
			elem.appendChild (nameElem);

			elem.removeAttribute ("user");
		};
		ljUserTag.FromKnown_ = [] (QDomElement& elem) -> bool
		{
			auto aElem = elem.firstChildElement ("a");
			while (!aElem.isNull ())
			{
				if (aElem.attribute ("id") == "nameLink")
					break;

				aElem = aElem.nextSiblingElement ("a");
			}
			if (aElem.isNull ())
				return false;
			const auto& username = aElem.text ();

			const auto& children = elem.childNodes ();
			while (!children.isEmpty ())
				elem.removeChild (children.at (0));

			elem.setTagName ("lj");
			elem.setAttribute ("user", username);

			return true;
		};
		tags << ljUserTag;

		IAdvancedHTMLEditor::CustomTag ljCutTag;
		ljCutTag.TagName_ = "lj-cut";
		ljCutTag.ToKnown_ = [] (QDomElement& elem) -> void
		{
			elem.setTagName ("div");
			const auto& text = elem.attribute ("text");
			elem.removeAttribute ("text");
			elem.setAttribute ("id", "cutTag");
			elem.setAttribute ("style", "overflow:auto;border-width:3px;border-style:dotted;margin-left:3em;padding:2em 2em;");
			elem.setAttribute ("text", text);
		};
		ljCutTag.FromKnown_ = [] (QDomElement& elem) -> bool
		{
			if (!elem.hasAttribute ("id") ||
					elem.attribute ("id") != "cutTag")
				return false;

			elem.removeAttribute ("id");
			elem.removeAttribute ("style");
			const auto& text = elem.attribute ("text");
			elem.removeAttribute ("text");
			elem.setTagName ("lj-cut");
			if (!text.isEmpty ())
				elem.setAttribute ("text", text);

			return true;
		};

		tags << ljCutTag;

		IAdvancedHTMLEditor::CustomTag ljPollTag;
		ljPollTag.TagName_ = "lj-poll";
		ljPollTag.ToKnown_ = [this] (QDomElement& elem) -> void
		{
			const auto& whoView = elem.attribute ("whoview");
			const auto& whoVote = elem.attribute ("whovote");
			const auto& name = elem.attribute ("name");

			auto children = elem.childNodes ();
			while (!children.isEmpty ())
				elem.removeChild (children.at (0));

			elem.setTagName ("div");
			elem.setAttribute ("style", "overflow:auto;border-width:2px;border-style:solid;border-radius:5px;margin-left:3em;padding:2em 2em;");
			elem.setAttribute ("id", "pollDiv");
			elem.setAttribute ("ljPollWhoview", whoView);
			elem.setAttribute ("ljPollWhovote", whoVote);
			elem.setAttribute ("ljPollName", name);
			auto textElem = elem.ownerDocument ().createTextNode (tr ("Poll: %1").arg (name));
			elem.appendChild (textElem);
		};
		ljPollTag.FromKnown_ = [] (QDomElement& elem) -> bool
		{
			if (!elem.hasAttribute ("id") ||
					elem.attribute ("id") != "pollDiv")
				return false;

			auto whoView = elem.attribute ("ljPollWhoview");
			auto whoVote = elem.attribute ("ljPollWhovote");
			auto name = elem.attribute ("ljPollName");

			elem.removeAttribute ("style");
			elem.removeAttribute ("ljPollWhoview");
			elem.removeAttribute ("ljPollWhovot");
			elem.removeAttribute ("ljPollName");
			elem.removeAttribute ("id");
			elem.removeChild (elem.firstChild ());

			elem.setTagName ("lj-poll");
			elem.setAttribute ("whoview", whoView);
			elem.setAttribute ("whovote", whoVote);
			elem.setAttribute ("name", name);

			return true;
		};

		tags << ljPollTag;

		IAdvancedHTMLEditor::CustomTag ljEmbedTag;
		ljEmbedTag.TagName_ = "lj-embed";
		ljEmbedTag.ToKnown_ = [this] (QDomElement& elem) -> void
		{
			const auto& id = elem.attribute ("id");
			elem.removeAttribute ("id");

			elem.setTagName ("div");
			elem.setAttribute ("style", "overflow:auto;border-width:2px;border-style:solid;border-radius:5px;margin-left:3em;padding:2em 2em;");
			elem.setAttribute ("id", "embedTag");
			elem.setAttribute ("name", id);
			auto textElem = elem.ownerDocument ().createTextNode (tr ("Embeded: %1")
					.arg (id));
			elem.appendChild (textElem);
		};
		ljEmbedTag.FromKnown_ = [] (QDomElement& elem) -> bool
		{
			if (!elem.hasAttribute ("id") ||
					elem.attribute ("id") != "embedTag")
				return false;

			elem.removeAttribute ("style");
			elem.removeChild (elem.firstChild ());
			const auto& id = elem.attribute ("name");
			elem.removeAttribute ("id");
			elem.setTagName ("lj-embed");
			elem.setAttribute ("id", id);
			return true;
		};

		tags << ljEmbedTag;

		IAdvancedHTMLEditor::CustomTag ljLikeTag;
		ljLikeTag.TagName_ = "lj-like";
		ljLikeTag.ToKnown_ = [this] (QDomElement& elem) -> void
		{
			const auto& buttons = elem.attribute ("buttons");
			elem.removeAttribute ("buttons");

			elem.setTagName ("div");
			elem.setAttribute ("style", "overflow:auto;border-width:2px;border-style:solid;border-radius:5px;margin-left:3em;padding:2em 2em;");
			elem.setAttribute ("likes", buttons);
			auto textElem = elem.ownerDocument ().createTextNode (tr ("Likes: %1")
					.arg (!buttons.isEmpty () ?
						buttons :
						"repost,facebook,twitter,google,vkontakte,surfingbird,tumblr,livejournal"));
			elem.appendChild (textElem);
		};
		ljLikeTag.FromKnown_ = [] (QDomElement& elem) -> bool
		{
			const auto& likes = elem.attribute ("likes");
			if (likes.isEmpty ())
				return false;

			elem.removeAttribute ("likes");
			elem.removeAttribute ("style");
			elem.setTagName ("lj-like");
			elem.setAttribute ("buttons", likes);
			elem.removeChild (elem.firstChild ());
			return true;
		};

		tags << ljLikeTag;

		return tags;
	}

	void LJBloggingPlatform::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
						"_Blogique_Metida_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings.value ("SerializedData").toByteArray ();
			LJAccount *acc = LJAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}
			LJAccounts_ << acc;
			emit accountAdded (acc);

			acc->Init ();
			Core::Instance ().GetLocalStorage ()->AddAccount (acc->GetAccountID ());
		}
		settings.endArray ();
	}

	void LJBloggingPlatform::saveAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
						"_Blogique_Metida_Accounts");
		settings.beginWriteArray ("Accounts");
		for (int i = 0, size = LJAccounts_.size (); i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData",
					LJAccounts_.at (i)->Serialize ());
		}
		settings.endArray ();
		settings.sync ();
	}

	void LJBloggingPlatform::handleAddLJUser ()
	{
		auto rootWM = Core::Instance ().GetCoreProxy ()->GetRootWindowsManager ();
		QString name = QInputDialog::getText (rootWM->GetPreferredWindow (),
				tr ("Add LJ User"),
				tr ("Enter LJ user name"));
		if (name.isEmpty ())
			return;

		emit insertTag (QString ("<lj user=\"%1\" />").arg (name));
	}

	void LJBloggingPlatform::handleAddLJPoll ()
	{
		PollDialog pollDlg;
		if (pollDlg.exec () == QDialog::Rejected)
			return;

		QStringList pqParts;
		QString pqPart = QString ("<lj-pq type=\"%1\" %2>%3%4</lj-pq>");
		bool isPqParam = false;
		for (const auto& pollType : pollDlg.GetPollTypes ())
		{
			const auto& map = pollDlg.GetPollFields (pollType);
			QStringList pqParams;
			if (pollType == "check" ||
					pollType == "radio" ||
					pollType == "drop")
			{
				isPqParam = false;
				for (const auto& value : map.values ())
					pqParams << QString ("<lj-pi>%1</lj-pi>")
							.arg (value.toString ());
			}
			else
			{
				isPqParam = true;
				for (const auto& key : map.keys ())
					pqParams << QString ("%1=\"%2\"")
							.arg (key)
							.arg (map [key].toString ());
			}
			pqParts << pqPart
					.arg (pollType)
					.arg (isPqParam ? pqParams.join (" ") : QString ())
					.arg (pollDlg.GetPollQuestion (pollType))
					.arg (!isPqParam ? pqParams.join (" ") : QString ());
		}

		QString pollPart = QString ("<lj-poll name=\"%1\" whovote=\"%2\" whoview=\"%3\">%4</lj-poll>")
				.arg (pollDlg.GetPollName ())
				.arg (pollDlg.GetWhoCanVote ())
				.arg (pollDlg.GetWhoCanView ())
				.arg (pqParts.join (""));
		emit insertTag (pollPart);
	}

	void LJBloggingPlatform::handleAccountValidated (bool validated)
	{
		IAccount *acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not an IAccount";;
			return;
		}

		emit accountValidated (acc->GetQObject (), validated);
		if (validated &&
				XmlSettingsManager::Instance ().Property ("CheckingInboxEnabled", true).toBool ())
			checkForMessages ();;
	}

	void LJBloggingPlatform::handleMessageChecking ()
	{
		if (!XmlSettingsManager::Instance ().Property ("CheckingInboxEnabled", true).toBool () &&
				MessageCheckingTimer_->isActive ())
			MessageCheckingTimer_->stop ();
	}

	void LJBloggingPlatform::handleMessageUpdateIntervalChanged ()
	{
		if (XmlSettingsManager::Instance ().Property ("CheckingInboxEnabled", true).toBool ())
			MessageCheckingTimer_->start (XmlSettingsManager::Instance ()
					.property ("UpdateInboxInterval").toInt () * 60 * 1000);
		else if (MessageCheckingTimer_->isActive ())
			MessageCheckingTimer_->stop ();
	}

	void LJBloggingPlatform::checkForMessages ()
	{
		for (auto account : LJAccounts_)
			account->RequestInbox ();
	}
}
}
}
