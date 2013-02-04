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

sk_msg_get_nick()
{
	echo ${sk_msg[from]%%\!*}
}

## Initialization functions

sk_error() { echo "error: $1"; exit 1; }

sk_initialize() { echo "initialize"; }
sk_initialize_2()
{
	if [[ $# -eq 3 ]]; then
		sk_initialize
		sk_id "$1"
		sk_name "$2"
		sk_version "$3"
	else
		sk_error "sk_initialize_2 needs 3 parameters, $# supplied."
	fi
}

sk_id() { echo "id: $1"; }
sk_name() { echo "name: $1"; }
sk_version() { echo "version: $1"; }
sk_add_command()
{
	if [[ $# -eq 2 ]]; then
		echo "add_command"
		echo "$1"
		echo "$2"
		echo "end_command"	
	else
		sk_error "sk_add_command needs 2 parameters, $# supplied."
	fi
}
sk_add_raw_command()
{
	if [[ $# -eq 2 ]]; then
		echo "add_raw_command"
		echo "$1"
		echo "$2"
		echo "end_command"	
	fi
}
sk_add_monitor() { echo "add_monitor"; }
sk_add_raw_monitor() { echo "add_raw_monitor"; }
sk_poll_me() { echo "poll_me: $1"; } # request poll every $1 seconds
sk_end_initialize() { echo "end_initialize"; }

## Utility functions

sk_say()
{
	who=$1
	shift
	echo "/say $who $*"
}

sk_reply()
{
	if [[ ${sk_msg[to]:0:1} == '#' ]]; then
		sk_say ${sk_msg[to]} "$*"
	else
		sk_say $(sk_msg_get_nick) "$*"
	fi
}
