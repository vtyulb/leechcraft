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

#include "structures.h"
#include <QtDebug>

namespace LeechCraft
{
namespace Poleemery
{
	QString ToHumanReadable (AccType type)
	{
		switch (type)
		{
		case AccType::BankAccount:
			return QObject::tr ("bank account");
		case AccType::Cash:
			return QObject::tr ("cash");
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown account type"
				<< static_cast<int> (type);
		return QString ();
	}

	bool operator== (const Account& a1, const Account& a2)
	{
		return a1.ID_ == a2.ID_ &&
				a1.Type_ == a2.Type_ &&
				a1.Name_ == a2.Name_ &&
				a1.Currency_ == a2.Currency_;
	}

	bool operator!= (const Account& a1, const Account& a2)
	{
		return !(a1 == a2);
	}

	EntryBase::EntryBase ()
	: ID_ { -1 }
	, AccountID_ { -1 }
	{
	}

	EntryBase::EntryBase (int accId, double amount, const QString& name, const QString& descr, const QDateTime& dt)
	: ID_ { -1 }
	, AccountID_ { accId }
	, Amount_ { amount }
	, Name_ { name }
	, Description_ { descr }
	, Date_ { dt }
	{
	}

	EntryBase::~EntryBase ()
	{
	}

	NakedExpenseEntry::NakedExpenseEntry ()
	{
	}

	NakedExpenseEntry::NakedExpenseEntry (int accId, double amount,
			const QString& name, const QString& descr, const QDateTime& dt,
			double count, const QString& shop)
	: EntryBase { accId, amount, name, descr, dt }
	, Count_ { count }
	, Shop_ { shop }
	{
	}

	ExpenseEntry::ExpenseEntry ()
	{
	}

	ExpenseEntry::ExpenseEntry (const NakedExpenseEntry& naked)
	: NakedExpenseEntry (naked)
	{
	}

	ExpenseEntry::ExpenseEntry (int accId, double amount, const QString& name, const QString& descr, const QDateTime& dt, double count, const QString& shop, const QStringList& cats)
	: NakedExpenseEntry { accId, amount, name, descr, dt, count, shop }
	, Categories_ { cats }
	{
	}

	Category::Category ()
	: ID_ { -1 }
	{
	}

	Category::Category (const QString& name)
	: ID_ { -1 }
	, Name_ { name }
	{
	}

	CategoryLink::CategoryLink ()
	: ID_ { -1 }
	{
	}

	CategoryLink::CategoryLink (const Category& category, const NakedExpenseEntry& entry)
	: ID_ { -1 }
	, Category_ { category.ID_ }
	, Entry_ { entry.ID_ }
	{
	}

	ReceiptEntry::ReceiptEntry ()
	{
	}

	ReceiptEntry::ReceiptEntry (int accId, double amount, const QString& name, const QString& descr, const QDateTime& dt)
	: EntryBase { accId, amount, name, descr, dt }
	{
	}
}
}