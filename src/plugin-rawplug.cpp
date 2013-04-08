/*
 * plugin-rawplug.cpp
 *
 *  Created on: 29 Jul 2011
 *      Author: oaskivvy@gmail.com
 */

/*-----------------------------------------------------------------.
| Copyright (C) 2011 SooKee oaskivvy@gmail.com                     |
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

// ulimit stuff
#include <sys/time.h>
#include <sys/resource.h>

#include <ext/stdio_filebuf.h>

#include <sookee/str.h>
#include <sookee/bug.h>

#include <skivvy/utils.h>
#include <skivvy/logrep.h>
#include <skivvy/ios.h>

namespace skivvy { namespace rawplug {

IRC_BOT_PLUGIN(RawplugIrcBotPlugin);
PLUGIN_INFO("rawplug", "Raw Plugin Interface", "0.2");

using namespace skivvy::types;
using namespace skivvy::utils;
using namespace sookee::string;
using namespace sookee::bug;
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
		if(!sgl(*stdis[id], line))
			return log_report("Error reading rawplug: " + id);
		trim(line);
		if(line.empty())
		{
			log("rawplug: WARN: empty command.");
			continue;
		}
		bug("responding to: " << line);
		if(!line.find("/log") && line.size() > 4)
		{
			bug("/log:");
			log(id << ": " << line.substr(4));
		}
		else if(!line.find("/bug") && line.size() > 4)
		{
			bug("/bug:");
			bug(id << ": " << line.substr(4));
		}
		else
		{
			bug("/other:");
			soss oss;
			bot.exec(line, &oss);
		}
		bug("/done:");
	}
	log("\t\tresponder closing down: " << id);
	return true;
}

bool RawplugIrcBotPlugin::exec(const message& msg)
{
//	lock_guard_x(lock, mtx);
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
		str_vec middles;
		str params, trailing, sep;
		msg.get_params(middles, trailing);

		for(const str& middle: middles)
			{ params += sep + middle; sep = " "; }

		stdo << msg.prefix << std::endl;
		stdo << msg.command << std::endl;
		stdo << params << std::endl;
		stdo << msg.get_to() << std::endl;
		stdo << trailing << std::endl;
	}

	return true;
}

bool RawplugIrcBotPlugin::poll()
{
	while(!done)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		st_time_point now = st_clk::now(); //std::time(0);
		for(str_time_point_pair& p: pollnows)
		{
			if(pollsecs[p.first] == std::chrono::seconds(0))
				continue;
			if(now - pollnows[p.first] < pollsecs[p.first])
				continue;
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
//	lock_guard_x(lock, mtx);
	lock_guard lock(mtx);
	log("loading exec: " << exec);

	pid_t pid;
	int pipe_in[2]; // write to child
	int pipe_out[2]; // red from child

	if(pipe(pipe_in) || pipe(pipe_out))
		return log_report(strerror(errno));

	/* Attempt to fork and check for errors */
	if((pid = fork()) == -1)
		return log_report(strerror(errno));

	if(pid)
	{
		// parent

//		rlimit olim;
//		rlimit nlim;
//		nlim.rlim_max = 20;
//		nlim.rlim_cur = 20;
//
//		prlimit(pid, RLIMIT_NPROC, &nlim, &olim);

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

		names[id] = name;
		versions[id] = version;
		protocols[id] = "0.0";

		enum
		{
			GOT_INITIALIZE = 0x01
			, GOT_ID = 0x02
			, GOT_NAME = 0x04
			, GOT_VERSION = 0x08
			, GOT_HEADER = GOT_INITIALIZE|GOT_ID|GOT_NAME|GOT_VERSION
			, GOT_ERROR = 0x8000
		};

		siz status = 0;

		while(sgl(stdi, line) && line != "end_initialize")
		{
			bug_var(line);
			bool raw_cmd = false;
			bool raw_mon = false;

			if(!line.find("initialize"))
			{
				status |= GOT_INITIALIZE;
				str protocol = "0.0";
				if((siss(line) >> protocol >> protocol))
					protocols[id] = protocol;

			}
			else if(!line.find("error:"))
			{
				status |= GOT_ERROR;
				sgl(siss(line) >> line >> std::ws, line);
				return log_report("ERROR: " + line);
			}
			else if(!line.find("id:"))
			{
				status |= GOT_ID;
				sgl(siss(line) >> line >> std::ws, id);
			}
			else if(!line.find("name:"))
			{
				status |= GOT_NAME;
				sgl(siss(line) >> line >> std::ws, name);
			}
			else if(!line.find("version:"))
			{
				status |= GOT_VERSION;
				sgl(siss(line) >> line >> std::ws, version);
			}
			else if(line == "add_command" || (raw_cmd = (line == "add_raw_command")))
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
//				lock_guard_x(lock, poll_mtx);
				lock_guard lock(poll_mtx);
				pollsecs[id] = std::chrono::seconds(secs);
				pollnows[id]; //0; // time of last poll
			}
		}
		if(status & GOT_HEADER) // all header info received
		{
//			lock_guard_x(lock, mtx);
			stdis[id] = stdip;
			stdos[id] = stdop;
//			futures.push_back(std::async(std::launch::async, [=]{ responder(id); }));
			responder_ids.insert(id);
		}
	}
	else
	{
		// child

		rlimit lim;
		if(!getrlimit(RLIMIT_NPROC, &lim))
		{
			lim.rlim_max += 10;
			lim.rlim_cur += 10;
			setrlimit(RLIMIT_NPROC, &lim);
		}

		close(1);
		dup(pipe_out[1]); // lowest unused fd

		close(0);
		dup(pipe_in[0]); // lowest unused fd

		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_in[0]);
		close(pipe_in[1]);

		str plugin_name = dir + "/" + exec;
		chdir(dir.c_str());
		execl(plugin_name.c_str(), plugin_name.c_str(), (char*)0);
		return false; // execl() failed
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
//			bug_var(file);
			for(str& exe: bot.get_vec(PLUGIN_EXE))
			{
//				bug_var(exe);
				if(exe == file)
					open_plugin(dir, exe);
			}
		}
	}
	for(const str& id: responder_ids)
		futures.push_back(std::async(std::launch::async, [=]{ responder(id); }));
	poll_fut = std::async(std::launch::async, [this]{ poll(); });

	return true;
}

// INTERFACE: IrcBotPlugin

str RawplugIrcBotPlugin::get_id() const { return ID; }
str RawplugIrcBotPlugin::get_name() const { return NAME; }
str RawplugIrcBotPlugin::get_version() const { return VERSION; }

void RawplugIrcBotPlugin::exit()
{
	// 2013-03-01 15:26:18:   Raw Plugin Interface
	// 2013-03-01 15:26:18: Colsing down rawplug poll
	// 2013-03-01 15:26:18: Exiting from rawplug: rawplug-cpp
	// 2013-03-01 15:26:18: Error reading rawplug: rawplug-cpp
	// 2013-03-01 15:26:18: Exiting from rawplug: rawplug-sdcv
	// 2013-03-01 15:26:18: Error reading rawplug: rawplug-sdcv
	// 2013-03-01 15:26:18: Closing down rawplug responders
	// 2013-03-01 15:26:18: Closing  input from rawplug: rawplug-cpp
	// 2013-03-01 15:26:18: Closing  input from rawplug: rawplug-sdcv
	// 2013-03-01 15:26:18: Closing output from rawplug: rawplug-cpp
	// 2013-03-01 15:26:18: Closing output from rawplug: rawplug-sdcv

	done = true;

	log("Colsing down rawplug poll");
	if(poll_fut.valid())
		if(poll_fut.wait_for(std::chrono::seconds(10)) == std::future_status::ready)
			poll_fut.get();

	log("Closing down rawplug responders");
	std::vector<std::future_status> stati(futures.size());
	std::vector<std::future_status>::iterator i = stati.begin();
	for(std::future<void>& fut: futures)
		if(fut.valid())
			if((*(i++) = fut.wait_for(std::chrono::seconds(10))) == std::future_status::ready)
				fut.get();

	for(const std::future_status& fs: stati)
		if(fs == std::future_status::timeout)
			log("responder timaout...");

	// Try clean exit
	for(std::pair<const str, stdiostream_sptr>& p: stdos)
		if(p.second.get())
		{
			log("Exiting from rawplug: " << p.first);
			*p.second << "exit" << std::endl;
		}

	for(std::pair<const str, stdiostream_sptr>& p: stdis)
	{
		log("Closing  input from rawplug: " << p.first);
		p.second->close();
	}

	for(std::pair<const str, stdiostream_sptr>& p: stdos)
	{
		log("Closing output from rawplug: " << p.first);
		p.second->close();
	}
}

// INTERFACE: IrcBotMonitor

void RawplugIrcBotPlugin::event(const message& msg)
{
	// distribute to all monitoring plugins
	for(const str& id: raw_monitors)
		if(stdos[id])
			*stdos[id] << "event" << std::endl << msg.line << std::endl;

	str_vec middles;
	str params, trailing, sep;
	msg.get_params(middles, trailing);

	for(const str& middle: middles)
		{ params += sep + middle; sep = " "; }

	for(const str& id: monitors)
		if(stdos[id])
		{
			*stdos[id] << msg.prefix << std::endl;
			*stdos[id] << msg.command << std::endl;
			*stdos[id] << params << std::endl;
			*stdos[id] << msg.get_to() << std::endl;
			*stdos[id] << trailing << std::endl;
		}
}

}} // skivvy::rawplug {
