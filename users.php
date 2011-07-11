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
<meta http-equiv="refresh" content="60;url=users.php" />
<title>webspod|users</title>
</head>
<body>
<a class="menu" href="index.php">connect</a>&nbsp;
<font class="menu">users</font>&nbsp;
<a class="menu" href="help.html">help</a>&nbsp;
<table align="center">
<tr>
	<th>&nbsp;#&nbsp;</th>
	<th>&nbsp;username&nbsp;</th>
	<th>&nbsp;hostname&nbsp;</th>
	<th>&nbsp;port&nbsp;</th>
	<th>&nbsp;ident&nbsp;</th>
	<th>&nbsp;connection time&nbsp;</th>
</tr>
<?php
// users.php

include "config.php";

// botch so far

$s = "webspod~";
$path = "/tmp/";

if (!($dp = @opendir($path)))
	dienice("Cannot open folder '$path'");

$i = 0;

while ($file = readdir($dp))
	if (eregi("$s([0-9]+)-lock", $file, $regs))
	{
		if (!@posix_kill($regs[1], SIGUSR2))
		{
			@unlink("$path$file");
			continue;
		}
		$i++;
		echo "<tr>\n<td>&nbsp;$i&nbsp;</td>\n";
		$data = file("$path$file");
		$time = time() - filemtime("$path$file");
		$username = explode(": ", $data[0]);
		$username = trim($username[1]);
		$hostname = explode(": ", $data[1]);
		$hostname = trim($hostname[1]);
		$port = explode(": ", $data[2]);
		$port = trim($port[1]);
		$ident = explode(": ", $data[3]);
		$ident = trim($ident[1]);
		echo "\t<td onclick=\"top.location.href='http://spodlist.org/$username'\" onmouseover=\"this.className='hover'\" onmouseout=\"this.className=''\">&nbsp;$username&nbsp;</td>\n";
		echo "\t<td onclick=\"top.location.href='index.php?hostname=$hostname&amp;port=$port'\" onmouseover=\"this.className='hover'\" onmouseout=\"this.className=''\">&nbsp;$hostname&nbsp;</td>\n";
		echo "\t<td onclick=\"top.location.href='index.php?hostname=$hostname&amp;port=$port'\" onmouseover=\"this.className='hover'\" onmouseout=\"this.className=''\">&nbsp;$port&nbsp;</td>\n";
		echo "\t<td>&nbsp;$ident&nbsp;</td>\n";
		echo "\t<td>&nbsp;";
		printf("%2d:%02d", $time / 60, $time % 60);
		echo "&nbsp;</td></tr>\n";
	}
if ($i == 1)
	echo "<tr>\n\t<th colspan=\"6\">1 user connected</th>\n</tr>";
else
	echo "<tr>\n\t<th colspan=\"6\">$i users connected</th>\n</tr>";
closedir($dp);
?>
</table>
</body>
</html>
