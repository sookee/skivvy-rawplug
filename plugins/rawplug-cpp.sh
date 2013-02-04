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

source rawplug-api.sh

LOG_FILE="rawplug.log"

log()
{
	echo "$(date +%Y%m%d-%H%M%S): $1" >> $LOG_FILE
}

trim()
{
	# Determine if 'extglob' is currently on.
	local extglobWasOff=1
	shopt extglob >/dev/null && extglobWasOff=0 
	(( extglobWasOff )) && shopt -s extglob # Turn 'extglob' on, if currently turned off.
	# Trim leading and trailing whitespace
	local var=$1
	var=${var##+([[:space:]])}
	var=${var%%+([[:space:]])}
	(( extglobWasOff )) && shopt -u extglob # If 'extglob' was off before, turn it back off.
	echo -n "$var"  # Output trimmed string.
}

sk_initialize
sk_id 'rawplug-cpp'
sk_name 'C++ Code Runner'
sk_version '0.01'
sk_add_command '!cpp' 'Run c++ code.'
#sk_poll_me 60
sk_end_initialize

while read line
do
	case $line in
		
		'exit')
			exit 0
		;;
		'!cpp')
			sk_read_msg
			# msg.text = "!cpp { c++ code; }"
			code=${sk_msg[text]#\!cpp*}
						
			#sk_reply 'Building program...'
			cat rawplug-cpp-headers.cpp > program.cpp
			echo $code >> program.cpp
			
			#sk_reply 'Compiling program...'
			g++ -std=c++11 -o program program.cpp > program-errors.txt 2>&1
						
			#sk_reply 'Running program...'
			timeout 5s ./program > program.output.txt 2>&1
						
			sk_reply $(cat program.output.txt);
		;;
		*)
			# errors
			log "error: unknown command: $line"	
			echo "/nop"
		;;
	esac
done
