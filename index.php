<?php
// index.php

include "config.php";

if ($HARD_CODED_TALKER)
{
	header("location:hc_index.php");
	exit;
}

if (!isset($hostname))
	$freshpage = 1;

if (!isset($hostname))
	$hostname	= $DEFAULT_HOSTNAME;
if (!isset($port))
	$port		= $DEFAULT_PORT;
if (!isset($theme))
	$theme	= $DEFAULT_STYLE;

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<noscript>
<meta http-equiv="refresh" content="0;url=nojs.html" />
</noscript>
<meta name="keywords" content="spod,talker,telnet,web,html,interface,cgi,firewall,terminal,free,software,service,browser,ew-too,nuts,amnuts,pg+,pg96,summink,sensei-summink" />
<meta name="description" content="Webspod is an HTML interface for connecting to talkers. You run it on a webserver and then browse to it to spod. It is free software, released under the GNU General Public License. It allows connection to telnet-only services through many firewalls" />
<meta http-equiv="Content-Language" content="EN" />
<meta name="author" content="Ben Sinclair (moopet)" />
<meta name="distribution" content="Global" />
<meta name="copyright" content="Copyright © 2003 Ben Sinclair" />
<meta name="robots" content="FOLLOW,INDEX" />
<title>webspod|connect</title>
<link rel="stylesheet" href="system.css" />
<?php
$talkers = @file($TALKER_FILE);
if (!$talkers)
{
?>
<meta http-equiv="refresh" content="5;url=updatetalkerlist.php" />
</head>
<body>
<font class="menu">warning</font>
<hr/><br/>
The talker details file doesn't exist. It will need to be (re)generated. This should happen automatically
in about 10 seconds, but could take up to about a minute to complete.
<br/>
This message should only appear once,
and is only here because the system has been recently (re)started.
<br/><br/>
Please <font color="red"><u>do not</u></font> hit refresh until this operation has completed.
<br/><br/>
Sorry for any inconvenience.
</body>
</html>
<?php
	exit;
}
?>
<script type="text/javascript">
<!--
function changeTalker(what)
{
	var opts = document.forms[0]

<?php
	for ($i = 0; $talkers[$i]; $i++)
		if (eregi("(.+), running at (.+) (.+)", $talkers[$i], $regs))
			echo "\tif (opts.talker.selectedIndex == '" . ($i + 1) . "')\n\t{\n\t\topts.hostname.value='$regs[2]'\n\t\topts.port.value='" . trim($regs[3]) . "'\n\t}\n";
?>
}
// -->
</script>
</head>
<body<?php if (isset($freshpage)) echo " onLoad=\"changeTalker()\"" ?>>
<font class="menu">connect</font>&nbsp;
<a class="menu" href="users.php">users</a>&nbsp;
<a class="menu" href="help.html">help</a>&nbsp;
<hr/><br/>
<center>
<table border="0" cellspacing="0" cellpadding="0" style="border: 1px solid white">
<form name="loginform" method="post" action="frameset.php">
<tr>
<td class="formtag">talker:&nbsp;</td>
<td><select class="select" name="talker" onchange="changeTalker(this)">
	<option class="option" value="" selected>&lt;select a talker&gt;</option>
<?php
	for ($i = 0; $talkers[$i]; $i++)
		if (eregi("(.+), running at (.+) (.+)", $talkers[$i], $regs))
			echo "\t<option class=\"option\" value=\"$i\">$regs[1]</option>\n";
?>
</select>
</td>
</tr><tr>
<td class="formtag">hostname:&nbsp;</td>
<td><input name="hostname" value="<?php echo $hostname ?>" size="30" maxlength="127" /></td>
</tr><tr>
<td class="formtag">port:&nbsp;</td>
<td><input name="port" value="<?php echo $port ?>" size="5" /></td>
</tr><tr>
<td class="formtag">theme:&nbsp;</td>
<td><select class="select" name="theme">
<?php
if (!($dp = @opendir($THEME_PATH)))
	die("Cannot read from themes folder");
while ($themes[] = readdir($dp));
sort($themes);
for ($i = 0; $i < count($themes); $i++)
{
	$file = $themes[$i];
	if ($file[0] == '.' || empty($file))
		continue;
	if ($file == $theme)
		echo "\t<option class=\"option\" value=\"$file\" selected>$file</option>\n";
	else
		echo "\t<option class=\"option\" value=\"$file\">$file</option>\n";
}
closedir($dp);
?>
</select></td>
</tr><tr>
<td class="formtag">timestamps:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="timestamps" value="1" /> prefixes every line with the current time</td>
</tr><tr>
<td class="formtag">smilies:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="smilies" value="1" checked /> converts ASCII smilies into images</td>
</tr><tr>
<td class="formtag">local echo:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="localecho" value="1" checked /> echoes your typed command to the screen</td>
</tr><tr>
<td class="formtag">IRC emulation:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="ircemu" value="1" /> emulates IRC-style chat</td>
</tr><tr>
<td class="formtag">sounds:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="sounds" value="1" /> turns sound on</td>
</tr><tr>
<td class="formtag">swearfilter:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="swearfilter" value="1" /> asterisks-out rude words</td>
</tr><tr>
<td class="formtag">hinames:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="hinames" value="1" checked /> highlights your username</td>
</tr><tr>
<td class="formtag">colours:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="colours" value="1" checked /> turns colours on</td>
</tr><tr>
<td class="formtag">new window:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="newwindow" value="1" checked /> opens the connection in a new window</td>
</tr><tr>
<td class="formtag">debugging:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="debugging" value="1" /> turns the debugger on</td>
</tr><tr>
<td colspan="2" align="center" valign="middle" style="height:40px; background: black; border-top: 1px solid black; "><input type="submit" class="button" value="&gt;- connect -&lt;" /></td>
</tr>
</form>
</table>
<br/>
to go straight to the default console, <a href="frameset.php">click here</a>.
</center>
</body>
</html>
