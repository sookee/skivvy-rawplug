#!/usr/bin/php -q
<?php
include_once 'rawplug-api.php';

// http://tinyurl.com/api-create.php?url=http://google.com

function make_tiny_url($url)
{
	return file_get_contents('http://tinyurl.com/api-create.php?url=' . urlencode($url));
}

function make_tiny_url_x($url)
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

	sk_bug('rss_has()');
	sk_bug($qchan);
	sk_bug($qname);
	sk_bug($res);
	sk_bug($store_file);
	
	if(!($ifp = fopen($store_file, 'r')))
	{
		sk_log('Unable to open input file: ' . $store_file);
		return false;
	}
	
	if(!flock($ifp, LOCK_SH))
	{
		sk_log('Unable to lock input file: ' . $store_file);
		fclose($ifp);
		return false;
	}
	
	sk_bug('checking');
		
	$res = false;
	while(($line = fgets($ifp)))
	{
		sk_bug("\tline: $line");
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
	
	return true;
}

function rss_add($chan, $name, $url)
{
	global $store_file;
	
	sk_bug('rss_add()');
	sk_bug($chan);
	sk_bug($name);
	sk_bug($url);
	sk_bug($store_file);
	
	sk_log("rss: adding $name to $chan: $url");
	
	$res = false;
	if(!rss_has($chan, $name, $res))
		return false;
	
	sk_bug('checked');
	
	if($res)
	{
		sk_reply('RSS feed: ' . $name . ' already exists.');
		return false;
	}
	
	sk_bug('adding');

	if(!($ofp = fopen($store_file, 'a')))
	{
		sk_log('Unable to open output file: ' . $store_file);
		return false;
	}
	
	if(!flock($ofp, LOCK_EX))
	{
		sk_log('Unable to open obtain lock: ' . $store_file);
		fclose($ofp);
		return false;
	}
	
	// feed: #chan|name|true|123456789|http://...com
	$pubDate = new DateTime(date(DATE_RSS));
	fputs($ofp, 'feed: ' . $chan . '|' . $name . '|' . true . '|' . strval($pubDate->getTimestamp()) . '|' . $url . "\n");
	flock($ofp, LOCK_UN);
	fclose($ofp);
	
	sk_log('done');
	
	return true;
}

function rss_del($chan, $name)
{
	global $store_file;

	sk_log('rss_del()');

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

		if($temp[0] != $chan || $temp[1] != $name)
			$store_temp[] = $line;
	}

	ftruncate($fp, 0);
	foreach ($store_temp as $line)
		fputs($fp, $line . "\n");
	flock($fp, LOCK_UN);
	fclose($fp);

	return true;
}

function rss_list($chan)
{
	global $store_file;

	sk_bug('rss_list()');
	sk_bug($chan);
	sk_bug($store_file);
	
	if(!($fp = fopen($store_file, 'r')))
	{
		sk_log('Unable to open input file: ' . $store_file);
		return false;
	}

	if(!flock($fp, LOCK_SH))
	{
		sk_log('error: failed to lock ' . $store_file);
		fclose($fp);
		return false;
	}

	$feeds = array();
	while(($line = fgets($fp)))
	{
		$line = trim($line);
		if(strlen($line) == 0)
			continue;

		$temp = preg_split('/:\s+/', $line, 2);

		if($temp[0] != 'feed')
			continue;

		$temp = preg_split('/\|/', $temp[1]);

		if($temp[0] == $chan)
			$feeds[] = $temp[1];
	}
	
	if(count($feeds) > 0)
		sk_reply(join(", ", $feeds));
	
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

if(file_exists($store_file))
{
	$ifs = fopen($store_file, 'r');
	flock($store_file, LOCK_UN);
	fclose($ifs);
}
else
{
	$ofs = fopen($store_file, 'w');
	fclose($ofs);
}

sk_initialize();
sk_id('rawplug-rss');
sk_name('RSS Feed Updates.');
sk_version('0.01');
sk_add_command('!rss', '!rss add <name> <url> | del <name> | list');
sk_poll_me(5 * 60); // receive poll every 10 minutes
sk_end_initialize();

if(php_sapi_name() != 'cli')
	sleep(60);

while(($line = fgets($stdi)) != false)
{
	$line = trim($line);
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
			sk_bug_msg();
			sk_bug("\targs: " . count($args));
			$a = 0;
			foreach ($args as $arg)
			{sk_bug("\t\targ $a: $arg"); $a++;}
			if(count($args) == 3 && $args[2] == 'list')
				rss_list($sk_msg['to']);
			else if(count($args) == 4 && $args[2] == 'del')
				rss_del($sk_msg['to'], $args[3]);
			else if(count($args) == 5 && $args[2] == 'add')
				rss_add($sk_msg['to'], $args[3], $args[4]);
			else
				sk_say($sk_msg['to'], 'usage: !rss (list | sub <name> | add <name> <url>)');
			break;

		default:
			echo "/nop\n";
		break;
	}
}
?>
