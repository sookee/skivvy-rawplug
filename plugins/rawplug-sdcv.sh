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

sk_say()
{
	echo "/say ${sk_msg[to]} $*."
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
			echo "!brit"
			echo "Britanica Concise Info [abbreviated]"
			echo "end_command"
			echo "add_command" # Receive parsed IRC message
			echo "!php"
			echo "PHP function reference [abbreviated]"
			echo "end_command"
			echo "end_initialize"
		;;
		'get_id')
			echo "rawplug-sdcv"
		;;
		'get_name')
			echo "sdcv interface."
		;;
		'get_version')
			echo "0.01"
		;;
		'exit')
			exit 0
		;;
		'!brit')
			sk_read_msg

			word=$(echo "${sk_msg[text]}"|cut -d " " -f 2)
			entry=$(sdcv -u 'The Britannica Concise' -n "$word")
			text=$(echo $entry|cut -d "-" -f 5-)
			text=$(echo $text|cut -d "." -f 1)
			
			if [[ ${text:1:${#word}} == $word ]]; then
				echo "/say ${sk_msg[to]} $text."
			else
				echo "/say ${sk_msg[to]} word: \"$word\" not found."
			fi
		;;
		'!php')
			sk_read_msg

			word=$(echo "${sk_msg[text]}"|cut -d " " -f 2)
			entry=$(sdcv -u 'php' -n "$word")
			text=$(echo $entry|cut -d "-" -f 5-)
			text=$(echo $text|cut -d "." -f 1)
			
			if [[ ${text:1:${#word}} == $word ]]; then
				echo "/say ${sk_msg[to]} $text."
			else
				echo "/say ${sk_msg[to]} word: \"$word\" not found."
			fi
		;;
		*)
			# errors
			log "error: unknown command: $line"	
		;;
	esac
done
