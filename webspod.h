/*	
	Webspod - a browser interface for talkers.

	File: webspod.h
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

#include <sys/types.h>
#define TELOPTS
#include <arpa/telnet.h>
#include <dirent.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

#define VERSION_MSG			"1.2 beta"
#define COPYRIGHT_MSG		"copyright &copy; 2003 Ben Sinclair (<a target=\"_top\" title=\"click to open link in new window\" href=\"http://moopet.net\">moopet</a>)"
#define MAX_VARLEN			256
#define BUFFER_SIZE			1024
#define SCROLL_SCRIPT		"<script type=\"text/javascript\">window.scroll(0,143165576)</script>"
#define KEEP_ALIVE_INTERVAL	30
#define MAX_MISSED_TIMEOUTS	4
#define HTTP_KEEP_ALIVE		"<i></i>"
#define TERMINAL_TYPE		"webspod"

/* signals */
#define SIGSUBMIT			SIGUSR1
#define SIGPING				SIGUSR2

/* normal text */
#define NOR					"\033[m"
#define NOR2				"\033[0m"
#define UNDERLINE			ansi_other[0]
#define ITALIC				ansi_other[1]
#define BOLD				ansi_other[2]

/* stealth command codes */
#define WAITGA_SWHO			1

/* botch because telnet.h (my one anyway) is broken */
char *_telcmds[] = {
	"EOF", "SUSP", "ABORT", "EOR",
	"SE", "NOP", "DMARK", "BRK", "IP", "AO", "AYT", "EC",
	"EL", "GA", "SB", "WILL", "WONT", "DO", "DONT", "IAC", 0,
};
char *_telopts[NTELOPTS+1] = {
	"BINARY", "ECHO", "RCP", "SUPPRESS GO AHEAD", "NAME",
	"STATUS", "TIMING MARK", "RCTE", "NAOL", "NAOP",
	"NAOCRD", "NAOHTS", "NAOHTD", "NAOFFD", "NAOVTS",
	"NAOVTD", "NAOLFD", "EXTEND ASCII", "LOGOUT", "BYTE MACRO",
	"DATA ENTRY TERMINAL", "SUPDUP", "SUPDUP OUTPUT",
	"SEND LOCATION", "TERMINAL TYPE", "END OF RECORD",
	"TACACS UID", "OUTPUT MARKING", "TTYLOC",
	"3270 REGIME", "X.3 PAD", "NAWS", "TSPEED", "LFLOW",
	"LINEMODE", "XDISPLOC", "OLD-ENVIRON", "AUTHENTICATION",
	"ENCRYPT", "NEW-ENVIRON",
	0,
};
#define	_TELCMD(x)	_telcmds[(x)-TELCMD_FIRST]
#define	_TELOPT(x)	_telopts[(x)-TELOPT_BINARY]
#define _TELOPT_OK(x) ((unsigned int)(x) <= TELOPT_NEW_ENVIRON)
/* end botch */

/*

	colours:				BLACK			RED				GREEN			YELLOW
							BLUE			MAGENTA			CYAN			WHITE
*/

char *ansi_fg_colours[] = {	"\033[30m",		"\033[31m",		"\033[32m",		"\033[33m", 
							"\033[34m",		"\033[35m",		"\033[36m",		"\033[37m",
							"\033[0;30m",	"\033[0;31m",	"\033[0;32m",	"\033[0;33m", 
							"\033[0;34m",	"\033[0;35m",	"\033[0;36m",	"\033[0;37m",
							"\033[1;30m",	"\033[1;31m",	"\033[1;32m",	"\033[1;33m",
							"\033[1;34m",	"\033[1;35m",	"\033[1;36m",	"\033[1;37m",
							0 };

char *html_colours[] = {	"#000000",		"#800000",		"#109010",		"#909010", 
							"#101090",		"#901090",		"#0080FF",		"#C0C0C0",
							"#000000",		"#800000",		"#109010",		"#909010", 
							"#101090",		"#901090",		"#0080FF",		"#C0C0C0",
							"#606060",		"#FF0000",		"#10FF10",		"#FFFF00",
							"#1010FF",		"#FF00FF",		"#00FFFF",		"#FFFFFF",
							0 };

char *ansi_bg_colours[] = {	"\033[40m",		"\033[41m",		"\033[42m",		"\033[43m",
							"\033[44m",		"\033[45m",		"\033[46m",		"\033[47m",
							0 };

char *ansi_other[] =	{	"\033[4m",		"\033[5m",		"\033[1m",
							0 };

char *html_other[] =	{	"text-decoration: underline;", "font-style: italic", "font-weight: bold",
							0 };

char *ascii_smilies[] = {	":)",			";)",			":(",			";(",
							":P",			":p",			";P",			";p",
							":>",			";>",			":->",			";->",
							":\\",			";\\",			":-\\",			";-\\",
							":/",			";/",			":-/",			";-/",
							0 };

char *graphical_smilies[]={	"happy",		"wink",			"sad",			"sad",
							"tongue",		"tongue",		"tongue",		"tongue",
							"vgrin",		"vgrin",		"vgrin",		"vgrin",
							"wonky",		"wonky",		"wonky",		"wonky",
							"wonky2",		"wonky2",		"wonky2",		"wonky2",
							0 };

char *banned_words[] =	{ "arse", "crap", "cunt", "dickhead", "felch", "fuck", "piss", "shit", 0 };
