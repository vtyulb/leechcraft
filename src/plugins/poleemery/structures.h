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

#pragma once

#include <memory>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <QStringList>
#include <QDateTime>
#include <QMetaType>
#include <QHash>
#include "oraltypes.h"

namespace LeechCraft
{
namespace Poleemery
{
	enum class AccType
	{
		Cash,
		BankAccount
	};

	QString ToHumanReadable (AccType);

	struct Account
	{
		oral::PKey<int> ID_;
		AccType Type_;
		QString Name_;
		QString Currency_;

		static QString ClassName () { return "Account"; }
	};

	bool operator== (const Account&, const Account&);
	bool operator!= (const Account&, const Account&);
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Poleemery::Account,
		(decltype (LeechCraft::Poleemery::Account::ID_), ID_)
		(LeechCraft::Poleemery::AccType, Type_)
		(QString, Name_)
		(QString, Currency_))

Q_DECLARE_METATYPE (LeechCraft::Poleemery::Account)

namespace LeechCraft
{
namespace Poleemery
{
	enum class EntryType
	{
		Expense,
		Receipt
	};

	struct EntryBase
	{
		oral::PKey<int> ID_;
		oral::References<Account, 0> AccountID_;

		/** This actually price.
		 */
		double Amount_;
		QString Name_;
		QString Description_;
		QDateTime Date_;

		EntryBase ();
		EntryBase (int accId, double amount, const QString& name, const QString& descr, const QDateTime& dt);
		virtual ~EntryBase ();

		virtual EntryType GetType () const = 0;
	};

	typedef std::shared_ptr<EntryBase> EntryBase_ptr;

	struct NakedExpenseEntry : EntryBase
	{
		double Count_;
		QString Shop_;

		QString EntryCurrency_;
		double Rate_;

		static QString ClassName () { return "NakedExpenseEntry"; }

		EntryType GetType () const { return EntryType::Expense; }

		NakedExpenseEntry ();
		NakedExpenseEntry (int accId, double amount,
				const QString& name, const QString& descr, const QDateTime& dt,
				double count, const QString& shop, const QString& currency, double rate);
	};

	struct ExpenseEntry : NakedExpenseEntry
	{
		QStringList Categories_;

		ExpenseEntry ();
		ExpenseEntry (const NakedExpenseEntry&);
		ExpenseEntry (int accId, double amount,
				const QString& name, const QString& descr, const QDateTime& dt,
				double count, const QString& shop, const QString& currency, double rate,
				const QStringList& cats);
	};

	typedef std::shared_ptr<ExpenseEntry> ExpenseEntry_ptr;
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Poleemery::NakedExpenseEntry,
		(decltype (LeechCraft::Poleemery::NakedExpenseEntry::ID_), ID_)
		(decltype (LeechCraft::Poleemery::NakedExpenseEntry::AccountID_), AccountID_)
		(double, Amount_)
		(QString, Name_)
		(QString, Description_)
		(QDateTime, Date_)
		(double, Count_)
		(QString, Shop_)
		(QString, EntryCurrency_)
		(double, Rate_))

namespace LeechCraft
{
namespace Poleemery
{
	struct Category
	{
		oral::PKey<int> ID_;
		oral::Unique<QString> Name_;

		Category ();
		explicit Category (const QString&);

		static QString ClassName () { return "Category"; }
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Poleemery::Category,
		(decltype (LeechCraft::Poleemery::Category::ID_), ID_)
		(decltype (LeechCraft::Poleemery::Category::Name_), Name_))

namespace LeechCraft
{
namespace Poleemery
{
	struct CategoryLink
	{
		oral::PKey<int> ID_;
		oral::References<Category, 0> Category_;
		oral::References<NakedExpenseEntry, 0> Entry_;

		CategoryLink ();
		CategoryLink (const Category&, const NakedExpenseEntry&);

		static QString ClassName () { return "CategoryLink"; }
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Poleemery::CategoryLink,
		(decltype (LeechCraft::Poleemery::CategoryLink::ID_), ID_)
		(decltype (LeechCraft::Poleemery::CategoryLink::Category_), Category_)
		(decltype (LeechCraft::Poleemery::CategoryLink::Entry_), Entry_))

namespace LeechCraft
{
namespace Poleemery
{
	struct ReceiptEntry : EntryBase
	{
		ReceiptEntry ();
		ReceiptEntry (int accId, double amount, const QString& name, const QString& descr, const QDateTime& dt);

		static QString ClassName () { return "ReceiptEntry"; }

		EntryType GetType () const { return EntryType::Receipt; }
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Poleemery::ReceiptEntry,
		(decltype (LeechCraft::Poleemery::ReceiptEntry::ID_), ID_)
		(decltype (LeechCraft::Poleemery::ReceiptEntry::AccountID_), AccountID_)
		(double, Amount_)
		(QString, Name_)
		(QString, Description_)
		(QDateTime, Date_))

namespace LeechCraft
{
namespace Poleemery
{
	struct Rate
	{
		oral::PKey<int> ID_;

		QString Code_;
		QDateTime SnapshotTime_;
		double Rate_;

		static QString ClassName () { return "Rate"; }
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LeechCraft::Poleemery::Rate,
		(decltype (LeechCraft::Poleemery::Rate::ID_), ID_)
		(QString, Code_)
		(QDateTime, SnapshotTime_)
		(double, Rate_))

namespace LeechCraft
{
namespace Poleemery
{
	struct BalanceInfo
	{
		double Total_;
		QHash<int, double> Accs_;
	};

	struct EntryWithBalance
	{
		EntryBase_ptr Entry_;
		BalanceInfo Balance_;
	};
}
}
