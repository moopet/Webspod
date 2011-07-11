<?php
// config.php

$TEMP_PATH = "/tmp/webspod~";

$TALKER_FILE = $TEMP_PATH . "talkers";
$THEME_PATH = "themes";
$DEFAULT_THEME = "mastodon";
$DEFAULT_HOSTNAME = "portugal-virtual.org";
$DEFAULT_PORT = 6969;
/*
	$DEFAULT_TALKER_NAME is currently only ever used if $HARD_CODED_TALKER
	is set (see below)
*/
$DEFAULT_TALKER_NAME = "Portugal Virtual";

/*
	set $HARD_CODED_TALKER to 1 if you don't want people to be able to
	chose where to connect. You must remember to also #define
	HARD_CODED_TALKER in config.h
*/
$HARD_CODED_TALKER = 1;
?>