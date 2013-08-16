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

#include "uploadphotosdialog.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include "interfaces/blasq/iaccount.h"
#include "selectalbumdialog.h"

namespace LeechCraft
{
namespace Blasq
{
	UploadPhotosDialog::UploadPhotosDialog (QObject *accObj, QWidget *parent)
	: QDialog (parent)
	, AccObj_ (accObj)
	, Acc_ (qobject_cast<IAccount*> (accObj))
	, FilesModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.PhotosView_->setModel (FilesModel_);
	}

	void UploadPhotosDialog::on_SelectAlbumButton__released ()
	{
		SelectAlbumDialog dia (Acc_);
		if (dia.exec () != QDialog::Accepted)
			return;

		SelectedCollection_ = dia.GetSelectedCollection ();
		Ui_.AlbumName_->setText (SelectedCollection_.data (CollectionRole::Name).toString ());
	}

	void UploadPhotosDialog::on_AddPhotoButton__released ()
	{
		const auto& filenames = QFileDialog::getOpenFileNames (this,
				tr ("Select photos to upload"),
				QDir::homePath (),
				tr ("Photos (*.jpg);;Images (*.jpg *.png *.gif);;All files (*.*)"));

		for (const auto& filename : filenames)
		{
			const QPixmap orig (filename);
			const auto& scaled = orig.scaled (Ui_.PhotosView_->gridSize () - QSize (32, 32),
					Qt::KeepAspectRatio, Qt::SmoothTransformation);
			auto item = new QStandardItem (scaled, QFileInfo (filename).fileName ());

			FilesModel_->appendRow (item);
		}
	}
}
}