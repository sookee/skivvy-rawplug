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

namespace skivvy { namespace ircbot {

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
	FILE_vec pipe;

	void exec(const message& msg);

public:
	RawplugIrcBotPlugin(IrcBot& bot);
	virtual ~RawplugIrcBotPlugin();

	// INTERFACE: BasicIrcBotPlugin

	virtual bool initialize();

	// INTERFACE: IrcBotPlugin

	virtual std::string get_name() const;
	virtual std::string get_version() const;
	virtual void exit();

	// INTERFACE: IrcBotMonitor

	virtual void event(const message& msg);
};

}} // sookee::ircbot

#endif // _SKIVVY_IRCBOT_RAWPLUG_H_
