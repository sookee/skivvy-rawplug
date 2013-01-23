<?php
require_once "XML/RSS.php";

$stdi = fopen( 'php://stdin', 'r' );

function sk_initialize() { echo 'initialize'; }

$sk_msg = array();

function sk_read_msg()
{
	$sk_msg['line'] = fgets($stdi);
	$sk_msg['from'] = fgets($stdi);
	$sk_msg['cmd'] = fgets($stdi);
	$sk_msg['params'] = fgets($stdi);
	$sk_msg['to'] = fgets($stdi);
	$sk_msg['text'] = fgets($stdi);
}

function sk_msg_get_nick()
{
	$bits = preg_split('!', $sk_msg['from']);
	return $bits[0];
}

## Initialization functions

function sk_initialize() { echo "initialize"; }
sk_id($id) { echo "id: $id"; }
sk_name($name) { echo "name: $name"; }
sk_version() { echo "version: $1"; }
sk_add_command($cmd $help)
{
	echo "add_command";
	echo "$cmd";
	echo "$help";
	echo "end_command";
}
sk_add_raw_command($cmd, $help)
{
	echo "add_raw_command";
	echo "$cmd";
	echo "$help";
	echo "end_command";
}
sk_add_monitor() { echo "add_monitor"; }
sk_add_raw_monitor() { echo "add_raw_monitor"; }
sk_end_initialize() { echo "end_initialize"; }

$rss_feeds = array();

function make_tiny_url($url)
{
	// http://tinyurl.com/create.php?source=indexpage&url=http%3A%2F%2Fwww.wikipedia.org%2F&submit=Make+TinyURL!&alias=
	// <b>http://tinyurl.com/53kja</b>
	$url = urlencode($url);
	$html = file_get_contents('http://tinyurl.com/create.php?source=indexpage&url=' . $url . '&submit=Make+TinyURL!&alias=');
	if(preg_match(/<b>http:\/\/tinyurl.com/[[:alphanum:]]+<\/b>/, $html, $matches))
	{
		return $matches[0];
	}
	return false;
}

function rss_add($name, $url)
{
	if(($url = make_tiny_url($url)))
	{
		$rss_feeds[$name] = array();
		$rss_feeds[$name]['url'] = $url;
		$rss_feeds[$name]['pubDate'] = new DateTime(date(DATE_RSS));//DATE_RSS);
		$rss_feeds[$name]['channel'] = '#openarenahelp';
		return true;
	}
	return false;
}
			
function rss_check($name)
{
	$xml = file_get_contents('rss.xml');
	$dom = new DomDocument();
	$dom->loadXML($xml);
	foreach($dom->getElementsByTagName('item') as $item)
	{
		$sxmle = simplexml_import_dom($item);
		$pubDate = new DateTime($sxmle->pubDate);
			
		if($pubDate > $rss_feeds[$name]['pubDate'])
		{
			$info = $sxmle->description . " " . $sxmle->link;
			sk_say($rss_feeds[$name]['channel'], $info);
			$rss_feeds[$name]['pubDate'] = $pubDate;
		}
	}
}		

sk_initialize();
sk_id("rawplug-sdcv");
sk_name("sdcv interface.");
sk_version("0.01");
sk_add_command("!rss" "!rss add|del <name> <url>");
sk_end_initialize();

while (($line = fgets($stdi)) != false)
{
	switch($line)
	{
		case 'exit':
			exit(0);
		break;
		
		default:
			echo "/nop";
		break;
	}
}

?>