/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "ircparser.h"
#include <boost/bind.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <QTextCodec>
#include "ircaccount.h"
#include "ircserverhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	using namespace boost::spirit::classic;

	IrcParser::IrcParser (IrcServerHandler *sh)
	: ISH_ (sh)
	, ServerOptions_ (sh->GetServerOptions ())
	{
	}

	void IrcParser::AuthCommand ()
	{
		QString pass = ServerOptions_.ServerPassword_;
		if (!pass.isEmpty ())
		{
			QString passCmd = QString ("PASS " + pass + "\r\n");
			ISH_->SendCommand (passCmd);
		}

		UserCommand ();
		NickCommand (QStringList () << ISH_->GetNickName ());
	}

	void IrcParser::UserCommand ()
	{
		QString userCmd = QString ("USER " +
				ISH_->GetAccount ()->GetUserName () + " 8 * :" +
				ISH_->GetAccount ()->GetRealName () + "\r\n");
		ISH_->SendCommand (userCmd);
	}

	void IrcParser::NickCommand (const QStringList& nick)
	{
		QString nickCmd = QString ("NICK " + nick.at (0) + "\r\n");
		ISH_->SendCommand (nickCmd);
	}

	void IrcParser::JoinCommand (const QString& channel)
	{
		//TODO Password
		QString joinCmd = QString ("JOIN " + channel + "\r\n");
		ISH_->SendCommand (joinCmd);
	}

	void IrcParser::PrivMsgCommand (const QString& msg,
			const QString& target)
	{
		QString privmsgCmd = QString ("PRIVMSG " + target + " :" +
				msg + "\r\n");
		ISH_->SendCommand (privmsgCmd);
	}

	void IrcParser::PartCommand (const QStringList& cmd)
	{
		QString partCmd = QString ("PART " + cmd.first () + " :" +
				QStringList (cmd.mid (1)).join (" ") + "\r\n");
		ISH_->SendCommand (partCmd);
	}

	void IrcParser::PongCommand (const QString& msg)
	{
		QString pongCmd = QString ("PONG :" + msg + "\r\n");
		ISH_->SendCommand (pongCmd);
	}

	void IrcParser::RawCommand (const QStringList& cmd)
	{
		QString rawCmd = cmd.join (" ") + "\r\n";
		ISH_->SendCommand (rawCmd);
	}

	void IrcParser::CTCPRequest (const QStringList& cmd)
	{
		if (!cmd.count ())
			return;
		QString ctcpCmd;
		if (cmd.count () > 2)
			ctcpCmd = "PRIVMSG " + cmd.first () + " :\001" +
					cmd.at (1) + " " + QStringList (cmd.mid (2))
							.join (" ") + "\001\r\n";
		else
			ctcpCmd = "PRIVMSG " + cmd.first () + " :\001" +
					cmd.at (1) + "\001\r\n";
		ISH_->SendCommand (ctcpCmd);
	}

	void IrcParser::CTCPReply (const QStringList& cmd)
	{
		if (!cmd.count ())
			return;

		QString ctcpCmd = QString ("NOTICE " + cmd.first () + " :" +
				cmd.last () + "\r\n");
		ISH_->SendCommand (ctcpCmd);
	}

	void IrcParser::TopicCommand (const QStringList& cmd)
	{
		if (!cmd.count ())
			return;

		QString topicCmd;
		switch (cmd.count ())
		{
		case 1:
			topicCmd = QString ("TOPIC " + cmd.first () + "\r\n");
			break;
		case 2:
			topicCmd = QString ("TOPIC " + cmd.first () + " " +
					cmd.last () + "\r\n");
			break;
		default:
			topicCmd = QString ("TOPIC " + cmd.first () + " :" +
					QStringList (cmd.mid (1)).join (" ") + "\r\n");
		}
		ISH_->SendCommand (topicCmd);
	}

	void IrcParser::NamesCommand (const QStringList& cmd)
	{
		QString target = QString ();
		if (cmd.count ())
			target =  cmd.first ();

		QString namesCmd = QString ("NAMES " + target + "\r\n");
		ISH_->SendCommand (namesCmd);
	}

	void IrcParser::InviteCommand (const QStringList& cmd)
	{
		QString inviteCmd = QString ("INVITE " + cmd.join (" ") +
				"\r\n");
		ISH_->SendCommand (inviteCmd);
	}

	void IrcParser::KickCommand (const QStringList& cmd)
	{
		QString kickCmd = QString ("KICK " + cmd.first () + " " +
				cmd.at (1) + " :" + QStringList (cmd.mid (2)).join (" ")
				+ "\r\n");
		ISH_->SendCommand (kickCmd);
	}

	void IrcParser::OperCommand (const QStringList& cmd)
	{
		QString operCmd = QString ("OPER " + cmd.at (0) + " :" +
				QStringList (cmd.mid (1)).join (" ") + "\r\n");
		ISH_->SendCommand (operCmd);
	}

	void IrcParser::SQuitCommand (const QStringList& cmd)
	{
		QString squitCmd = QString ("SQUIT " + cmd.first () + " :" +
				QStringList (cmd.mid (1)).join (" ") + "\r\n");
		ISH_->SendCommand (squitCmd);
	}

	void IrcParser::MOTDCommand (const QStringList& cmd)
	{
		QString motdCmd;
		if (!cmd.count ())
			motdCmd = QString ("MOTD\r\n");
		else
			motdCmd = QString ("MOTD " + cmd.first () + "\r\n");

		ISH_->SendCommand (motdCmd);
	}

	void IrcParser::LusersCommand (const QStringList& cmd)
	{
		QString lusersCmd = QString ("LUSERS " + cmd.join (" ") +
				"\r\n");
		ISH_->SendCommand (lusersCmd);
	}

	void IrcParser::VersionCommand (const QStringList& cmd)
	{
		QString versionCmd = QString ("VERSION " + cmd.join (" ") +
				"\r\n");
		ISH_->SendCommand (versionCmd);
	}

	bool IrcParser::ParseMessage (const QString& message)
	{
		IrcMessageOptions_.Command_.clear ();
		IrcMessageOptions_.Nick_.clear ();
		IrcMessageOptions_.Message_.clear ();
		IrcMessageOptions_.Parameters_.clear ();

		std::string nickStr;
		std::string commandStr;
		std::string msgStr;
		QList<std::string> opts;

		range<> ascii (char (0x01), char (0x7F));
		rule<> special = lexeme_d [ch_p ('[') | ']' | '\\' | '`' |
				'_' | '^' | '{' | '|' | '}'];
		rule<> shortname = *(alnum_p
				>> *(alnum_p || ch_p ('-'))
				>> *alnum_p);
		rule<> hostname = shortname
				>> *(ch_p ('.')
				>> shortname);
		rule<> nickname = (alpha_p | special)
				>> * (alnum_p | special | ch_p ('-'));
		rule<> user =  +(ascii - '\r' - '\n' - ' ' - '@' - '\0');
		rule<> host = lexeme_d [+(anychar_p - ' ')] ;
		rule<> nick = lexeme_d [nickname [assign_a (nickStr)]
				>> !(!(ch_p ('!')
				>> user)
				>> ch_p ('@')
				>> host)];
		rule<> nospcrlfcl = (anychar_p - '\0' - '\r' - '\n' -
				' ' - ':');
		rule<> lastParam = lexeme_d [ch_p (' ')
				>> !ch_p (':')
				>> (*(ch_p (':') | ch_p (' ') | nospcrlfcl))
					[assign_a (msgStr)]];
		rule<> firsParam = lexeme_d [ch_p (' ')
				>> (nospcrlfcl
				>> *(ch_p (':') | nospcrlfcl))
					[push_back_a (opts)]];
		rule<> params =  *firsParam
				>> !lastParam;
		rule<> command = longest_d [(+alpha_p) |
				(repeat_p (3) [digit_p])][assign_a (commandStr)];
		rule<> prefix = longest_d [hostname | nick];
		rule<> reply = (lexeme_d [!(ch_p (':')
				>> prefix >> ch_p (' '))]
				>> command
				>> !params
				>> eol_p);

		bool res = parse (message.toUtf8 ().constData (), reply).full;

		if (!res)
		{
			qWarning () << "input string is not a valide IRC command"
					<< message;
			return false;
		}
		else
		{
			IrcMessageOptions_.Nick_ =
					QString::fromUtf8 (nickStr.c_str ());
			IrcMessageOptions_.Command_ =
					QString::fromUtf8 (commandStr.c_str ()).toLower ();
			IrcMessageOptions_.Message_ =
					QString::fromUtf8 (msgStr.c_str ());
			IrcMessageOptions_.Parameters_ = opts;
		}

		return true;
	}

	IrcMessageOptions IrcParser::GetIrcMessageOptions () const
	{
		return IrcMessageOptions_;
	}

};
};
};
