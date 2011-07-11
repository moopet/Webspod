<?php
// hc_index.php
/*
	same as index.php except for hard coded talker details - you don't
	get the menu of talkers or host/port options on the login form
*/

include "config.php";

if (!$HARD_CODED_TALKER)
{
	header("location:index.php");
	exit;
}

if (!isset($hostname))
	$freshpage = 1;

if (!isset($theme))
	$theme = $DEFAULT_THEME;

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<meta name="keywords" content="spod,talker,telnet,web,html,interface,cgi,firewall,terminal,free,software,service,browser,ew-too,nuts,amnuts,pg+,pg96,summink,sensei-summink">
<meta name="description" content="Webspod is an HTML interface for connecting to talkers. You run it on a webserver and then browse to it to spod. It is free software, released under the GNU General Public License. It allows connection to telnet-only services through many firewalls">
<meta http-equiv="Content-Language" content="EN">
<meta name="author" content="Ben Sinclair (moopet)">
<meta name="distribution" content="Global">
<meta name="copyright" content="Copyright © 2003 Ben Sinclair">
<meta name="robots" content="FOLLOW,INDEX">
<title>webspod login page</title>
<link rel="stylesheet" href="system.css" />
</head>
<body>
<font class="menu">connect</font>&nbsp;
<a class="menu" href="users.php">users</a>&nbsp;
<a class="menu" href="help.html">help</a>&nbsp;
<hr/><br/>
<table border="0" align="center" cellspacing="0" cellpadding="0" style="border: 1px solid black">
<form name="loginform" method="post" action="frameset.php">
<tr>
<tr><td colspan="2" style="padding:0px; width:400; height:70; background: url(images/banner.jpg); text-align:right" valign="top"><a class="about" href="help.html">help</a></td>
</tr><tr>
<td colspan="2" style="border-top: 1px solid black">&nbsp;</td>
</tr><tr>
<td class="formtag">talker:&nbsp;</td>
<td><input value="<?php echo $DEFAULT_TALKER_NAME ?>" size="30" readonly /></td>
</tr><tr>
<td class="formtag">hostname:&nbsp;</td>
<td><input name="hostname" value="<?php echo $DEFAULT_HOSTNAME ?>" size="30" readonly /></td>
</tr><tr>
<td class="formtag">port:&nbsp;</td>
<td><input name="port" value="<?php echo $DEFAULT_PORT ?>" size="5" readonly /></td>
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
<td class="notes"><input class="toggle" type="checkbox" name="localecho" value="1" /> echoes your typed command to the screen</td>
</tr><tr>
<td class="formtag">IRC emulation:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="ircemu" value="1" /> emulates IRC-style chat</td>
</tr><tr>
<td class="formtag">sounds:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="sounds" value="1" /> turns sounds on</td>
</tr><tr>
<td class="formtag">swearfilter:&nbsp;</td>
<td class="notes"><input class="toggle" type="checkbox" name="swearfilter" value="1" /> asterisks out rude words</td>
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
<td colspan="2">&nbsp;</td>
</tr><tr>
<td colspan="2" align="center" valign="middle" style="height: 70px; border-top: 1px solid black; background: url(images/banner2.jpg)"><input type="submit" class="button" value="&gt;- connect -&lt;" /></td>
</tr>
</form>
</table>
</body>
</html>
