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

#include <skivvy/plugin-rawplug.h>

#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <ext/stdio_filebuf.h>

#include <skivvy/logrep.h>
#include <skivvy/ios.h>
#include <skivvy/str.h>

namespace skivvy { namespace rawplug {

IRC_BOT_PLUGIN(RawplugIrcBotPlugin);
PLUGIN_INFO("rawplug", "Raw Plugin Interface", "0.2");

using namespace skivvy::types;
using namespace skivvy::utils;
using namespace skivvy::string;
using namespace __gnu_cxx;

const str PLUGIN_DIR = "rawplug.dir";
const str PLUGIN_EXE = "rawplug.exe";

bool log_report(const str& msg, bool err = true)
{
	log(msg);
	return err;
}

RawplugIrcBotPlugin::RawplugIrcBotPlugin(IrcBot& bot)
: BasicIrcBotPlugin(bot)
{
}

RawplugIrcBotPlugin::~RawplugIrcBotPlugin() {}

bool RawplugIrcBotPlugin::responder(const str& id)
{
	if(!stdis[id].get())
		return log_report("Bad plugin: " + id);
	str line;
	while(!done)
	{
		{
//			lock_guard lock(mtx);
			if(!sgl(*stdis[id], line))
				return log_report("Error reading rawplug: " + id);
		}
		bug_var(line);
		if(!line.find("/log"))
		{
			line = line.substr(4);
			log(trim(line));
		}
		else
		{
			soss oss;
			bot.exec(line, &oss);
		}
	}
	return true;
}

bool RawplugIrcBotPlugin::exec(const message& msg)
{
	lock_guard lock(mtx);
	BUG_COMMAND(msg);
	// send msg to appropriate plugin based upon msg.cmd
	bool raw = false;

	str id;
	str_map::iterator it;
	if((it = cmds.find(msg.get_user_cmd())) != cmds.end())
		id = it->second;
	else if((it = raw_cmds.find(msg.get_user_cmd())) != raw_cmds.end() && (raw = true))
		id = it->second;
	else
		return log_report("Unknown command: " + msg.line);

	bug_var(raw);

	if(!stdos[id].get())
		return log_report("No communication wth this plugin: ");

	std::ostream& stdo = *stdos[id];

	stdo << msg.get_user_cmd() << std::endl;
	stdo << msg.line << std::endl;
	if(!raw)
	{
		stdo << msg.from << std::endl;
		stdo << msg.cmd << std::endl;
		stdo << msg.params << std::endl;
		stdo << msg.to << std::endl;
		stdo << msg.text << std::endl;
	}

	return true;
}

//bool RawplugIrcBotPlugin::open_plugin(const str& dir, const str& exec)
//{
//	bug_func();
//	lock_guard lock(mtx);
//	log("loading exec: " << exec);
//
//	pid_t pid;
//	int pipe_in[2]; /* This is the pipe with wich we write to the child process. */
//	int pipe_out[2]; /* This is the pipe with wich we read from the child process. */
//
//	if(pipe(pipe_in) || pipe(pipe_out))
//		return log_report(strerror(errno));
//
//	/* Attempt to fork and check for errors */
//	if((pid = fork()) == -1)
//		return log_report(strerror(errno));
//
//	if(pid)
//	{
//		stdiostream_sptr stdip(new stdiostream(pipe_out[0], std::ios::in));
//		stdiostream_sptr stdop(new stdiostream(pipe_in[1], std::ios::out));
//
//		close(pipe_out[1]);
//		close(pipe_in[0]);
//
//		if(!stdip.get())
//			return log_report(("Unable to create stdiostream object."));
//		if(!stdop.get())
//			return log_report(("Unable to create stdiostream object."));
//
//		stdiostream& stdi = *stdip.get();
//		stdiostream& stdo = *stdop.get();
//
//		str line, id, name, version;
//
//		// initialize
//		stdo << "get_id" << std::endl;
//		if(!sgl(stdi, id) || id.empty())
//			return log_report("Error, expected plugin id, got: " + id);
//
//		stdo << "get_name" << std::endl;
//		if(!sgl(stdi, name) || name.empty())
//			return log_report("Error, expected plugin name, got: " + name);
//
//		stdo << "get_version" << std::endl;
//		if(!sgl(stdi, version) || version.empty())
//			return log_report("Error, expected plugin version, got: " + line);
//
//		names[id] = name;
//		versions[id] = version;
//
//		stdo << "initialize" << std::endl;
//		while(sgl(stdi, line) && line != "end_initialize")
//		{
//			bool raw_cmd = false;
//			bool raw_mon = false;
//			if(line == "add_command" || (raw_cmd = (line == "add_raw_command")))
//			{
//				str cmd;
//				if(!sgl(stdi, line) || line.empty() || line[0] != '!')
//					return log_report("Error, expected command name, got: " + line);
//
//				cmd = line;
//				str sep, help;
//				while(sgl(stdi, line) && line != "end_command")
//				{
//					help += sep + line;
//					sep = '\n';
//				}
//				if(raw_cmd)
//				{
//					cmds.erase(id);
//					raw_cmds[cmd] = id;
//				}
//				else
//				{
//					raw_cmds.erase(id);
//					cmds[cmd] = id;
//				}
//
//				add
//				({
//					cmd, help, [&](const message& msg){ this->exec(msg); }
//				});
//			}
//			else if(line == "add_monitor" || (raw_mon = (line == "add_raw_command")))
//			{
//				// Ensure only rw_monitors or monitors but not both
//				bot.add_monitor(*this);
//				if(raw_mon)
//				{
//					monitors.erase(id);
//					raw_monitors.insert(id);
//				}
//				else
//				{
//					raw_monitors.erase(id);
//					monitors.insert(id);
//				}
//			}
//		}
//		stdis[id] = stdip;
//		stdos[id] = stdop;
//		futures.push_back(std::async(std::launch::async, [=]{ responder(id); }));
//	}
//	else
//	{
//		/* The child has the zero pid returned by fork*/
//		close(1);
//		dup(pipe_out[1]); /* dup uses the lowest numbered unused file descriptor as new descriptor. In our case this now is 1. */
//
//		close(0); /* dup uses the lowest numbered unused file descriptor as new descriptor. In our case this now is 0. */
//		dup(pipe_in[0]);
//
//		close(pipe_out[0]);
//		close(pipe_out[1]);
//		close(pipe_in[0]);
//		close(pipe_in[1]);
//
//		str plugin_name = dir + "/" + exec;
//		execl(plugin_name.c_str(), plugin_name.c_str(), (char*)0);
//		return false; /* Only reached if execl() failed */
//	}
//
//	return true;
//}

bool RawplugIrcBotPlugin::poll()
{
	while(!done)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		lock_guard lock(poll_mtx);
//		siz now = std::time(0);
		st_time_point now = st_clk::now(); //std::time(0);
		//bug_var(now);
		for(str_time_point_pair& p: pollnows)
		{
			//bug_var(p.first);
			if(pollsecs[p.first] == std::chrono::seconds(0))
				continue;
			//bug_var(pollnows[p.first]);
			if(now - pollnows[p.first] < pollsecs[p.first])
				continue;
			//bug_var(stdos[p.first]);
			if(!stdos[p.first])
				continue;
			*stdos[p.first] << "poll" << std::endl;
			pollnows[p.first] = now;
		}
	}
	return true;
}

bool RawplugIrcBotPlugin::open_plugin(const str& dir, const str& exec)
{
	bug_func();
	lock_guard lock(mtx);
	log("loading exec: " << exec);

	pid_t pid;
	int pipe_in[2]; /* This is the pipe with wich we write to the child process. */
	int pipe_out[2]; /* This is the pipe with wich we read from the child process. */

	if(pipe(pipe_in) || pipe(pipe_out))
		return log_report(strerror(errno));

	/* Attempt to fork and check for errors */
	if((pid = fork()) == -1)
		return log_report(strerror(errno));

	if(pid)
	{
		stdiostream_sptr stdip(new stdiostream(pipe_out[0], std::ios::in));
		stdiostream_sptr stdop(new stdiostream(pipe_in[1], std::ios::out));

		close(pipe_out[1]);
		close(pipe_in[0]);

		if(!stdip.get())
			return log_report(("Unable to create stdiostream object."));
		if(!stdop.get())
			return log_report(("Unable to create stdiostream object."));

		stdiostream& stdi = *stdip.get();
		//stdiostream& stdo = *stdop.get();

		str skip, line, id, name, version;

		// initialize

		if(!sgl(stdi, line) || line != "initialize")
			return log_report("Expected 'initialize' from plugin, got: " + line);
		bug_var(line);

		if(!sgl(stdi, line) || line.find("id:"))
			return log_report("Expected id: from plugin, got: " + line);
		sgl(sgl(siss(line), skip, ':') >> std::ws, id);
		bug_var(line);

		if(!sgl(stdi, line) || line.find("name:"))
			return log_report("Expected name: from plugin, got: " + line);
		sgl(sgl(siss(line), skip, ':') >> std::ws, name);
		bug_var(line);

		if(!sgl(stdi, line) || line.find("version:"))
			return log_report("Expected version: from plugin, got: " + line);
		sgl(sgl(siss(line), skip, ':') >> std::ws, version);
		bug_var(line);

		names[id] = name;
		versions[id] = version;

		while(sgl(stdi, line) && line != "end_initialize")
		{
			bug_var(line);
			bool raw_cmd = false;
			bool raw_mon = false;
			if(line == "add_command" || (raw_cmd = (line == "add_raw_command")))
			{
				str cmd;
				if(!sgl(stdi, line) || line.empty() || line[0] != '!')
					return log_report("Error, expected command name, got: " + line);

				cmd = line;
				str sep, help;
				while(sgl(stdi, line) && line != "end_command")
				{
					help += sep + line;
					sep = '\n';
				}
				if(raw_cmd)
				{
					cmds.erase(id);
					raw_cmds[cmd] = id;
				}
				else
				{
					raw_cmds.erase(id);
					cmds[cmd] = id;
				}

				add
				({
					cmd, help, [&](const message& msg){ this->exec(msg); }
				});
			}
			else if(line == "add_monitor" || (raw_mon = (line == "add_raw_command")))
			{
				// Ensure only rw_monitors or monitors but not both
				bot.add_monitor(*this);
				if(raw_mon)
				{
					monitors.erase(id);
					raw_monitors.insert(id);
				}
				else
				{
					raw_monitors.erase(id);
					monitors.insert(id);
				}
			}
			else if(!line.find("poll_me:"))
			{
				// Ensure only rw_monitors or monitors but not both
				siz secs;
				if(!(siss(line) >> line >> secs))
					secs = 5 * 60; // five minuted default
				bug_var(secs);
				lock_guard lock(poll_mtx);
				pollsecs[id] = std::chrono::seconds(secs);
				pollnows[id]; //0; // time off last poll
			}
		}
		stdis[id] = stdip;
		stdos[id] = stdop;
		futures.push_back(std::async(std::launch::async, [=]{ responder(id); }));
	}
	else
	{
		/* The child has the zero pid returned by fork*/
		close(1);
		dup(pipe_out[1]); /* dup uses the lowest numbered unused file descriptor as new descriptor. In our case this now is 1. */

		close(0); /* dup uses the lowest numbered unused file descriptor as new descriptor. In our case this now is 0. */
		dup(pipe_in[0]);

		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_in[0]);
		close(pipe_in[1]);

		str plugin_name = dir + "/" + exec;
		execl(plugin_name.c_str(), plugin_name.c_str(), (char*)0);
		return false; /* Only reached if execl() failed */
	}

	return true;
}

// INTERFACE: BasicIrcBotPlugin

bool RawplugIrcBotPlugin::initialize()
{
	bug_func();
	str_vec files;
	for(const str& dir: bot.get_vec(PLUGIN_DIR))
	{
		log("Scanning folder: " << dir);
		if(!ios::ls(dir, files)) {}
//		{
//			log(strerror(errno));
//			continue;
//		}
		for(const str& file: files)
		{
			bug_var(file);
			for(str& exe: bot.get_vec(PLUGIN_EXE))
			{
				bug_var(exe);
				if(exe == file)
					open_plugin(dir, exe);
			}
		}
	}
	poll_fut = std::async(std::launch::async, [this]{ poll(); });

	return true;
}

// INTERFACE: IrcBotPlugin

str RawplugIrcBotPlugin::get_id() const { return ID; }
str RawplugIrcBotPlugin::get_name() const { return NAME; }
str RawplugIrcBotPlugin::get_version() const { return VERSION; }

void RawplugIrcBotPlugin::exit()
{
	done = true;

	// Try clean exit
	for(std::pair<const str, stdiostream_sptr>& p: stdos)
		if(p.second.get())
			*p.second << "exit" << std::endl;

	for(std::future<void>& fut: futures)
		if(fut.valid())
			if(fut.wait_for(std::chrono::seconds(10)) == std::future_status::ready)
				fut.get();

	for(std::pair<const str, stdiostream_sptr>& p: stdis)
		p.second->close();

	for(std::pair<const str, stdiostream_sptr>& p: stdos)
		p.second->close();

	for(std::future<void>& fut: futures)
		if(fut.valid())
			if(fut.wait_for(std::chrono::seconds(10)) == std::future_status::ready)
				fut.get();
	if(poll_fut.valid())
		if(poll_fut.wait_for(std::chrono::seconds(10)) == std::future_status::ready)
			poll_fut.get();
}

// INTERFACE: IrcBotMonitor

void RawplugIrcBotPlugin::event(const message& msg)
{
	// distribute to all monitoring plugins
	for(const str& id: raw_monitors)
		if(stdos[id])
			*stdos[id] << "event" << std::endl << msg.line << std::endl;

	for(const str& id: monitors)
		if(stdos[id])
		{
			*stdos[id] << msg.from << std::endl;
			*stdos[id] << msg.cmd << std::endl;
			*stdos[id] << msg.params << std::endl;
			*stdos[id] << msg.to << std::endl;
			*stdos[id] << msg.text << std::endl;
		}
}

}} // skivvy::rawplug {
