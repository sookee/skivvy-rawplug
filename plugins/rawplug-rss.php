#!/usr/bin/php -q
<?php
date_default_timezone_set('UTC');
$stdi = fopen( 'php://stdin', 'r' );

$sk_msg = array();

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

function sk_read_msg()
{
	global $sk_msg;
	global $stdi;
	$sk_msg['line'] = fgets($stdi);
	$sk_msg['from'] = fgets($stdi);
	$sk_msg['cmd'] = fgets($stdi);
	$sk_msg['params'] = fgets($stdi);
	$sk_msg['to'] = fgets($stdi);
	$sk_msg['text'] = fgets($stdi);
}

function sk_msg_get_nick()
{
	global $sk_msg;
	$bits = preg_split('!', $sk_msg['from']);
	return $bits[0];
}

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

function make_tiny_url($url)
{
	$url = urlencode($url);
	$html = file_get_contents('http://tinyurl.com/create.php?source=indexpage&url=' . $url . '&submit=Make+TinyURL!&alias=');
	if(preg_match('/<b>(http:\/\/tinyurl.com\/[a-zA-Z0-9]+)<\/b>/', $html, $matches))
		return $matches[1];
	return false;
}

// File Format
// feed: #chan|name|true|123456789|http://...com

$store_file = $_SERVER['HOME'] . '/.skivvy/rawplug-rss-store-file.txt';

/**
 * Find out if a feed called $qname exists for channel $qchan
 * @param unknown $qchan channel to query
 * @param unknown $qname feed name to query
 * @param unknown &$res returns true if rss feed exists else false
 * @return boolean true on successs, false on error
 */
function rss_has($qchan, $qname, &$res)
{
	global $store_file;

	if(!($ifp = fopen($store_file, 'r')))
	{
		sk_log('Unable to open input file: ' . $store_file);
		return false;
	}
	
	if(!floc($ifp, LOCK_SH))
	{
		sk_log('Unable to lock input file: ' . $store_file);
		fclose($ifp);
		return false;
	}
		
	$res = false;
	while(($line = fgets($ifp)))
	{
		$line = trim($line);
		if(strlen($line) == 0)
			continue;

		$temp = preg_split('/:\s+/', $line, 2);
				
		if($temp[0] != 'feed')
			continue;

		$temp = preg_split('/\|/', $temp[1]);
		
		if($qchan == $temp[0] && $qname == $temp[1])
		{
			$res = true;
			break;
		}
	}
	flock($ifp, LOCK_UN);
	fclose($ifp);
	
	return $res;
}

function rss_add($chan, $name, $url)
{
	global $store_file;
	
	$res = false;
	if(!rss_has($chan, $name, $res))
		return false;

	if($res)
	{
		sk_reply('RSS feed: ' . $name . ' already exists.');
		return false;
	}
	
	if(!($ofp = fopen($store_file, 'a')))
	{
		sk_log('Unable to open output file: ' . $store_file);
		return false;
	}
	
	if(!flock($afp, LOCK_EX))
	{
		sk_log('Unable to open obtail lock: ' . $store_file);
		fclose($ofp);
		return false;
	}
	
	// feed: #chan|name|true|123456789|http://...com
	$pubDate = new DateTime(date(DATE_RSS));
	fputs($ofp, 'feed: ' . $chan . '|' . $name . '|' . true . '|' . strval($pubDate->getTimestamp()) . '|' . $url . "\n");
	flock($ofp, LOCK_UN);
	fclose($ofp);
	
	return true;
}

function rss_check()
{
	global $store_file;
	
	sk_log('rss_check()');

	if(!($fp = fopen($store_file, 'c+')))
	{
		sk_log('Unable to open input file: ' . $store_file);
		return false;
	}
	
	if(!flock($fp, LOCK_EX))
	{
		sk_log('error: failed to lock ' . $store_file);
		fclose($fp);
		return false;
	}

	$store_temp = array();
	
	while(($line = fgets($fp)))
	{
		$line = trim($line);
		if(strlen($line) == 0)
			continue;

		$temp = preg_split('/:\s+/', $line, 2);
		
		if($temp[0] != 'feed')
			continue;

		$temp = preg_split('/\|/', $temp[1]);
		
		$chan = $temp[0];
		$name = $temp[1];
		$active = $temp[2];

		$lastPubDate = new DateTime();
		$lastPubDate->setTimestamp($temp[3]);
		$maxPubDate = $lastPubDate;
		
		$url = $temp[4];
		
		sk_log("Feed: $name last checked: " . $lastPubDate->format('Y-m-d H:i:s'));

		if($active)
		{
			$xml = file_get_contents($url);
			$dom = new DomDocument();
			$dom->loadXML($xml);
			
			foreach($dom->getElementsByTagName('item') as $item)
			{
				$sxmle = simplexml_import_dom($item);				
				$pubDate = new DateTime($sxmle->pubDate);
					
				if($pubDate <= $lastPubDate)
					continue;

				$tiny = make_tiny_url($sxmle->link);

				if(!$tiny)
					continue;

				sk_say($chan, "[$name] " . $sxmle->title . " " . $tiny);
				if($pubDate > $maxPubDate)
					$maxPubDate = $pubDate;
			}
		}
		// feed: #chan|name|true|123456789|http://...com
 		$pubDate = new DateTime();
		sk_log('    pubDate: ' . $pubDate->format('Y-m-d H:i:s'));
 		sk_log(' maxPubDate: ' . $maxPubDate->format('Y-m-d H:i:s'));
		sk_log('lastPubDate: ' . $lastPubDate->format('Y-m-d H:i:s'));
		$store_temp[] = 'feed: ' . $chan . '|' . $name . '|' . $active . '|' . strval($maxPubDate->getTimestamp()) . '|' . $url;
	}
	
	ftruncate($fp, 0);
	foreach ($store_temp as $line)
		fputs($fp, $line . "\n");
	flock($fp, LOCK_UN);
	fclose($fp);

	return true;
}

if(!file_exists($store_file))
{
	$ofs = fopen($store_file, 'w');
	fclose($ofs);
}

sk_initialize();
sk_id('rawplug-rss');
sk_name('RSS Feed Updates.');
sk_version('0.01');
sk_add_command('!rss', '!rss add|del <name> <url>');
sk_poll_me(5 * 60); // receive poll every 10 minutes
sk_end_initialize();

sleep(60);

while(($line = fgets($stdi)) != false)
{
	$line = trim($line);
	//echo "line: $line";
	switch($line)
	{
		case 'exit':
			exit(0);
		break;
		case 'poll':
			rss_check();
		break;
		
		case '!rss':
			//!rss add <name> <url>
			//!rss del <name>
			//!rss list
			sk_read_msg();
			
			$args = preg_split('/\s+/', $sk_msg['text']);
			if(count($args) == 2 && $args[1] == 'list')
				do_list($chan);
			else if(count($args) == 3 && $args[1] == 'del')
				so_del($chan, $args[2]);
			else if(count($args) == 4 && $args[1] == 'add')
				so_add($chan, $args[2], $args[3]);
			else
				sk_reply("usage: !rss (list | sub <name> | add <name> <url>)");
			break;

		default:
			echo "/nop\n";
		break;
	}
}
?>
