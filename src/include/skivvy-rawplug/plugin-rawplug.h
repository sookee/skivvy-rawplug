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

#include <ext/stdio_filebuf.h>

namespace skivvy { namespace ircbot {

using namespace __gnu_cxx;

typedef stdio_filebuf<char> stdiobuf;
typedef stdio_filebuf<wchar_t> wstdiobuf;

//template<typename Char>
//class basic_stdiostream
//: public virtual std::basic_istream<Char>
//, public virtual std::basic_ostream<Char>
//{
//public:
//	typedef Char char_type;
//	typedef std::basic_iostream<char_type> stream_type;
//	typedef stdio_filebuf<char_type> buf_type;
//
//protected:
//	buf_type ibuf;
//	buf_type obuf;
//
//public:
////	basic_stdiostream(): stream_type(&buf) {}
//	basic_stdiostream(int ifd, int ofd)
//	: std::basic_istream<Char>(&ibuf)
//	, std::basic_ostream<Char>(&obuf)
//	, ibuf(ifd, std::ios::in), obuf(ofd, std::ios::out)
//	{
//	}
//};

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
};

typedef basic_stdiostream<char> stdiostream;
typedef basic_stdiostream<wchar_t> wstdiostream;


//class child_io
//{
//private:
//	stdiostream* i;
//	stdiostream* o;
//
//	std::ios::iostate state;
//
//public:
//	child_io(int ip, int op)
//	: i(new stdiostream(ip, std::ios::in))
//	, o(new stdiostream(op, std::ios::out))
//	{
//	}
//
//	operator void*()
//	{
//		if((state & std::ios::failbit) | (state & std::ios::badbit))
//			return 0;
//		return this;
//	}
//
//	child_io& operator<<(const str& s)
//	{
//		if(o)
//			*o << s;
//		else
//			state |= std::ios::badbit;
//		return *this;
//	}
//
//	child_io& operator>>(str& s)
//	{
//		if(i)
//			*i >> s;
//		else
//			state |= std::ios::badbit;
//		return *this;
//	}
//
//	child_io& getline(str& s, char delim = '\n')
//	{
//		if(i)
//			sgl(*i, s, delim);
//		else
//			state |= std::ios::badbit;
//		return *this;
//	}
//};

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

	str_map cmds; // !cmd -> id
	str_map raw_cmds; // !rawcmd -> id
	str_set monitors; // id, id, id
	std::map<str, stdiostream*> stdis; // id -> stdiostream*
	std::map<str, stdiostream*> stdos; // id -> stdiostream*
	std::vector<std::future<void>> futures;

	str_map names; // id -> name
	str_map versions; // id -> version

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
