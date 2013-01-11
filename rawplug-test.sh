#!/bin/bash
# /*-----------------------------------------------------------------.
# | Copyright (C) 2012 SooKee oaskivvy@gmail.com                     |
# '------------------------------------------------------------------'
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
# 
# http://www.gnu.org/licenses/gpl-2.0.html
# 
# '-----------------------------------------------------------------*/


	virtual bool initialize();

	// INTERFACE: IrcBotPlugin

	virtual std::string get_name() const;
	virtual std::string get_version() const;
	virtual void exit();

	// INTERFACE: IrcBotMonitor

	virtual void event(const message& msg);


while read line
do

	case line in
	
		initialize)
			echo "add"
			echo "!cmd1"
			echo "Ecample command 1"
			echo "Does example 1 type things"
			echo "end"
			echo "add"
			echo "!cmd2"
			echo "Ecample command 2"
			echo "Does example 2 type things"
			echo "end"
			echo "add_monitor"
		;;
		get_name)
			echo "rawplug-test"
		;;
		get_version)
			echo "0.00"
		;;
		exit)
			exit 0
		;;
		!cmd1)
			echo 
		;;
		# If add_moniter was done unknown lines are raw monitor data else errors
		*)
			echo "monitor: $line"	
		;;
	esac

	echo $line

done
