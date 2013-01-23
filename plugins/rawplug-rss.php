<?php
require_once "XML/RSS.php";

$stdi = fopen( 'php://stdin', 'r' );

function sk_initialize() { echo 'initialize'; }

$sk_msg = array();

function sk_read_msg()
{
	global $sk_msg;
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

function sk_initialize() { echo "initialize"; }
function sk_id($id) { echo "id: $id"; }
function sk_name($name) { echo "name: $name"; }
function sk_version($version) { echo "version: $version"; }
function sk_add_command($cmd $help)
{
	echo "add_command";
	echo "$cmd";
	echo "$help";
	echo "end_command";
}
function sk_add_raw_command($cmd, $help)
{
	echo "add_raw_command";
	echo "$cmd";
	echo "$help";
	echo "end_command";
}
function sk_add_monitor() { echo "add_monitor"; }
function sk_add_raw_monitor() { echo "add_raw_monitor"; }
function sk_poll_me($secs) { echo  "poll_me $secs"; }
function sk_end_initialize() { echo "end_initialize"; }

$rss_chanfeeds = array();

function make_tiny_url($url)
{
	// http://tinyurl.com/create.php?source=indexpage&url=http%3A%2F%2Fwww.wikipedia.org%2F&submit=Make+TinyURL!&alias=
	// <b>http://tinyurl.com/53kja</b>
	$url = urlencode($url);
	$html = file_get_contents('http://tinyurl.com/create.php?source=indexpage&url=' . $url . '&submit=Make+TinyURL!&alias=');
	if(preg_match(/<b>http:\/\/tinyurl.com/[[:alphanum:]]+<\/b>/, $html, $matches))
		return $matches[0];
	return false;
}

function rss_add($chan, $name, $url)
{
	global $rss_chanfeeds;
	if(($url = make_tiny_url($url)))
	{
		$rss_chanfeeds[$chan][$name] = array();
		$rss_chanfeeds[$chan][$name]['url'] = $url;
		$rss_chanfeeds[$chan][$name]['pubDate'] = new DateTime(date(DATE_RSS));
		$rss_chanfeeds[$chan][$name]['active'] = true;
		return true;
	}
	return false;
}
			
function rss_check()
{
	global $rss_chanfeeds;
	foreach($rss_chanfeeds as $chan => $rss_feeds)
	{
		foreach($rss_feeds as $name => $rss_feed)
		{
			$xml = file_get_contents('rss.xml');
			$dom = new DomDocument();
			$dom->loadXML($xml);
			foreach($dom->getElementsByTagName('item') as $item)
			{
				$sxmle = simplexml_import_dom($item);
				$pubDate = new DateTime($sxmle->pubDate);
					
				if($pubDate > $rss_feed['pubDate'])
				{
					if($rss_feed['active'])
					{
						$info = $sxmle->description . " " . $sxmle->link;
						sk_say($chan, "[$name] $info");
					}
					$rss_feed['pubDate'] = $pubDate;
				}
			}
		}
	}
}		

sk_initialize();
sk_id("rawplug-rss");
sk_name("RSS Feed Updates.");
sk_version("0.01");
sk_add_command("!rss" "!rss add|del <name> <url>");
sk_poll_me(10); // receive poll every 10 minutes
sk_end_initialize();

while (($line = fgets($stdi)) != false)
{
	switch($line)
	{
		case 'exit':
			exit(0);
		break;
		case 'poll':
			rss_check();
		break;
		
		default:
			echo "/nop";
		break;
	}
}

?>