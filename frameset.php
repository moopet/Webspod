<?php
// frameset.php

if ($newwindow)
{
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
<noscript>
<meta http-equiv="refresh" content="0;url=nojs.html" />
</noscript>
<script type="text/javascript">
<!--
window.open("frameset.php?theme=<?php echo $theme ?>&hostname=<?php echo $hostname ?>&smilies=<?php echo $smilies ?>&sounds=<?php echo $sounds ?>&swearfilter=<?php echo $swearfilter ?>&debugging=<?php echo $debugging ?>&port=<?php echo $port ?>&localecho=<?php echo $localecho ?>&ircemu=<?php echo $ircemu ?>&timestamps=<?php echo $timestamps ?>&colours=<?php echo $colours ?>&hinames=<?php echo $hinames ?>","_blank", "height=600,width=760,status=no,toolbar=no,menubar=no,location=no,resizable=yes,scrollbars=no")
self.location='index.php?theme=<?php echo $theme ?>&hostname=<?php echo $hostname ?>&port=<?php echo $port ?>'
// -->
</script>
</head>
</body>
</html>
<?php
	exit;
}
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Frameset//EN"
   "http://www.w3.org/TR/html4/frameset.dtd">
<html>
<head>
<noscript>
<meta http-equiv="refresh" content="0;url=nojs.html" />
</noscript>
<meta name="keywords" content="spod,talker,telnet,web,html,interface,cgi,firewall,terminal,free,software,service,browser,ew-too,nuts,amnuts,pg+,pg96,summink,sensei-summink">
<meta name="description" content="Webspod is an HTML interface for connecting to talkers. You run it on a webserver and then browse to it to spod. It is free software, released under the GNU General Public License. It allows connection to telnet-only services through many firewalls">
<meta http-equiv="Content-Language" content="EN">
<meta name="author" content="Ben Sinclair (moopet)">
<meta name="distribution" content="Global">
<meta name="copyright" content="Copyright © 2003 Ben Sinclair">
<meta name="robots" content="FOLLOW,INDEX">
<title><?php echo $SERVICE_NAME ?></title>
<script type="text/javascript">
<!--
var suppressCloser=false;

function window_onbeforeunload() {
	event.returnValue="This will terminate your session, requiring you to Logon again to continue."
}
// -->
</script>
</head>
<frameset rows="20,*,20,0,40" marginwidth="0" marginheight="0" frameborder="0" framespacing="0" border="0" onBeforeUnload="window_onbeforeunload()">
	<frameset cols="20,*,20" frameborder="0" border="0">
		<frame name="topleft" src="frame.php?position=topleft&theme=<?php echo $theme ?>" scrolling="no" noresize>
		<frame name="topbit" src="frame.php?position=top&theme=<?php echo $theme ?>&hostname=<?php echo $hostname ?>&port=<?php echo $port ?>" scrolling="no">
		<frame name="topright" src="frame.php?position=topright&theme=<?php echo $theme ?>" scrolling="no" noresize>
	</frameset>
	<frameset cols="20,*,20" frameborder="0" border="0">
		<frame name="leftside" src="frame.php?position=leftside&theme=<?php echo $theme ?>" scrolling="no">
		<frame name="output" src="output.cgi?theme=<?php echo $theme ?>&hostname=<?php echo $hostname ?>&port=<?php echo $port ?>&debugging=<?php echo $debugging ?>&ircemu=<?php echo $ircemu ?>&sounds=<?php echo $sounds ?>&swearfilter=<?php echo $swearfilter ?>&smilies=<?php echo $smilies ?>&localecho=<?php echo $localecho ?>&timestamps=<?php echo $timestamps ?>&hinames=<?php echo $hinames ?>&colours=<?php echo $colours ?>" marginwidth="2" marginheight="0" scrolling="yes">
		<frame name="rightside" src="frame.php?position=rightside&theme=<?php echo $theme ?>" scrolling="no">
	</frameset>
	<frameset cols="20,*,20" frameborder="0" border="0">
		<frame name="bottomleft" src="frame.php?position=bottomleft&theme=<?php echo $theme ?>" scrolling="no" noresize>
		<frame name="bottom" src="frame.php?position=bottom&theme=<?php echo $theme ?>" scrolling="no">
		<frame name="bottomright" src="frame.php?position=bottomright&theme=<?php echo $theme ?>" scrolling="no" noresize>
	</frameset>
	<frame name="pinger" src="loading.php?pinger=1" marginwidth="0" marginheight="0" scrolling="no" noresize>
	<frame name="commandprompt" src="loading.php?theme=<?php echo $theme ?>" marginwidth="6" marginheight="8" scrolling="no">
</frameset>
</html>