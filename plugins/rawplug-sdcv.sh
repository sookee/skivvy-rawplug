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

## Message processing

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
sk_id "rawplug-sdcv"
sk_name "sdcv interface."
sk_version "0.01"
sk_add_command "!ox" "Oxford Adv. Learners Dict. [abbreviated]"
sk_add_command "!brit" "Britanica Concise Info [abbreviated]"
sk_add_command "!php" "PHP function reference [abbreviated]"
sk_add_command "!calc" "Calculator"
sk_add_command "!raw" "Rawplug Test Function"
#sk_poll_me 60
sk_end_initialize

while read line
do
	case $line in
		
		'poll')
			sk_say '#skivvy' 'poll'
		;;
		'exit')
			exit 0
		;;
		'!ox')
			sk_read_msg
			# !ox   tart  [2] \!ox\s+\w+(?:\s+\d+)?
			# !ox "tart bart" [2] \!ox\s+"[^"]*?"(\s+\d+)?
			
			if [[ $(echo "${sk_msg[text]}"|grep -P -o '\!ox\s+\w+(\s+\d+)?') ]]; then
				word=$(echo $(trim $(echo "${sk_msg[text]}"|cut -d ' ' -f 2-))|cut -d ' ' -f 1)
				num=$(echo "${sk_msg[text]}"|grep -P -o '\d+')
				echo word: $word
				echo num : $num
			elif [[ $(echo "${sk_msg[text]}"|grep -P -o '\!ox\s+"[^"]*?"(\s+\d+)?') ]]; then
				word=$(echo "${sk_msg[text]}"|grep -P -o '"[^"]*"'|grep -P -o '[^"]+')
				num=$(echo "${sk_msg[text]}"|grep -P -o '\d+')
				echo word: $word
				echo num : $num
			else
				echo bad
			fi
			
			if [[ -z $num ]]; then num=1; fi
			((num2 = num + 1))
			#echo  num: $num
			#echo num2: $num2
			text=$(sdcv -u "Oxford Advanced Learner's Dictionary" -n "$word")
			#echo text: $text
			text=${text//\*/\\*}
			#echo text: $text
			text=$(echo $text|cut -d "-" -f 5-)
			#echo text: $text
			text=$(echo $text|cut -d "$num" -f 2-|cut -d "$num2" -f 1)
			#echo text: $text
			sk_reply $num $text
		;;
		'!brit')
			sk_read_msg

			word=$(echo "${sk_msg[text]}"|cut -d " " -f 2)
			text=$(sdcv -u 'The Britannica Concise' -n "$word")
			text=$(echo $text|cut -d "-" -f 5-|cut -d "." -f 1)
			
			if [[ ${text:1:${#word}} == $word ]]; then
				sk_reply "$text."
			else
				sk_reply "word: \"$word\" not found."
			fi
		;;
		'!php')
			sk_read_msg

			word=$(echo "${sk_msg[text]}"|cut -d " " -f 2)
			text=$(sdcv -u 'php' -n "$word")
			text=$(echo $text|cut -d "-" -f 5-|cut -d "." -f 1)
			
			if [[ ${text:1:${#word}} == $word ]]; then
				sk_reply "$text."
			else
				sk_reply "function: \"$word\" not found."
			fi
		;;
		'!calc')
			sk_read_msg
			sk_reply "=" $(echo "${sk_msg[text]}"|cut -d " " -f 2-|bc)
		;;
		'!raw')
			sk_read_msg
			sk_reply "Do I reply to the right place?"
		;;
		*)
			# errors
			log "error: unknown command: $line"	
			echo "/nop"
		;;
	esac
done
