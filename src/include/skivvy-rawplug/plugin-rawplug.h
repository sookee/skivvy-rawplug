#pragma once
#ifndef _SKIVVY_IRCBOT_RAWPLUG_H_
#define _SKIVVY_IRCBOT_RAWPLUG_H_
/*
 * plugin-rawplug.h
 *
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

#include <ext/stdio_filebuf.h>

namespace skivvy { namespace ircbot {

using namespace __gnu_cxx;

typedef stdio_filebuf<char> stdiobuf;
typedef stdio_filebuf<wchar_t> wstdiobuf;

template<typename Char>
class basic_stdiostream
: public std::basic_iostream<Char>
{
public:
	typedef Char char_type;
	typedef std::basic_iostream<char_type> stream_type;
	typedef stdio_filebuf<char_type> buf_type;

protected:
	buf_type buf;

public:
	basic_stdiostream(int fd, const std::ios::openmode& mode)
	: stream_type(&buf)
	, buf(fd, mode)
	{
	}

	void close()
	{
		::close(buf.fd()); // interrupt blocking?
	}
};

typedef basic_stdiostream<char> stdiostream;
typedef basic_stdiostream<wchar_t> wstdiostream;


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
	//FILE_vec pipe;

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

	typedef std::map<str, st_time_point> str_time_point_map;
	typedef std::pair<const str, st_time_point> str_time_point_pair;
	typedef std::map<str, std::chrono::seconds> str_sec_map;
	typedef std::pair<const str, std::chrono::seconds> str_sec_pair;

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

}} // sookee::ircbot

#endif // _SKIVVY_IRCBOT_RAWPLUG_H_
