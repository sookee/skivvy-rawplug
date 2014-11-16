#ifndef _SKIVVY_PLUGIN_RAWPLUG_H_
#define _SKIVVY_PLUGIN_RAWPLUG_H_
/*
 *  Created on: 10 Dec 2012
 *      Author: oaskivvy@gmail.com
 */

/*-----------------------------------------------------------------.
| Copyright (C) 2012 SooKee oaskivvy@gmail.com               |
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

#include <skivvy/ircbot.h>

#include <deque>
#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <utility>

#include <sookee/ios.h>

namespace skivvy { namespace rawplug {

using namespace skivvy::ircbot;
using namespace sookee::ios;

/**
 * PROPERTIES: (Accesed by: bot.props["property"])
 *
 * grabfile - location of grabbed text file "grabfile.txt"
 *
 */
class RawplugIrcBotPlugin
: public BasicIrcBotPlugin
 , public IrcBotMonitor
{
public:
	typedef std::vector<FILE*> FILE_vec;

private:
	std::mutex mtx;

	typedef std::shared_ptr<stdiostream> stdiostream_sptr;

	str_map cmds; // !cmd -> id
	str_map raw_cmds; // !rawcmd -> id
	str_set monitors; // id, id, id
	str_set raw_monitors; // id, id, id

	std::map<str, stdiostream_sptr> stdis; // id -> stdiostream*
	std::map<str, stdiostream_sptr> stdos; // id -> stdiostream*

	std::vector<std::future<void>> futures;

	str_map names; // id -> name
	str_map versions; // id -> version
	str_map protocols; // id -> protocol

	str_set responder_ids; // id of loaded rawplug tnat need a responder

	typedef std::map<str, st_time_point> str_time_point_map;
	typedef std::map<str, std::chrono::seconds> str_sec_map;

	std::mutex poll_mtx;
	std::future<void> poll_fut;
	str_sec_map pollsecs; // id -> poll-time_secs
	str_time_point_map pollnows; // id -> time of last poll
	bool poll();

	bool done = false;
	bool responder(const str& id);
	bool exec(const message& msg);
	bool open_plugin(const str& dir, const str& exec);

public:
	RawplugIrcBotPlugin(IrcBot& bot);
	virtual ~RawplugIrcBotPlugin();

	// INTERFACE: BasicIrcBotPlugin

	virtual bool initialize();

	// INTERFACE: IrcBotPlugin

	virtual std::string get_id() const;
	virtual std::string get_name() const;
	virtual std::string get_version() const;
	virtual void exit();

	// INTERFACE: IrcBotMonitor

	virtual void event(const message& msg);
};

}} // skivvy::rawplug

#endif // _SKIVVY_PLUGIN_RAWPLUG_H_
