<?php
$runtype = 'live';
$debug = true;

date_default_timezone_set('UTC');
$stdi = fopen( 'php://stdin', 'r' );

$sk_msg = array();

if($debug)
{
	function sk_bug($text)
	{
		echo "/bug $text\n";
	}
	
	function sk_bug_msg()
	{
		global $sk_msg;
		sk_bug('// ------- sk_bug(): ---------------------');
		sk_bug('//   line: ' . $sk_msg['line']);
		sk_bug('//   from: ' . $sk_msg['from']);
		sk_bug('//    cmd: ' . $sk_msg['cmd']);
		sk_bug('// params: ' . $sk_msg['params']);
		sk_bug('//     to: ' . $sk_msg['to']);
		sk_bug('//   text: ' . $sk_msg['text']);
		sk_bug('// ---------------------------------------');
	}
}
else 
{
	function sk_bug($text){}
	function sk_bug_msg() {}
}

if($runtype == 'test')
{
	print_r($argv);
	print_r(join(' ', $argv));
	
	function sk_read_msg()
	{
		global $argv;
		global $sk_msg;
		$sk_msg['line'] = '';
		$sk_msg['from'] = 'nick!~user@host.com';
		$sk_msg['cmd'] = 'PRIVMSG';
		$sk_msg['params'] = '#channel';
		$sk_msg['to'] = '#channel';
		$sk_msg['text'] = join(' ', $argv);	
	}
	
}
else
{
	function sk_read_msg()
	{
		global $argv;
		global $sk_msg;
		global $stdi;
		$sk_msg['line'] = fgets($stdi);
		$sk_msg['from'] = fgets($stdi);
		$sk_msg['cmd'] = fgets($stdi);
		$sk_msg['params'] = fgets($stdi);
		$sk_msg['to'] = fgets($stdi);
		$sk_msg['text'] = fgets($stdi);
	}
}
function sk_say($who, $what)
{
	echo "/say $who $what\n";
}

function sk_reply($what)
{
	global $sk_msg;
	if(substr($sk_msg['to'], 0, 1) == '#')
	{
		sk_say($sk_msg['to'], $what);
	}
	else
	{
		sk_say(preg_split('/[!]/', $sk_msg['from'])[0], $what);
	}
}

function sk_log($msg) { echo "/log $msg\n"; }

function sk_msg_get_nick()
{
	global $sk_msg;
	$bits = preg_split('!', $sk_msg['from']);
	return $bits[0];
}
// TODO: Use this when chaging to new message format
#$chan_params = array
#(
#	array("PRIVMSG", "JOIN", "MODE", "KICK", "PART", "TOPIC")
#	, array("332", "333", "366", "404", "474", "482", "INVITE")
#	, array("353", "441")
#);
#
#$chan_start = "#&+!";
#
#function sk_msg_get_chan()
#{
#	$params = sk_msg_get_params();
#
#	for($i = 0; $i < count($chan_params); $i++)
#		if(in_array(command, $chan_params[i]) && count($params) > $i)
#			if(!empty($params[$i]) && in_array($params[$i][0], $chan_start))
#		return $params[$i];
#	return "";
#}

## Initialization functions

function sk_initialize() { echo "initialize\n"; }
function sk_id($id) { echo "id: $id\n"; }
function sk_name($name) { echo "name: $name\n"; }
function sk_version($version) { echo "version: $version\n"; }
function sk_add_command($cmd, $help)
{
	echo "add_command\n";
	echo "$cmd\n";
	echo "$help\n";
	echo "end_command\n";
}
function sk_add_raw_command($cmd, $help)
{
	echo "add_raw_command\n";
	echo "$cmd\n";
	echo "$help\n";
	echo "end_command\n";
}
function sk_add_monitor() { echo "add_monitor\n"; }
function sk_add_raw_monitor() { echo "add_raw_monitor\n"; }
function sk_poll_me($secs) { echo  "poll_me: $secs\n"; }
function sk_end_initialize() { echo "end_initialize\n"; }
?>
