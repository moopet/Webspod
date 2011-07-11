<?php include "config.php" ?>
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
<?php if (isset($theme)) echo "<link rel=\"stylesheet\" href=\"" . $THEME_PATH . "/" . $theme . "/theme.css\" />" ?>
</head>
<body class="input">
<?php
if (!isset($pinger))
	echo "[ Command prompt will be automatically enabled only when a session is established ]";
?>
</body>
</html>