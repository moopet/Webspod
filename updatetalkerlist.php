<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<meta name="keywords" content="spod,talker,telnet,web,html,interface,cgi,firewall,terminal,free,software,service,browser,ew-too,nuts,amnuts,pg+,pg96,summink,sensei-summink" />
<meta name="description" content="Webspod is an HTML interface for connecting to talkers. You run it on a webserver and then browse to it to spod. It is free software, released under the GNU General Public License. It allows connection to telnet-only services through many firewalls" />
<meta http-equiv="Content-Language" content="EN" />
<meta name="author" content="Ben Sinclair (moopet)" />
<meta name="distribution" content="Global" />
<meta name="copyright" content="Copyright © 2003 Ben Sinclair" />
<meta name="robots" content="FOLLOW,INDEX" />
<link rel="stylesheet" href="system.css" />
<title>talker list grabber script</title>
</head>
<body>
<?php
// updatetalkerlist.php

include "config.php";

$list = @file("http://list.ewtoo.org/downloadlist.cgi/talkerlist.txt?type=talker&style=ewtoo&oldest=10&sortby=lowername&filetype=text&method=download&downtimes=n");

if (!$list)
{
?>
<font class="menu">error</font>
<hr/><br/>
The current talker list file cannot be downloaded from <a href="http://list.ewtoo.org">list.ewtoo.org</a>.
<br/>
If you wish to try again, please hit refresh or click <a href="<?php echo $PHP_SELF ?>">here</a>.
<br/><br/>
Sorry for any inconvenience.
</body>
</html>
<?php
	exit;
}

?>
<font class="menu">downloading talker list, please wait...</font>
<hr/><br/>
<?php

$fp = fopen($TALKER_FILE, "w");
for ($i = 0; $list[$i]; $i++)
{
	echo "&nbsp;$list[$i]<br/>";
	fwrite($fp, $list[$i]);
}
fclose($fp);
?>
<hr/><br/>
Download complete. Please click <a href="index.php">here</a> to return to the main page.
</body>
</html>