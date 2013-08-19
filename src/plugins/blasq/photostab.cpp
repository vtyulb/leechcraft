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

#include "photostab.h"
#include <QToolBar>
#include <QComboBox>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeNetworkAccessManagerFactory>
#include <QGraphicsObject>
#include <QClipboard>
#include <QtDebug>
#include <interfaces/core/ientitymanager.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/sys/paths.h>
#include <util/util.h>
#include <util/network/networkdiskcache.h>
#include "interfaces/blasq/iaccount.h"
#include "interfaces/blasq/isupportuploads.h"
#include "accountsmanager.h"
#include "xmlsettingsmanager.h"
#include "uploadphotosdialog.h"
#include "photosproxymodel.h"

Q_DECLARE_METATYPE (QModelIndex)

namespace LeechCraft
{
namespace Blasq
{
	namespace
	{
		class NAMFactory : public QDeclarativeNetworkAccessManagerFactory
		{
		public:
			QNetworkAccessManager* create (QObject *parent)
			{
				auto nam = new QNetworkAccessManager (parent);

				auto cache = new Util::NetworkDiskCache ("blasq/cache", nam);
				const auto cacheSize = XmlSettingsManager::Instance ().property ("CacheSize").toInt ();
				cache->setMaximumCacheSize (cacheSize * 1024 * 1024);

				nam->setCache (cache);

				return nam;
			}
		};
	}

	PhotosTab::PhotosTab (AccountsManager *accMgr, const TabClassInfo& tc, QObject *plugin, ICoreProxy_ptr proxy)
	: TC_ (tc)
	, Plugin_ (plugin)
	, AccMgr_ (accMgr)
	, Proxy_ (proxy)
	, ProxyModel_ (new NamedModel<PhotosProxyModel> (this))
	, AccountsBox_ (new QComboBox)
	, Toolbar_ (new QToolBar)
	{
		Ui_.setupUi (this);

		Ui_.ImagesView_->setResizeMode (QDeclarativeView::SizeRootObjectToView);

		auto rootCtx = Ui_.ImagesView_->rootContext ();
		rootCtx->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		rootCtx->setContextProperty ("collectionModel",
				QVariant::fromValue<QObject*> (ProxyModel_));
		rootCtx->setContextProperty ("listingMode", "false");
		rootCtx->setContextProperty ("collRootIndex", QVariant::fromValue (QModelIndex ()));

		auto engine = Ui_.ImagesView_->engine ();
		engine->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));
		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			engine->addImportPath (cand);
		engine->setNetworkAccessManagerFactory (new NAMFactory);

		const auto& path = Util::GetSysPath (Util::SysPath::QML, "blasq", "PhotoView.qml");
		Ui_.ImagesView_->setSource (QUrl::fromLocalFile (path));

		auto rootObj = Ui_.ImagesView_->rootObject ();
		connect (rootObj,
				SIGNAL (imageSelected (QString)),
				this,
				SLOT (handleImageSelected (QString)));
		connect (rootObj,
				SIGNAL (imageOpenRequested (QVariant)),
				this,
				SLOT (handleImageOpenRequested (QVariant)));
		connect (rootObj,
				SIGNAL (imageDownloadRequested (QVariant)),
				this,
				SLOT (handleImageDownloadRequested (QVariant)));
		connect (rootObj,
				SIGNAL (copyURLRequested (QVariant)),
				this,
				SLOT (handleCopyURLRequested (QVariant)));

		AccountsBox_->setModel (AccMgr_->GetModel ());
		AccountsBox_->setModelColumn (AccountsManager::Column::Name);
		connect (AccountsBox_,
				SIGNAL (activated (int)),
				this,
				SLOT (handleAccountChosen (int)));

		UploadAction_ = new QAction (tr ("Upload photos..."), this);
		UploadAction_->setProperty ("ActionIcon", "svn-commit");
		connect (UploadAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (uploadPhotos ()));

		if (AccountsBox_->count ())
			handleAccountChosen (0);

		Toolbar_->addWidget (AccountsBox_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (UploadAction_);

		connect (Ui_.CollectionsTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleRowChanged (QModelIndex)));
	}

	PhotosTab::PhotosTab (AccountsManager *accMgr, ICoreProxy_ptr proxy)
	: PhotosTab (accMgr, {}, nullptr, proxy)
	{
	}

	TabClassInfo PhotosTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* PhotosTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void PhotosTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* PhotosTab::GetToolBar () const
	{
		return Toolbar_.get ();
	}

	QModelIndex PhotosTab::GetSelectedImage () const
	{
		if (SelectedID_.isEmpty ())
			return {};

		auto model = CurAcc_->GetCollectionsModel ();
		QModelIndex allPhotosIdx;
		for (auto i = 0; i < model->rowCount (); ++i)
		{
			const auto& idx = model->index (i, 0);
			if (idx.data (CollectionRole::Type).toInt () == ItemType::AllPhotos)
			{
				allPhotosIdx = idx;
				break;
			}
		}

		if (!allPhotosIdx.isValid ())
			return {};

		for (auto i = 0, rc = model->rowCount (allPhotosIdx); i < rc; ++i)
		{
			const auto& idx = allPhotosIdx.child (i, 0);
			if (idx.data (CollectionRole::ID).toString () == SelectedID_)
				return idx;
		}

		return {};
	}

	void PhotosTab::HandleImageSelected (const QModelIndex& index)
	{
		Ui_.ImagesView_->rootContext ()->setContextProperty ("listingMode", QVariant (false));

		handleImageSelected (index.data (CollectionRole::ID).toString ());

		QMetaObject::invokeMethod (Ui_.ImagesView_->rootObject (),
				"showImage",
				Q_ARG (QVariant, index.data (CollectionRole::Original).toUrl ()));
	}

	void PhotosTab::HandleCollectionSelected (const QModelIndex& index)
	{
		auto rootCtx = Ui_.ImagesView_->rootContext ();
		if (!rootCtx->contextProperty ("listingMode").toBool ())
		{
			QMetaObject::invokeMethod (Ui_.ImagesView_->rootObject (),
					"showImage",
					Q_ARG (QVariant, QUrl ()));

			rootCtx->setContextProperty ("listingMode", true);
		}

		rootCtx->setContextProperty ("collRootIndex", QVariant::fromValue (index));

		SelectedID_.clear ();
	}

	void PhotosTab::handleAccountChosen (int idx)
	{
		auto accVar = AccountsBox_->itemData (idx, AccountsManager::Role::AccountObj);
		auto accObj = accVar.value<QObject*> ();
		auto acc = qobject_cast<IAccount*> (accObj);
		if (acc == CurAcc_)
			return;

		if (CurAccObj_)
			disconnect (CurAccObj_,
					0,
					this,
					0);

		CurAccObj_ = accObj;
		CurAcc_ = acc;

		CurAcc_->UpdateCollections ();

		auto model = CurAcc_->GetCollectionsModel ();

		if (auto sel = Ui_.CollectionsTree_->selectionModel ())
			disconnect (sel,
					SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
					this,
					SLOT (handleRowChanged (QModelIndex)));
		Ui_.CollectionsTree_->setModel (model);
		connect (Ui_.CollectionsTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleRowChanged (QModelIndex)));

		ProxyModel_->setSourceModel (model);

		Ui_.ImagesView_->rootContext ()->setContextProperty ("collRootIndex", QVariant::fromValue (QModelIndex ()));
		HandleCollectionSelected ({});

		UploadAction_->setEnabled (qobject_cast<ISupportUploads*> (CurAccObj_));
	}

	void PhotosTab::handleRowChanged (const QModelIndex& index)
	{
		if (index.data (CollectionRole::Type).toInt () == ItemType::Image)
			HandleImageSelected (ProxyModel_->mapFromSource (index));
		else
			HandleCollectionSelected (ProxyModel_->mapFromSource (index));
	}

	void PhotosTab::uploadPhotos ()
	{
		UploadPhotosDialog dia (CurAccObj_, this);
		if (dia.exec () != QDialog::Accepted)
			return;

		auto isu = qobject_cast<ISupportUploads*> (CurAccObj_);
		isu->UploadImages (dia.GetSelectedCollection (), dia.GetSelectedFiles ());
	}

	void PhotosTab::handleImageSelected (const QString& id)
	{
		SelectedID_ = id;
	}

	void PhotosTab::handleImageOpenRequested (const QVariant& var)
	{
		const auto& url = var.toUrl ();
		if (!url.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid URL"
					<< var;
			return;
		}

		const auto& entity = Util::MakeEntity (url, QString (), FromUserInitiated | OnlyHandle);
		Proxy_->GetEntityManager ()->HandleEntity (entity);
	}

	void PhotosTab::handleImageDownloadRequested (const QVariant& var)
	{
		const auto& url = var.toUrl ();
		if (!url.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid URL"
					<< var;
			return;
		}

		const auto& entity = Util::MakeEntity (url, QString (), FromUserInitiated | OnlyDownload);
		Proxy_->GetEntityManager ()->HandleEntity (entity);
	}

	void PhotosTab::handleCopyURLRequested (const QVariant& var)
	{
		const auto& url = var.toUrl ();
		if (!url.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid URL"
					<< var;
			return;
		}

		auto cb = qApp->clipboard ();
		cb->setText (url.toString (), QClipboard::Clipboard);
	}
}
}
