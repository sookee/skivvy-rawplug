/*
 * plugin-rawplug.cpp
 *
 *  Created on: 29 Jul 2011
 *      Author: oaskivvy@gmail.com
 */

/*-----------------------------------------------------------------.
| Copyright (C) 2011 SooKee oaskivvy@gmail.com               |
'------------------------------------------------------------------'

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

http://www.gnu.org/licenses/gpl-2.0.html

'-----------------------------------------------------------------*/

#include <skivvy-rawplug/plugin-rawplug.h>

#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

//#include <unistd.h>

#include <skivvy/logrep.h>
#include <skivvy/str.h>

namespace skivvy { namespace ircbot {

IRC_BOT_PLUGIN(RawplugIrcBotPlugin);
PLUGIN_INFO("Comment Grabber", "0.2");

using namespace skivvy::types;
using namespace skivvy::utils;
using namespace skivvy::string;

const str EXEC = "rawplug.exec";

struct entry
{
	str stamp;
	str nick;
	str text;
	entry(const quote& q);
	entry(const str& stamp, const str& nick, const str& text);
};

entry::entry(const quote& q)
: nick(q.msg.get_nick())
, text(q.msg.text)
{
	std::ostringstream oss;
	oss << q.stamp;
	stamp = oss.str();
}

entry::entry(const str& stamp, const str& nick, const str& text)
: stamp(stamp)
, nick(nick)
, text(text)
{
}

RawplugIrcBotPlugin::RawplugIrcBotPlugin(IrcBot& bot)
: BasicIrcBotPlugin(bot), max_quotes(100)
{
}

RawplugIrcBotPlugin::~RawplugIrcBotPlugin() {}

void RawplugIrcBotPlugin::exec(const message& msg)
{
	BUG_COMMAND(msg);

}

// INTERFACE: BasicIrcBotPlugin

bool RawplugIrcBotPlugin::initialize()
{
	str_vec execs = bot.get_vec(EXEC);

	for(str& exec: execs)
	{
		log("loading exec: " << exec);
		FILE* p = popen(exec, "rw");

		add
		({
			"!grab"
			, "!grab <nick> [<number>|<text>] Grab what <nick> said"
				" an optional <number> of comments back, OR that contains <text>."
			, [&](const message& msg){ exec(msg); }
		});
	}
	add
	({
		"!rq"
		, "!rq [<nick>] request a previously grabbed quote, optionally from a specific nick."
		, [&](const message& msg){ rq(msg); }
	});
	bot.add_monitor(*this);
	return true;
}

// INTERFACE: IrcBotPlugin

str RawplugIrcBotPlugin::get_name() const { return NAME; }
str RawplugIrcBotPlugin::get_version() const { return VERSION; }

void RawplugIrcBotPlugin::exit()
{
//	bug_func();
}

// INTERFACE: IrcBotMonitor

void RawplugIrcBotPlugin::event(const message& msg)
{
	mtx_quotes.lock();
	if(msg.cmd == "PRIVMSG") quotes.push_front(msg);
	while(quotes.size() > max_quotes) quotes.pop_back();
	mtx_quotes.unlock();
}

}} // sookee::ircbot
