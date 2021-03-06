#!/bin/bash
# /*-----------------------------------------------------------------.
# | Copyright (C) 2013 SooKee oaskivvy@gmail.com                     |
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


declare -A sk_msg

sk_read_msg()
{
	read sk_msg[line]
	read sk_msg[from]
	read sk_msg[cmd]
	read sk_msg[params]
	read sk_msg[to]
	read sk_msg[text]	
}

LOG_FILE="$HOME/tmp/rawplug.log"
CPPFLUSH="$HOME/bin/cppflush"

log()
{
	echo "$(date +%Y%m%d-%H%M%S): $1" | $CPPFLUSH >> $LOG_FILE
}

while read line
do
	log $line
	
	case $line in
	
		'initialize')
			echo "add_command" # Receive parsed IRC message
			echo "!cmd1"
			echo "Ecample command 1"
			echo "Does example 1 type things"
			echo "end_command"
			echo "add_raw_command" # Receive raw IRC protocol
			echo "!cmd2"
			echo "Ecample command 2"
			echo "Does example 2 type things"
			echo "end_command"
			#echo "add_monitor" # Only if you want to receive all channel messages
			#echo "add_raw_monitor" # Only if you want to receive all channel messages
			echo "end_initialize"
		;;
		'get_id')
			echo "rawplug-test-00"
		;;
		'get_name')
			echo "A Rawplug Test Script 00"
		;;
		'get_version')
			echo "0.00"
		;;
		'exit')
			exit 0
		;;
		'!cmd1')
			sk_read_msg
			# mgs examples
			# msg_line   :SooKee!~SooKee@SooKee.users.quakenet.org PRIVMSG #skivvy :!oafind all
			# msg_from   SooKee!~SooKee@SooKee.users.quakenet.org
			# msg_cmd    PRIVMSG
			# msg_params #skivvy
			# msg_to     #skivvy
			# msg_text   !oafind all
		
			echo "/say ${sk_msg[to]} rawplug is working!!"
		;;
		'!cmd2')
			read msg_line
			# $msg_line examples
			# :Skivvy!~Skivvy@Skivvy.users.quakenet.org JOIN #openarenahelp
			# :Zimmermint!~Zimmy@5ac0ceb1.bb.sky.com PRIVMSG #skivvy-admin :Hello all
			# :Q!TheQBot@CServe.quakenet.org MODE #omfg +v satyamash
			# :Skivvy!~Skivvy@Skivvy.users.quakenet.org QUIT :Signed off
			echo "/say #skivvy rawplug is working!!"
		;;
		'event')
			# If add_moniter was done unknown lines are raw monitor data else errors
			read msg
			#echo "monitor: $msg"	
		;;
		*)
			# errors
			log "error: unknown command: $line"	
		;;
	esac
done
