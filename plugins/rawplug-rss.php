#!/usr/bin/php -q
<?php
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

$rss_chanfeeds = array();

function make_tiny_url($url)
{
	// http://tinyurl.com/create.php?source=indexpage&url=http%3A%2F%2Fwww.wikipedia.org%2F&submit=Make+TinyURL!&alias=
	// <b>http://tinyurl.com/53kja</b>
	$url = urlencode($url);
	$html = file_get_contents('http://tinyurl.com/create.php?source=indexpage&url=' . $url . '&submit=Make+TinyURL!&alias=');
	if(preg_match('/<b>(http:\/\/tinyurl.com\/[a-zA-Z0-9]+)<\/b>/', $html, $matches))
		return $matches[1];
	return false;
}

// chan.name.url: http://
// chan.name.pubDate: Mon, 15 Aug 2005 15:52:01 +0000
// chan.name.active: true

// File Format
// feed: #chan|name|true|123456789|http://...com

$store_file = $_SERVER['HOME'] . '/.skivvy/rawplug-rss-store-file.txt';
$store_lock = $_SERVER['HOME'] . '/.skivvy/rawplug-rss-store-lock.txt';
$store_temp = $_SERVER['HOME'] . '/.skivvy/rawplug-rss-store-temp.txt';

function store_lock()
{
	global $store_file;
	global $store_lock;

	// attempt to lock file
	$retries = 10;
	while(--$retries && !rename($store_file, $store_lock))
		sleep(1);

	if($retries)
		return true;

	sk_log('Unable to gain file lock on: ' . $store_file);
	return false;
}

function store_is_locked()
{
	if(!($ifp = fopen($store_lock, 'r')))
		return false;
	fclose($ifp);
	return true;
}

function store_unlock()
{
	global $store_file;
	global $store_lock;

	if(rename($store_lock, $store_file))
		return true;

	sk_log('Unable to unlock file: ' . $store_file);
	return false;
}

/**
 * Find out if a feed called $qname exists for channel $qchan
 * @param unknown $qchan channel to query
 * @param unknown $qname feed name to query
 * @param unknown &$res returns true if rss feed exists else false
 * @return boolean true on successs, false on error
 */
function rss_has($qchan, $qname, &$res)
{
	global $store_lock;
	global $store_temp;

	if(!store_lock())
		return false;

	if(!($ifp = fopen($store_lock, 'r')))
	{
		sk_log('Unable to open input file: ' . $store_lock);
		store_unlock();
		return false;
	}
	
	$res = false;
	while(($line = fgets($ifp)))
	{
		$line = trim($line);
		if(strlen($line) == 0)
			continue;

		//echo "line: $line\n";
		$temp = preg_split('/:\s+/', $line, 2);
	//	print_r($temp);
				
		if($temp[0] != 'feed')
		{
//			fputs($ofp, $line . "\n");
			continue;
		}

		$temp = preg_split('/\|/', $temp[1]);
		$chan = $temp[0];
		$name = $temp[1];
		
		if($chan == $qchan && $name == $qname)
		{
			$res = true;
			return store_unlock();
		}
	}
	fclose($ifp);
	
	return store_unlock();
}

function rss_add($chan, $name, $url)
{
	global $store_lock;
	
	$res = false;
	if(!rss_has($chan, $name, $res))
		return false;

	if($res)
	{
		sk_reply('RSS feed: ' . $name . ' already exists.');
		return false;
	}
	
	if(!store_lock())
		return false;
	
	if(!($ofp = fopen($store_lock, 'a')))
	{
		sk_log('Unable to open output file: ' . $store_lock);
		store_unlock();
		return false;
	}
	
	// feed: #chan|name|true|123456789|http://...com
	date_default_timezone_set('UTC');
	$pubDate = new DateTime(date(DATE_RSS));
	fputs($ofp, 'feed: ' . $chan . '|' . $name . '|' . true . '|' . strval($pubDate->getTimestamp()) . '|' . $url . "\n");
	fclose($ofp);
	return store_unlock();
}

function rss_check()
{
	global $store_lock;
	global $store_temp;
	
	sk_log('rss_check()');
	//echo "rss_check()\n";
	
	// attempt to lock file
	if(!store_lock())
		return false;

	if(!($ifp = fopen($store_lock, 'r')))
	{
		sk_log('Unable to open input file: ' . $store_lock);
		store_unlock();
		return false;
	}
	if(!($ofp = fopen($store_temp, 'w')))
	{
		sk_log('Unable to open output file: ' . $store_temp);
		store_unlock();
		return false;
	}
	
	while(($line = fgets($ifp)))
	{
		//echo "line: $line\n);
		$line = trim($line);
		if(!strlen($line))
			continue;

		$temp = preg_split('/:\s+/', $line, 2);
		//print_r($temp);
		
		if($temp[0] != 'feed')
			continue;

		$temp = preg_split('/\|/', $temp[1]);
		
		//print_r($temp);
		
		$chan = $temp[0];
		$name = $temp[1];
		$active = $temp[2];
		date_default_timezone_set('UTC');
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
				
				//print_r($sxmle);
				
				$pubDate = new DateTime($sxmle->pubDate);
// 				sk_log("\tpubDate: " . $pubDate->format('Y-m-d H:i:s'));
					
				if($pubDate > $lastPubDate)
				{
					sk_log("Make tiny: " . $sxmle->link);
					$tiny = make_tiny_url($sxmle->link);
					sk_log("tiny: " . $tiny);
					if($tiny)
					{
						$info = $sxmle->title . " " . $tiny;
						sk_say($chan, "[$name] $info");
						if($pubDate > $maxPubDate)
							$maxPubDate = $pubDate;
					}
				}
			}
		}
		// feed: #chan|name|true|123456789|http://...com
		$pubDate = new DateTime();
		fputs($ofp, 'feed: ' . $chan . '|' . $name . '|' . $active . '|' . strval($maxPubDate->getTimestamp()) . '|' . $url . "\n");
	}
	
	fclose($ifp);
	fclose($ofp);
	
	// update locked file
	if(rename($store_temp, $store_lock))
		return store_unlock();

	sk_log('Failed to update locked file: ' . $store_lock);
	return false;
}


//$tiny = make_tiny_url("http://google.com");
//print_r($tiny);

//exit(1);


sk_initialize();
sk_id('rawplug-rss');
sk_name('RSS Feed Updates.');
sk_version('0.01');
sk_add_command('!rss', '!rss add|del <name> <url>');
sk_poll_me(5 * 60); // receive poll every 10 minutes
sk_end_initialize();

if(store_is_locked())
{
	sk_log('Unlocking stare:');
	store_unlock();
}

// TESTING
//sk_read_msg();
//echo rss_add('#channel', 'name', 'http://live-clan.de/.xml/?type=rss');
//echo rss_check();


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
			sk_read_msg();
		break;

		default:
			echo "/nop\n";
		break;
	}
}
?>
