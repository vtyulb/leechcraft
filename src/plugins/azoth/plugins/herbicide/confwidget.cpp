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

#include "confwidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Herbicide
{
	ConfWidget::ConfWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		
		LoadSettings ();
		
		QList<QPair<QString, QStringList>> mathQuests;
		mathQuests << qMakePair (QString ("(cos(x))'"), QStringList ("-sin(x)"));
		mathQuests << qMakePair (QString::fromUtf8 ("e^(iπ)"), QStringList ("-1"));
		PredefinedQuests_ << mathQuests;
	}
	
	QString ConfWidget::GetQuestion () const
	{
		return Ui_.Question_->toPlainText ();
	}
	
	QStringList ConfWidget::GetAnswers () const
	{
		return Ui_.Answers_->toPlainText ()
			.split ('\n', QString::SkipEmptyParts);
	}
	
	void ConfWidget::SaveSettings () const
	{
		XmlSettingsManager::Instance ().setProperty ("Question", GetQuestion ());
		XmlSettingsManager::Instance ().setProperty ("Answers", GetAnswers ());
	}

	void ConfWidget::LoadSettings ()
	{
		const QString& question = XmlSettingsManager::Instance ()
				.property ("Question").toString ();
		Ui_.Question_->setPlainText (question);

		const QStringList& answers = XmlSettingsManager::Instance ()
				.property ("Answers").toStringList ();
		Ui_.Answers_->setPlainText (answers.join ("\n"));
	}
	
	void ConfWidget::accept ()
	{
		SaveSettings ();
	}
	
	void ConfWidget::reject ()
	{
		LoadSettings ();
	}
	
	void ConfWidget::on_QuestStyle__currentIndexChanged (int idx)
	{
		if (PredefinedQuests_.size () <= idx - 1 || !idx)
			return;
		
		Ui_.QuestVariant_->clear ();
		QPair<QString, QStringList> pair;
		Q_FOREACH (pair, PredefinedQuests_.at (idx - 1))
			Ui_.QuestVariant_->addItem (pair.first, pair.second);
	}
	
	void ConfWidget::on_QuestVariant__currentIndexChanged (int idx)
	{
		Ui_.Question_->setPlainText (Ui_.QuestVariant_->currentText ());
		Ui_.Answers_->setPlainText (Ui_.QuestVariant_->itemData (idx).toStringList ().join ("\n"));
	}
}
}
}
