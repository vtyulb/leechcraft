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

#include "authmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <util/passutils.h>
#include "consts.h"

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	AuthManager::AuthManager (QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	{
	}

	QString AuthManager::GetSID (const QString& login)
	{
		if (Login2Sid_.contains (login))
			return Login2Sid_ [login];

		const auto& pass = Util::GetPassword ("org.LeechCraft.LMP.MP3Tunes.Account." + login,
				tr ("Enter password for MP3tunes account %1:").arg (login),
				this,
				!FailedAuth_.contains (login));
		if (pass.isEmpty ())
		{
			emit sidError (login, tr ("Empty password."));
			return QString ();
		}

		const auto authUrl = QString ("https://shop.mp3tunes.com/api/v1/login?output=xml&"
				"username=%1&password=%2&partner_token=%3")
					.arg (login)
					.arg (pass)
					.arg (Consts::PartnerId);
		auto reply = NAM_->get (QNetworkRequest (authUrl));
		reply->setProperty ("Login", login);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAuthReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleAuthReplyError ()));

		return QString ();
	}

	void AuthManager::handleAuthReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& login = reply->property ("Login").toString ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			emit sidError (login, tr ("Unable to parse authentication reply."));
			return;
		}

		const auto& docElem = doc.documentElement ();
		if (docElem.firstChildElement ("status").text () != "1")
		{
			FailedAuth_ << login;
			emit sidError (login, docElem.firstChildElement ("errorMessage").text ());
			return;
		}

		Login2Sid_ [login] = docElem.firstChildElement ("session_id").text ();
		FailedAuth_.remove (login);

		emit sidReady (login);
	}

	void AuthManager::handleAuthReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		emit sidError (reply->property ("Login").toString (),
				tr ("Unable to parse authentication reply."));
	}
}
}
}
