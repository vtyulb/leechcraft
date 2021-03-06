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

#include "reportwizard.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QtDebug>
#include <QMessageBox>
#include "chooseuserpage.h"
#include "userstatuspage.h"
#include "reporttypepage.h"
#include "bugreportpage.h"
#include "featurerequestpage.h"
#include "fileattachpage.h"
#include "previewpage.h"
#include "finalpage.h"

namespace LeechCraft
{
namespace Dolozhee
{
	ReportWizard::ReportWizard (ICoreProxy_ptr proxy, QWidget *parent)
	: QWizard (parent)
	, Proxy_ (proxy)
	, NAM_ (new QNetworkAccessManager (this))
	, ChooseUser_ (new ChooseUserPage)
	, ReportType_ (new ReportTypePage)
	, BugReportPage_ (new BugReportPage (proxy))
	, FRPage_ (new FeatureRequestPage)
	, FilePage_ (new FileAttachPage)
	, PreviewPage_ (new PreviewPage)
	, FirstAuth_ (true)
	{
		setWindowTitle (tr ("Issue reporter"));

		connect (ChooseUser_,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)));
		connect (ChooseUser_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));

		setPage (PageID::ChooseUser, ChooseUser_);
		setPage (PageID::UserStatus, new UserStatusPage ());
		setPage (PageID::ReportType, ReportType_);
		setPage (PageID::BugDetails, BugReportPage_);
		setPage (PageID::FeatureDetails, FRPage_);
		setPage (PageID::PreviewRequestPage, PreviewPage_);
		setPage (PageID::FilePage, FilePage_);
		auto final = new FinalPage;
		setPage (PageID::Final, final);
		connect (final,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));

		connect (NAM_,
				SIGNAL (authenticationRequired (QNetworkReply*, QAuthenticator*)),
				this,
				SLOT (handleAuthenticationRequired (QNetworkReply*, QAuthenticator*)));
	}

	QNetworkAccessManager* ReportWizard::GetNAM () const
	{
		return NAM_;
	}

	QNetworkReply* ReportWizard::PostRequest (const QString& address,
			const QByteArray& data, const QByteArray& contentType)
	{
		QNetworkRequest req ("http://dev.leechcraft.org" + address);
		req.setHeader (QNetworkRequest::ContentTypeHeader, contentType);

		const auto& user = ChooseUser_->GetLogin ().toUtf8 ();
		const auto& pass = ChooseUser_->GetPassword ().toUtf8 ();
		req.setRawHeader ("Authorization", "Basic " + (user + ':' + pass).toBase64 ());
		return NAM_->post (req, data);
	}

	ChooseUserPage* ReportWizard::GetChooseUserPage () const
	{
		return ChooseUser_;
	}

	ReportTypePage* ReportWizard::GetReportTypePage () const
	{
		return ReportType_;
	}

	BugReportPage* ReportWizard::GetBugReportPage () const
	{
		return BugReportPage_;
	}

	FeatureRequestPage* ReportWizard::GetFRPage () const
	{
		return FRPage_;
	}

	FileAttachPage* ReportWizard::GetFilePage () const
	{
		return FilePage_;
	}

	void ReportWizard::handleAuthenticationRequired (QNetworkReply*, QAuthenticator *auth)
	{
		qDebug () << Q_FUNC_INFO << FirstAuth_;
		QMessageBox::warning (this, "Dolozhee", tr ("Invalid credentials"));
		restart ();
	}
}
}
