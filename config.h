/*	
	Webspod - a browser interface for talkers.

	File: config.c
	Copyright 2003 Ben Sinclair (ben@moopet.net)

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* defaults */

#define DEFAULT_COLOURS		1
#define DEFAULT_HINAMES		1
#define DEFAULT_IRCEMU		0
#define DEFAULT_LOCALECHO	1
#define DEFAULT_SMILIES		1
#define DEFAULT_SOUNDS		0
#define DEFAULT_SWEARFILTER	0
#define DEFAULT_TIMESTAMPS	0


/* security */
/*
    ALLOW_LOW_PORTS is here to let you block or unblock access to ports < 1024
	in case someone wants to use it to telnet to places or fake mail or sommat.
*/
#define ALLOW_LOW_PORTS		0

/*
	USE_SYSLOG, when set to 1, logs all fatal errors, under the ident
	"webspod".
*/
#define USE_SYSLOG			1

/*
	HARD_CODED_TALKER hard code in the hostname and port to the DEFAULT_...
	of your talker so people can't use your bandwidth for connecting to other
	talkers. It's a security measure in that people can't use it for remote,
	"anonymous"	telnet sessions to other people's servers. Leave it
	zero if you want normal open behaviour. You must also set
	$HARD_CODED_TALKER to 1 in config.php if you don't use a custom loader.
*/
#define HARD_CODED_TALKER	1
#define DEFAULT_HOSTNAME	"portugal-virtual.org"
#define DEFAULT_PORT		6969

/*	CHECK_IDENT will eventually link into a simple ident server for ident
	forwarding (where available). At the moment, it just checks and prints the
	client's ident information as a system message. It may cause problems on
	systems with odd or unavailable ident, and hasn't been very fully tested.
	FORWARD_IDENT toggles whether this information is presented to the talker.
	Obviously for most systems, FORWARD_IDENT will be 1 if CHECK_IDENT is.
*/
#define CHECK_IDENT			0
#define FORWARD_IDENT		1

/* themes */
#define THEMES_PATH			"themes/"
#define DEFAULT_THEME		"mastodon"
#define ENABLE_THEME_SCRIPTS 1

/* system settings */

/*
    TEMP_PATH needs to exist and be writable to the user apache runs as (duh)
	Since it's just prepended to the temporary file names, you can use
	/tmp/webspod/
	or
	/tmp/webspod~
	if you can't guarantee there will be a webspod directory (like me)
*/
#define TEMP_PATH			"/tmp/webspod~"

/*
	CONSOLE_PREFIX can be changed here to any character you like, since there
	are conceivably situations where a slash is a conflict of interest :)
*/
#define CONSOLE_PREFIX		'/'

/*
	HIGHLIGHT_LINKS enables link highlighting if set to 1.
	LINK_SHORTEN is used to crop the text inside an anchor. Stops a couple of
	buggy browser problems and makes the text on screen more readable. Set it
	to 0 to disable this feature. Only applicable if HIGHLIGHT_LINKS is 1,
	obviously. If set to over MAX_VARLEN, it's ignored.
*/
#define HIGHLIGHT_LINKS		1
#define LINK_SHORTEN		120

/*
	If CATCH_ALL_INPUT is set to 1, clicking anywhere outside the command box
	will simply result in the command box being refocussed. Great for tabbing
	to other windows then back again without having to click the input box by
	mouse, but it also means you can't type a new address into the window to
	navigate away. Could be seen as a good thing, the option is here to let
	you turn it off if it causes a problem. It means people can't highlight
	text for cutting and pasting, for example.
*/
#define	CATCH_ALL_INPUT		0

/*
	If AUTO_RUN_FIXUP is set to 1, fixup() will run when webspod is launched.
*/
#define AUTO_RUN_FIXUP		0
