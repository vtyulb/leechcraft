/*
	LeechCraft - modular cross-platform feature rich internet client.
	Copyright (C) 2010-2011  Oleg Linkin

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
*/


#include "servercommandmessage.h"
#include <QTextDocument>
#include "ircserverclentry.h"
#include "ircserverhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ServerCommandMessage::ServerCommandMessage (const QString& msg,
			IrcServerCLEntry *entry)
	: QObject (entry)
	, ParentEntry_ (entry)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (DOut)
	, Type_ (MTMUCMessage)
	, SubType_ (MSTOther)
	{
	}

	ServerCommandMessage::ServerCommandMessage (const QString& msg,
			IMessage::Direction dir, IrcServerCLEntry *entry,
			IMessage::MessageType mtype, IMessage::MessageSubType mstype)
	: QObject (entry)
	, ParentEntry_ (entry)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (dir)
	, Type_ (mtype)
	, SubType_ (mstype)
	{
	}

	QObject* ServerCommandMessage::GetQObject ()
	{
		return this;
	}

	void ServerCommandMessage::Send ()
	{
		if (!ParentEntry_)
			return;

		ParentEntry_->GetIrcServerHandler ()->
				SendMessage2Server (Message_.split (' '));
	}

	void ServerCommandMessage::Store ()
	{
		qWarning () << Q_FUNC_INFO
				<< "cannot store ServerCommandMessage";
	}

	IMessage::Direction ServerCommandMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::MessageType ServerCommandMessage::GetMessageType () const
	{
		return Type_;
	}

	void ServerCommandMessage::SetMessageType (IMessage::MessageType t)
	{
		Type_ = t;
	}

	IMessage::MessageSubType ServerCommandMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	void ServerCommandMessage::SetMessageSubType
			(IMessage::MessageSubType type)
	{
		SubType_ = type;
	}

	QObject* ServerCommandMessage::OtherPart () const
	{
		return ParentEntry_;
	}

	QObject* ServerCommandMessage::ParentCLEntry () const
	{
		return ParentEntry_;
	}

	QString ServerCommandMessage::GetOtherVariant () const
	{
		return FromVariant_;
	}

	QString ServerCommandMessage::GetBody () const
	{
		return Qt::escape (Message_);
	}

	void ServerCommandMessage::SetBody (const QString& msg)
	{
		Message_ = msg;
	}

	QDateTime ServerCommandMessage::GetDateTime () const
	{
		return Datetime_;
	}

	void ServerCommandMessage::SetDateTime (const QDateTime& dt)
	{
		Datetime_ = dt;
	}
}
}
}
