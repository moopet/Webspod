/*	
	Webspod - a browser interface for talkers.

	File: output.c
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

#include "webspod.h"

#define HR		"--------------------------------------------------"

#define bell()	play_sound("%s%s/bell.wav", THEMES_PATH, theme)

/******************************** local prototypes **************************/

typedef void command_func(char *, char *);

/* internal subroutines */
time_t	filetime(char *);
int		trim_cr(char *);

/* signal handlers */
void	input_handler(int);
void	meta_handler(int);

/* formatted output routines */
void	debug_message(char *, ...);
void	local_echo(char *);
void	fatal_error(char *, ...);
void	system_message(char *, ...);
void	to_browser(char *, ...);

/* theme routines & frame control */
void	retitle(char *, ...);
int		refresh_theme(char *);
void	enable_input_window(void);
void	disable_input_window(void);

/* telopt routines */
int		handle_telopt(unsigned char *, char *);
void	send_telopt(unsigned char, unsigned char);

/* console commands worthy of their own functions */
command_func command_beep;
command_func command_clone;
command_func command_listcommands;
command_func command_disconnect;
command_func command_help;
command_func command_listthemes;
command_func command_listusers;
command_func command_listworlds;
command_func command_mark;
command_func command_reconnect;
command_func command_repeat;
command_func command_set;
command_func command_spodlist;
command_func command_status;
command_func command_telnet;
command_func command_version;
command_func command_world;
int irc_helper(char *, char *);

/* main routines */
int		get_talker_name(void);
int		overrun_buffer(char *);
void	get_client_info(void);
void	get_form_vars(void);
char	*extract_var(char *, char *);
char	*html_anchor(char *);
void	play_sound(char *, ...);
char	*get_timestamp(void);
int		replace_colours(char *);
int		replace_username(char *);
int		replace_smilies(char *);
void	replace_swearing(char *);
int		replace_URLs(char *);
void	get_ident(void);
void	to_talker(char *, ...);
void	connect_talker(void);
int		connect_talker_by_name(char *);
void	disconnect_talker(void);
void	fixup(void);
void	update_pidfile(void);
void	delete_pidfile(void);

/******************************** global variables **************************/

struct 
{
	command_func	*function;
	char			*command, *alias;
} command_list [] = {
	{ command_beep,			"beep",			0				},
	{ command_clone,		"clone",		0				},
	{ command_disconnect,	"dc",			"disconnect"	},
	{ command_help,			"help",			0				},
	{ command_listcommands,	"listcommands",	"commands"		},
	{ command_listthemes,	"listthemes",	0				},
	{ command_listusers,	"listusers",	"wsctl"			},
	{ command_listworlds,	"listworlds",	0				},
	{ command_mark,			"mark",			0				},
	{ command_reconnect,	"reconnect",	0				},
	{ command_repeat,		"repeat",		0				},
	{ command_set,			"set",			0				},
	{ command_spodlist,		"spodlist",		0				},
	{ command_status,		"status",		0				},
	{ command_telnet,		"telnet",		0				},
	{ command_version,		"version",		0				},
	{ command_world,		"world",		"connect"		},
	{ 0,					0,				0				}
};

char		talkername[MAX_VARLEN], username[MAX_VARLEN], hostname[MAX_VARLEN], ident[MAX_VARLEN];
char		lastline[MAX_VARLEN], *client_ip, *client;
int			prompt_enabled = 0, input_counter = 0, port = 0, connected = 0, ewe_editor = 0;
int			missed_timeouts = 0, sockfd = -1, newline = 1, iacga = 0;
int			net_bytes = 0, net_packets = 0, net_average = 0, passwordmode = 0;
time_t		session_time, connection_time;
unsigned	pid;

/************************** session variables (defaults) *********************/

int			colours		= DEFAULT_COLOURS;
int			debugging	= 0;
int			hinames		= DEFAULT_HINAMES;
int			ircemu		= DEFAULT_IRCEMU;
int			localecho	= DEFAULT_LOCALECHO;
int			smilies		= DEFAULT_SMILIES;
int			sounds		= DEFAULT_SOUNDS;
int			swearfilter	= DEFAULT_SWEARFILTER;
int			timestamps	= DEFAULT_TIMESTAMPS;
char		theme[MAX_VARLEN];


/*****************************************************************************
	function:	filetime
	purpose:	subroutine to get the last-modified time of a file
	returns:	timestamp on file
*****************************************************************************/
time_t filetime(char *filename)
{
	struct stat	fs;

	if (stat(filename, &fs) < 0)
		return 0;

	return fs.st_mtime;
}


/*****************************************************************************
	function:	trim_cr
	purpose:	subroutine to remove trailing carriage returns from a string
	returns:	number of caracters removed
*****************************************************************************/
int trim_cr(char *s)
{
	int n = 0;

	while (s[strlen(s) - 1] == '\n')
	{
		s[strlen(s) - 1] = 0;
		n++;
	}
	return n;
}

/***************************************************************************** 
	function:	get_talker_name
	purpose:	sets the global 'talkername' to the name associated with
				'hostname' and 'port' in the talker list file.
	returns:	1 on success, 0 on failure.
*****************************************************************************/
int get_talker_name()
{
	char	filename[MAX_VARLEN], line[MAX_VARLEN], details[MAX_VARLEN], *temp;
	FILE	*fp;

	talkername[0] = 0;

	sprintf(filename, "%stalkers", TEMP_PATH);
	fp = fopen(filename, "r");
	if (!fp)
	{
		system_message("warning: talker details file is unavailable!");
		return 0;
	}

	sprintf(details, "%s %d", hostname, port);

	while (fgets(line, MAX_VARLEN, fp))
		if (strstr(line, details))
		{
			fclose(fp);
			temp = strchr(line, ',');
			if (!temp)
			{
				system_message("warning: talker details file is corrupt!");
				return 0;
			}
			*temp = 0;
			strcpy(talkername, line);
			return 1;
		}

	fclose(fp);
	return 0;
}


/*****************************************************************************
	function:	connect_talker_by_name
	purpose:	Looks up the global 'talkername' in the talker list file.
				Sets 'hostname' and 'port' and calls connect_talker()
	returns:	1 on success, 0 on failure.
*****************************************************************************/
int connect_talker_by_name(char *str)
{
	char	filename[MAX_VARLEN], line[MAX_VARLEN], *temp, *temp2;
	FILE	*fp;

	talkername[0] = 0;

	while (*str == ' ')
		str++;
	trim_cr(str);
	temp = str;
	while (*temp)
		*temp++ = tolower(*temp);
	sprintf(filename, "%stalkers", TEMP_PATH);
	fp = fopen(filename, "r");
	if (!fp)
	{
		system_message("warning: talker details file is unavailable!");
		return 0;
	}

	while (fgets(line, MAX_VARLEN, fp))
	{
		temp = line;
		while (*temp)
			*temp++ = tolower(*temp);
		temp = strchr(line, ',');
		if (!temp)
		{
			system_message("warning: talker details file is corrupt!");
			system_message("cannot match talker '%s'", str);
			fclose(fp);
			return 0;
		}

		*temp = 0;
		if (strstr(line, str))
		{
			if (strcasecmp(temp, str) != 0)
			{
				system_message("searching for talker '%s'", str);
				system_message("matched talker '%s'", line);
			}

			temp += 13;
			temp2 = strrchr(temp, ' ');
			if (!temp2)
			{
				system_message("warning: cannot extract details for talker '%s'", line);
				system_message("warning: talker details file may be corrupt!");
				fclose(fp);
				return 0;
			}
			*temp2++ = 0;
			
			port = atoi(temp2);
			strcpy(hostname, temp);
			strcpy(talkername, line);
			connect_talker();
			return 1;
		}
	}

	system_message("cannot match talker '%s'", str);
	fclose(fp);
	return 0;
}


/*****************************************************************************
	function:	get_form_vars
	purpose:	Runs through the submitted browser form variables and calls
				extract_var() as needed to assign values to globals.
	returns:	nothing.
*****************************************************************************/
void get_form_vars(void)
{
	char	vars[2048], request[10], *temp;
	int		length;

	if(!getenv("REQUEST_METHOD"))
	{
		system_message("couldn't get passed form REQUEST_METHOD");
		return;
	}
	
	strncpy(request, getenv("REQUEST_METHOD"), 9);
	request[9] = 0;
	
	if(!strlen(request))
	{
		system_message("couldn't get passed form request");
		return;
	}

	if(strncmp(request, "GET", 3) == 0)
	{
		strncpy(vars, getenv("QUERY_STRING"), 2048);
		vars[2047] = 0;
		if(!strlen(vars))
		{
			system_message("couldn't get passed QUERY_STRING");
			return;
		}
	}
	else if(strncmp(request, "POST", 4) == 0)
	{
		if(!getenv("CONTENT_LENGTH"))
		{
			system_message("couldn't get passed form CONTENT_LENGTH");
			return;
		}
		length = atoi(getenv("CONTENT_LENGTH"));
		if(!length || length == 0)
		{
			system_message("couldn't get passed form CONTENT_LENGTH");
		return;
		}
		
		fread(vars, length > 2048 ? 2048 : length, 1, stdin);
		vars[length] = 0;
	}
	else
	{
		system_message("passed form REQUEST_METHOD is not known");
		return;
	}

	if (HARD_CODED_TALKER)
	{
		strcpy(hostname, DEFAULT_HOSTNAME);
		port = DEFAULT_PORT;
		system_message("using hard-coded talker '%s:%d'", hostname, port);
	}
	else
	{
		temp = extract_var(vars, "hostname");
		if (temp)
			strncpy(hostname, temp, MAX_VARLEN);
		temp = extract_var(vars, "port");
		if (temp)
			port = atoi(temp);
	}

	temp = extract_var(vars, "timestamps");
	if (temp)
		timestamps = atoi(temp);
	temp = extract_var(vars, "debugging");
	if (temp)
		debugging = atoi(temp);
	temp = extract_var(vars, "smilies");
	if (temp)
		smilies = atoi(temp);
	temp = extract_var(vars, "colours");
	if (temp)
		colours = atoi(temp);
	temp = extract_var(vars, "localecho");
	if (temp)
		localecho = atoi(temp);
	temp = extract_var(vars, "hinames");
	if (temp)
		hinames = atoi(temp);
	temp = extract_var(vars, "ircemu");
	if (temp)
		ircemu = atoi(temp);
	temp = extract_var(vars, "sounds");
	if (temp)
		sounds = atoi(temp);
	temp = extract_var(vars, "swearfilter");
	if (temp)
		swearfilter = atoi(temp);
	temp = extract_var(vars, "theme");
	if (temp)
		strncpy(theme, temp, MAX_VARLEN);
	else
		strcpy(theme, DEFAULT_THEME);

	talkername[0] = 0;
}


/*****************************************************************************
	function:	extract_var
	purpose:	subroutine called by get_form_vars() for string manipulation
	returns:	pointer to string value of specified form variable, or null.
*****************************************************************************/
char *extract_var(char *vars, char *name)
{
	static char		temp[128];
	char			*pos, *end;
	int				length;

	sprintf(temp, "%s=", name);
	pos = (char *) strstr(vars, temp);
	if (!pos)
		return 0;

	pos += strlen(name) + 1;
	length = 0;
	for (end = pos; *end != 0 && *end != '&'; end++)
		length++;
	bzero(temp, 127);
	if (length > 127)
		length = 127;
	strncpy(temp, pos, length);
	return (char *) temp;
}


/*****************************************************************************
	function:	system_message
	purpose:	outputs a formatted message to the browser using the 'sysmsg'
				class of the current theme
	returns:	nothing
*****************************************************************************/
void system_message(char *format, ...)
{
    char	buffer[512];
	va_list	arguments;

	va_start(arguments, format);
	vsprintf(buffer, format, arguments);
	va_end(arguments);

	if (!newline)
		printf("<br/>");
	to_browser("<font class='server'>[</font><font class='sysmsg'>sysmsg</font><font class='server'>]</font>&nbsp;<font class='normal'>%s</font>\n", buffer);
}


/*****************************************************************************
	function:	debug_message
	purpose:	outputs a formatted message to the browser using the 'debug'
				class of the current theme ONLY IF debugging is turned on.
	returns:	nothing
*****************************************************************************/
void debug_message(char *format, ...)
{
    char	buffer[512];
	va_list	arguments;

	if (!debugging)
		return;
	va_start(arguments, format);
	vsprintf(buffer, format, arguments);
	va_end(arguments);

	if (!newline)
		printf("<br/>");
	printf("<font class='server'>[</font><font class='debug'>_debug</font><font class='server'>]</font>&nbsp;<font class='normal'>%s", buffer);
	to_browser("</font>\n");
}


/*****************************************************************************
	function:	local_echo
	purpose:	outputs an unformatted string to the browser using the
				'localecho'	class of the current theme ONLY IF localecho
				is turned on and it's not a password blanking stage.
	returns:	nothing
*****************************************************************************/
void local_echo(char *str)
{
	int i;

/* prevents echoing username/password back to the screen */
	if (!localecho || !connected)
		return;

	if (!newline)
		printf("<br/>");
	printf("<font class='server'>[</font><font class='localecho'>__sent</font><font class='server'>]</font>&nbsp;<font class='localecho_text'>");
	
	if (!*str)
		printf("&lt;enter&gt;");
	else if (passwordmode)
		printf("********");
	else
		while (*str)
		{
			switch(*str)
			{
				case '<':	printf("&lt;");
							break;
				case '>':	printf("&gt;");
				case '\n':	break;
				default:	putchar(*str);
			}
			str++;
		}
	to_browser("</font>\n");
}


/*****************************************************************************
	function:	fatal_error
	purpose:	outputs a formatted message to the browser using the 'error'
				class of the current theme, disables the command prompt and
				cleanly exits the application
	returns:	nothing
*****************************************************************************/
void fatal_error(char *format, ...)
{
    char	string[512], dummy = 0;
	va_list	arguments;

	va_start(arguments, format);
	vsprintf(string, format, arguments);
	va_end(arguments);

    to_browser("<hr/><font class='server'>[</font><font class='error'>_FATAL</font><font class='server'>]</font>&nbsp;<font class='normal'>%s</font>", string);
	if (errno)
		to_browser(": %s", strerror(errno));
	to_browser("<hr/>\n");

	if (USE_SYSLOG)
		syslog(LOG_ERR, "%s%s%s. %s", string, errno ? ": " : &dummy,
				errno ? strerror(errno) : &dummy, username[0] ? username : "unknown user");
	
	if (sockfd >= 0)
		shutdown(sockfd, 2);
	disable_input_window();
	bell();
	printf("%s</font></pre>\n</body>\n</html>\n", SCROLL_SCRIPT);
	fflush(stdout);
	delete_pidfile();
	exit(0);
}


/***************************************************************************** 
	function:	fixup
	purpose:	removes dead lock files from the temporary directory. Called
				at startup.
	returns:	nothing
*****************************************************************************/
void fixup()
{
	DIR				*tempdir;
	struct dirent	*entry;
	int				i = 0, temppid;
	char			path[MAX_VARLEN], filename[MAX_VARLEN], temp[MAX_VARLEN], temp2[MAX_VARLEN], *s, *t;
	
	if (!AUTO_RUN_FIXUP)
		return;
	
	strcpy(path, TEMP_PATH);
	s = strrchr(path, '/');
	if (s)
		if (s - strlen(path) + 1 < (char *) path)
			*s++ = 0;

	tempdir = opendir(path);
	
	if (!tempdir)
	{
		debug_message("fixup  : couldn't open temporary folder");
		return;
	}

	temp[0] = 0;

	while (entry = readdir(tempdir))
	{
		strcpy(filename, entry->d_name);

		t = strstr(entry->d_name, "-lock");
		if (!t)
			continue;

		if (s)
			if (!strstr(entry->d_name, s))
				continue;

		*t = 0;

		if (s)
			t = entry->d_name + strlen(s);
		else
			t = entry->d_name;
		
		temppid = atoi(t);

		if (pid != temppid)
			if (kill(temppid, SIGUSR2) < 0)
			{
				sprintf(temp2, "%s/%s", path, filename);
				debug_message("fixup  : removing dead lock file for process %d (%s)", temppid, temp2);
				unlink(temp2);
				continue;
			}

		if (i)
		    strcat(temp, ", ");
		if (pid == temppid)
		{
			strcat(temp, "<b>");
			strcat(temp, t);
			strcat(temp, "</b>");
		}
		else
			strcat(temp, t);
		i++;
	}
	closedir(tempdir);

	if (i == 1)
		debug_message("fixup  : 1 service running: %s.", temp);
	else
		debug_message("fixup  : %d services running: %s.", i, temp);
}


/*****************************************************************************
	function:	html_anchor
	purpose:	takes a URL and formats it nicely as HTML
	returns:	a pointer to the formatted anchor string
*****************************************************************************/
char *html_anchor(char *url)
{
	static char anchor[MAX_VARLEN * 3], temp[MAX_VARLEN], action[50];

	if (strchr(url, '@'))
		strcpy(action, "click to send mail");
	else
		strcpy(action, "click to open link in new window");

	if (HIGHLIGHT_LINKS && LINK_SHORTEN)
	{
		if (LINK_SHORTEN >= MAX_VARLEN)
			strncpy(temp, url, MAX_VARLEN);
		else
			strncpy(temp, url, LINK_SHORTEN);
	}
	else
		strncpy(temp, url, MAX_VARLEN);

	if (strstr(url, "http://"))
			sprintf(anchor, "<a target=\"_blank\" title=\"%s\" href=\"%s\">%s</a>", action, url, temp);
	else
		sprintf(anchor, "<a target=\"_blank\" title=\"%s\" href=\"http://%s\">%s</a>", action, url, temp);
	return (char *) anchor;
}


/*****************************************************************************
	function:	update_pidfile
	purpose:	(re)creates the lock file in the temporary directory and
				fills it with useful process and connection information
	returns:	nothing
*****************************************************************************/
void update_pidfile()
{
	static	int started = 0;
	FILE	*fp;
	char	filename[MAX_VARLEN];

	sprintf(filename, "%s%d-lock", TEMP_PATH, pid);

	if (started)
	{
/*
		if (unlink(filename) != 0)
			fatal_error("connection dropped from console");
*/	
		unlink(filename);
	}
	else
		started = 1;

	fp = fopen(filename, "w");
	if (!fp)
		fatal_error("couldn't create lock file '%s'", filename);

	fprintf(fp, "username: %s\nhostname: %s\nport: %d\n", username, hostname, port);
	fprintf(fp, "ident: %s\naddress: %s\nclient: %s\n", ident, client_ip, client);
	fprintf(fp, "pid: %d\ntheme: %s", pid, theme);
	fclose(fp);
}


/*****************************************************************************
	function:	delete_pidfile
	purpose:	deletes the current lock file (if any) from the temporary
				directory
	returns:	nothing
*****************************************************************************/
void delete_pidfile()
{
	FILE	*fp;
	char	filename[MAX_VARLEN];

	sprintf(filename, "%s%d-lock", TEMP_PATH, pid);

	if (unlink(filename) < 0)
		debug_message("couldn't delete lock file '%s'", filename);
}


/*****************************************************************************
	function:	get_client_info
	purpose:	loads the globals 'client' and 'client_ip' with local browser-
				provided information
	returns:	nothing
*****************************************************************************/
void get_client_info()
{
	client_ip = getenv("HTTP_CLIENT_IP");
	if(!client_ip) 
	{
		client_ip = getenv("HTTP_X_FORWARDED_FOR");
		if(!client_ip) 
			client_ip = getenv("REMOTE_ADDR"); 
	}
	
	client = getenv("HTTP_USER_AGENT");

	if (debugging)
	{
		if (client_ip)
			debug_message("client IP address: %s", html_anchor(client_ip));
		else
			debug_message("warning: couldn't resolve client IP address!");
		if (client)
			debug_message("client software: %s", client);
		else
			debug_message("warning: couldn't determine client software!");
	}
}


/*****************************************************************************
	function:	get_ident
	purpose:	attempts to connect to the browser's address and query identd
				(if running). Results are loaded into the global 'ident'
	returns:	nothing
*****************************************************************************/
void get_ident()
{
	char				temp[100];
	int					s, server_port, i;
	struct sockaddr_in	serv_addr;
    struct hostent		*server;

	if (!CHECK_IDENT)
	{
		debug_message("ident  : ident forwarding disabled");
		strcpy(ident, "<unavailable>");
		return;
	}
	
	if (!client_ip)
	{
		system_message("ident  : ident forwarding unavailable");
		strcpy(ident, "<unavailable>");
		return;
	}

	server_port = atoi(getenv("SERVER_PORT"));
	if (!server_port)
		server_port = 80;
	
	system_message("ident  : checking ident");

	if (!getenv("REMOTE_PORT"))
	{
		if (!client_ip)
			system_message("ident  : couldn't resolve remote ident server");
		else
			system_message("ident  : &lt;unknown&gt;@%s", html_anchor(client_ip));
		return;
	}

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) 
	{
        system_message("ident  : error connecting to remote ident server");
		return;
	}

	server = gethostbyname(client_ip);
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(113);
    if (connect(s, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
        system_message("ident  : error connecting to remote ident server");
		close(s);
		return;
	}

	sprintf(temp, "%s,%d\n", getenv("REMOTE_PORT"), server_port);
	send(s, temp, strlen(temp), 0);
	recv(s, temp, 100, 0);
	close(s);

	strncpy(ident, strrchr(temp, ' ') + 1, MAX_VARLEN);
	for (i = strlen(ident) - 1; i; i--)
		if (ident[i] <= ' ')
			ident[i] = 0;
	system_message("ident  : %s@%s", ident, html_anchor(client_ip));
}


/*****************************************************************************
	function:	connect_talker
	purpose:	verbosely connects to the talker specified by the globals
				'hostname' and 'port'. If an existing connection to any server
				exists, it is cleanly disconnected first.
	returns:	nothing
*****************************************************************************/
void connect_talker()
{
    struct sockaddr_in	serv_addr;
    struct hostent		*server;

	if (port < 1024 && !ALLOW_LOW_PORTS)
	{
		system_message("connect: access to port %d forbidden", port);
		return;
	}

	ewe_editor = 0;
	net_bytes = net_packets = net_average = 0;

	disconnect_talker();

	if (get_talker_name())
		system_message("connect: connecting to %s:%d (%s)", html_anchor(hostname), port, talkername);
	else
		system_message("connect: connecting to %s:%d", html_anchor(hostname), port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
	{
		system_message("connect: error opening socket");
		return;
	}

	system_message("connect: resolving hostname: %s", html_anchor(hostname));
	server = gethostbyname(hostname);
    if (server == NULL)
    {
		system_message("connect: no such host: %s", html_anchor(hostname));
		return;
	}

	system_message("connect: opening port");
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		system_message("connect: error connecting to remote host");
		return;
	}

	connected = 1;
	connection_time = time(0);
	update_pidfile();

	send_telopt(DONT, TELOPT_ECHO);
	send_telopt(DONT, TELOPT_SGA);
	if (talkername[0])
		retitle("webspod|%s (connected)", talkername);
	else
		retitle("webspod|%s:%d (connected)", hostname, port);
}


/*****************************************************************************
	function:	disconnect_talker
	purpose:	disconnects from the current talker session (if connected)
	returns:	nothing
*****************************************************************************/
void disconnect_talker()
{
	if (connected)
	{
		connected = 0;
		net_bytes = net_packets = net_average = 0;

		system_message("closing port");
		if (sockfd > -1)
			shutdown(sockfd, 2);
		talkername[0] = 0;
		if (USE_SYSLOG)
		{
			if (talkername[0])
				syslog(LOG_INFO, "%s (%s@%s) disconnected from %s (%s:%d)",
						username[0] ? username : "unknown user", ident, client_ip, talkername, hostname, port);
			else
				syslog(LOG_INFO, "%s (%s@%s) disconnected from %s:%d",
						username[0] ? username : "unknown user", ident, client_ip, hostname, port);
		}
		retitle("webspod (not connected)");
	}
}


/*****************************************************************************
	function:	play_sound
	purpose:	makes the browser play a sound file ONLY IF sounds are enabled
	returns:	nothing
*****************************************************************************/
void play_sound(char *format, ...)
{
    char	buffer[MAX_VARLEN];
	va_list	arguments;

	if (!sounds)
		return;

	va_start(arguments, format);
	vsprintf(buffer, format, arguments);
	va_end(arguments);

	printf("<embed src=\"%s\" autostart=\"true\" width=\"0\" height=\"0\" hidden />", buffer);
	printf("<noembed><bgsound src=\"%s\" /></noembed>", buffer);
}


/*****************************************************************************
	function:	get_timestamp
	purpose:	subroutine to generate a formatted timestamp string for
				prefixing lines ONLY IF timestamping is turned on
	returns:	pointer to the formatted timestamp string
*****************************************************************************/
char *get_timestamp()
{
	static char timestamp[120], dummy = 0;
	time_t		t;
	struct tm	*time_now;
	
	if (!timestamps)
		return (&dummy);

	t = time(0);
	time_now = gmtime(&t);
	sprintf(timestamp, "<font class='server'>[_</font><font class='timestamp'>%02d:%02d</font><font class='server'>]</font> ", time_now->tm_hour, time_now->tm_min);

	return ((char *) timestamp);
}


/*****************************************************************************
	function:	to_talker
	purpose:	sends a formatted string to the talker
	returns:	nothing
*****************************************************************************/
void to_talker(char *format, ...)
{
	char	buffer[MAX_VARLEN];
	va_list	arguments;

	if (connected)
	{
		va_start(arguments, format);
		vsprintf(buffer, format, arguments);
		va_end(arguments);

		send(sockfd, buffer, strlen(buffer), 0);
	}
	else
		system_message("network: --| no command sent (not connected)");
}


/*****************************************************************************
	function:	to_browser
	purpose:	outputs a formatted message to the browser and includes any
				line breaks and scrolling script as required
	returns:	nothing
*****************************************************************************/
void to_browser(char *format, ...)
{
	char	buffer[512];
	va_list	arguments;

	va_start(arguments, format);
	vsprintf(buffer, format, arguments);
	va_end(arguments);

	printf(buffer);
	if (strchr(buffer, '\n') || strstr(buffer, "<br"))
		printf(SCROLL_SCRIPT);

	if (trim_cr(buffer))
		newline = 1;
	else
		newline = 0;
	fflush(stdout);
}


/*****************************************************************************
	function:	command_beep
	purpose:	sends a (themed) beep to the browser
	returns:	nothing
*****************************************************************************/
void command_beep(char *command, char *params)
{
	if (sounds)
	{
		bell();
		system_message("beep!");
	}
	else
		system_message("sound  : sounds are currently disabled");
}


/*****************************************************************************
	function:	command_clone
	purpose:	opens a duplicate connection window
	returns:	nothing
*****************************************************************************/
void command_clone(char *command, char *params)
{
	system_message("cloning session");
	printf("<script type=\"text/javascript\">\n<--\nwindow.open('frameset.php?hostname=%s&port=%d&theme=%s&ircemu=%d&timestamps=%d&localecho=%d&smilies=%d&sounds=%d&colours=%d&debugging=%d&swearfilter=%d', ",
			hostname, port, theme, ircemu, timestamps, localecho, smilies, sounds, colours, debugging, swearfilter);
	printf("'_blank', 'height=600,width=760,status=no,toolbar=no,menubar=no,location=no,resizable=yes,scrollbars=no')\n// -->\n</script>");
	fflush(stdout);
}


/*****************************************************************************
	function:	command_disconnect
	purpose:	disconnects from the talker (if connected)
	returns:	nothing
*****************************************************************************/
void command_disconnect(char *command, char *params)
{
	if (connected)
		disconnect_talker();
	else
		system_message("disconnect: not connected");
}


/*****************************************************************************
	function:	command_listcommands
	purpose:	lists all available commands
	returns:	nothing
*****************************************************************************/
void command_listcommands(char *command, char *params)
{
	int count;

	system_message("--------------- available commands ---------------");

	for (count = 0; command_list[count].command; count++)
		if (strcmp(command_list[count].command, "telnet") != 0 || ALLOW_LOW_PORTS)
		{
			if (command_list[count].alias)
				system_message("#%2d - %-16s or %s", count + 1, command_list[count].command, command_list[count].alias);
			else
				system_message("#%2d - %s", count + 1, command_list[count].command);
		}

	system_message("--------------- %2d commands listed ---------------", count);
}


/*****************************************************************************
	function:	command_listthemes
	purpose:	lists all available themes
	returns:	nothing
*****************************************************************************/
void command_listthemes(char *command, char *params)
{
	DIR				*themedir;
	struct dirent	*entry;
	int				count = 0;
	char			temp[MAX_VARLEN];

	themedir = opendir(THEMES_PATH);
	
	if (!themedir)
	{
		system_message("themes: couldn't open themes folder");
		return;
	}

	temp[0] = 0;
	while (entry = readdir(themedir))
	{
		if (entry->d_name[0] == '.')
			continue;

		count++;
		if (count == 1)
			system_message("-------------------- themelist -------------------");
		system_message("#%2d - %s", count, entry->d_name);
	}
	closedir(themedir);
	
	if (count == 0)
		system_message("worldlist is empty");
	else if (count == 1)
		system_message("----------------- 1 theme listed ------------------", count);
	else if (count > 1)
		system_message("---------------- %2d themes listed ----------------", count);
}


/***************************************************************************** 
	function:	command_listusers
	purpose:	lists who else is using webspod
	returns:	nothing
*****************************************************************************/
void command_listusers(char *command, char *params)
{
	FILE			*fp;
	DIR				*tempdir;
	struct dirent	*entry;
	int				count = 0, temppid;
	char			path[MAX_VARLEN], filename[MAX_VARLEN], temp[MAX_VARLEN], temp2[MAX_VARLEN], temp3[10], *s, *t;
	time_t			tt = 0;
	struct stat		fs;
	struct tm		*ft;
	
	strcpy(path, TEMP_PATH);
	s = strrchr(path, '/');
	if (s)
		if (s - strlen(path) + 1 < (char *) path)
			*s++ = 0;

	tempdir = opendir(path);
	
	if (!tempdir)
	{
		debug_message("wsctl  : couldn't open temporary folder");
		if (!debugging)
			system_message("wsctl  : unable to display list of users");
		return;
	}

	while (entry = readdir(tempdir))
	{
		strcpy(filename, entry->d_name);

		t = strstr(entry->d_name, "-lock");
		if (!t)
			continue;

		if (s)
			if (!strstr(entry->d_name, s))
				continue;

		*t = 0;

		if (s)
			t = entry->d_name + strlen(s);
		else
			t = entry->d_name;
		
		temppid = atoi(t);

		sprintf(temp2, "%s/%s", path, filename);

		if (pid != temppid)
			if (kill(temppid, SIGUSR2) < 0)
			{
				debug_message("wsctl  : removing dead lock file for process %d (%s)", temppid, temp2);
				unlink(temp2);
				continue;
			}

		if (stat(temp2, &fs) >= 0)
		{
			tt = fs.st_mtime;
			ft = localtime(&tt);
		}
		
		fp = fopen(temp2, "r");
		if (!fp)
		{
			debug_message("wsctl  : failed to open file '%s'", temp2);
			continue;
		}

		fgets(temp2, MAX_VARLEN, fp);
		trim_cr(temp2);

		if (!strchr(temp2, ' '))
		{
			fclose(fp);
			debug_message("wsctl  : failed to read from file '%s'", entry->d_name);
			continue;
		}

		count++;
		if (count == 1)
			system_message("-------------------- user list -------------------");

		if (tt)
			sprintf(temp, "#%2d - [%2d:%02d] %", count, ft->tm_hour, ft->tm_min);
		else
			sprintf(temp, "#%2d - [??:??] ", count);

		if (debugging)
		{
			sprintf(temp3, "[%5d] ", temppid);
			strcat(temp, temp3);
		}

		strcat(temp, strrchr(temp2, ' '));
		strcat(temp, "@");

		fgets(temp2, MAX_VARLEN, fp);
		trim_cr(temp2);
		strcat(temp, strrchr(temp2, ' ') + 1);
		strcat(temp, ":");
		fgets(temp2, MAX_VARLEN, fp);
		trim_cr(temp2);
		if (strlen(strrchr(temp2, ' ')) > 4)
			strcat(temp, strrchr(temp2, ' ') + 1);
		else
		{
			temp[strlen(temp) - 1] = 0;
			strcat(temp, "unconnected console session");
		}

		fclose(fp);
		
		if (pid == temppid)
			system_message("<b>%s</b>", temp);
		else
			system_message(temp);
	}
	closedir(tempdir);
	system_message(HR);
}


/*****************************************************************************
	function:	command_listworlds
	purpose:	lists all available worlds in the style of TinyFugue.
	returns:	nothing
*****************************************************************************/
void command_listworlds(char *command, char *params)
{
	char	filename[MAX_VARLEN], line[MAX_VARLEN], *temp, *temp2, dummy = 0;
	FILE	*fp;
	int		count = 0, shortform = 0;
	
	if (params)
	{
		temp = params;
		while (*temp)
			*temp++ = tolower(*temp);
		temp = strrchr(params, ' ');
		if (temp != strchr(params, ' '))
		{
			system_message("usage  : %c%s [-s|--short] [pattern]", CONSOLE_PREFIX, command);
			return;
		}
		if (!strncmp(params, "-s", 2))
		{
			shortform = 1;
			params = temp;
			if (params)
				while (*params == ' ')
					params++;
			else
				params = &dummy;
		}
	}
	else
		params = &dummy;

	sprintf(filename, "%stalkers", TEMP_PATH);
	fp = fopen(filename, "r");
	if (!fp)
	{
		system_message("warning: talker details file is unavailable!");
		return;
	}

	while (fgets(line, MAX_VARLEN, fp))
	{
		temp = line;
		while (*temp)
			*temp++ = tolower(*temp);
		temp = strchr(line, ',');
		if (!temp)
		{
			system_message("warning: talker details file is corrupt!");
			fclose(fp);
			return;
		}

		*temp = 0;
		if (strstr(line, params) || !params)
		{
			count++;
			if (count == 1)
				system_message("-------------------- worldlist -------------------");
			temp += 13;
			temp2 = strrchr(temp, ' ');
			if (!temp2)
			{
				system_message("warning: cannot extract details for talker '%s'", line);
				system_message("warning: talker details file may be corrupt!");
				fclose(fp);
				return;
			}
			*temp2++ = 0;
			
			trim_cr(temp2);
			if (shortform)
				system_message(line);
			else
				system_message("#%2d - %-30s - %s %s", count, line, temp, temp2);
		}
	}

	if (count == 0 && *params)
		system_message("no worlds match the pattern '%s'", params);
	else if (count == 0)
		system_message("worldlist is empty");
	else if (count == 1)
		system_message("----------------- 1 world listed ------------------", count);
	else if (count > 1)
		system_message("---------------- %2d worlds listed ----------------", count);
	fclose(fp);
}


/*****************************************************************************
	function:	command_reconnect
	purpose:	drops and reestablistes talker connection (if any)
	returns:	nothing
*****************************************************************************/
void command_reconnect(char *command, char *params)
{
	if (!port)
		system_message("connect: not currently connected");
	else
		connect_talker();
}


/*****************************************************************************
	function:	command_repeat
	purpose:	repeats last *talker* command a specified number of times
	returns:	nothing
*****************************************************************************/
void command_repeat(char *command, char *params)
{
	int i;

	if (passwordmode)
		system_message("repeat: cannot repeat whilst in password-blanking mode!");
	else if (!lastline[0])
		system_message("repeat: nothing to repeat!");
	else if (!params)
	{ 
		if (ircemu && lastline[1] != CONSOLE_PREFIX && !passwordmode > 1 && !ewe_editor)
			to_talker("say %s\n", lastline);
		else
			to_talker("%s\n", lastline);
		local_echo(lastline);
	}
	else
		for (i = atoi(params); i > 0; i--)
		{ 
			if (ircemu && lastline[1] != CONSOLE_PREFIX && !passwordmode && !ewe_editor)
				to_talker("say %s\n", lastline);
			else
				to_talker("%s\n", lastline);
			local_echo(lastline);
		}
}


/*****************************************************************************
	function:	command_set
	purpose:	sets a session variable
	returns:	nothing
*****************************************************************************/
void command_set(char *command, char *params)
{
	char	*temp;
	int		state = 0;

	if (params)
		if (strstr(params, "on") || strstr(params, "yes") || strstr(params, "1"))
			state = 1;

	if (!params)
	{
		system_message("---------------- current settings ----------------");
		system_message("colours             : %s", colours ? "on" : "off");
		system_message("debugging           : %s", debugging ? "on" : "off");
		system_message("hinames             : %s", hinames ? "on" : "off");
		system_message("IRC emulation       : %s", ircemu ? "on" : "off");
		system_message("localecho           : %s", localecho ? "on" : "off");
		system_message("smilies             : %s", smilies ? "on" : "off");
		system_message("sounds              : %s", sounds ? "on" : "off");
		system_message("swearfilter         : %s", swearfilter ? "on" : "off");
		system_message("theme               : %s", theme);
		system_message("timestamps          : %s", timestamps ? "on" : "off");
		system_message(HR);
	}
	else if (strstr(params, "debug"))
	{
		debugging = state;
		system_message("debugging mode is %s", state ? "on" : "off");
	}
	else if (strstr(params, "smilie") || strstr(params, "smiley"))
	{
		smilies = state;
		system_message("smilies are %s", state ? "on" : "off");
	}
	else if (strstr(params, "colour") || strstr(params, "color"))
	{
		colours = state;
		system_message("colours are %s", state ? "on" : "off");
	}
	else if (strstr(params, "local"))
	{
		localecho = state;
		system_message("local echo is %s", state ? "on" : "off");
	}
	else if (strstr(params, "hiname"))
	{
		hinames = state;
		system_message("hinames are %s", state ? "on" : "off");
	}
	else if (strstr(params, "timestamp"))
	{
		timestamps = state;
		system_message("timestamping is %s", state ? "on" : "off");
	}
	else if (strstr(params, "irc"))
	{
		ircemu = state;
		system_message("IRC emulation is %s", state ? "on" : "off");
	}
	else if (strstr(params, "sound"))
	{
		sounds = state;
		system_message("sounds are %s", state ? "on" : "off");
	}
	else if (strstr(params, "swear"))
	{
		swearfilter = state;
		system_message("swearfilter is %s", state ? "on" : "off");
	}
	else if (strstr(params, "theme"))
	{
		temp = strrchr(params, ' ');
		if (temp)
			refresh_theme(temp + 1);
		else
		{
			system_message("usage  : %c%s &lt;theme&gt;", CONSOLE_PREFIX, command);
			system_message("use %clistthemes to see what themes are available.", CONSOLE_PREFIX);
		}
	}
	else
	{
		system_message("usage  : %c%s &lt;variable&gt; &lt;on|off|value&gt;", CONSOLE_PREFIX, command);
		system_message("variables: colours, debugging, hinames, ircemu, localecho,");
		system_message("           smilies, sounds, swearfilter, theme, timestamps.");
	}
}


/*****************************************************************************
	function:	command_help
	purpose:	opens a new browser window pointed at the help page
	returns:	nothing
*****************************************************************************/
void command_help(char *command, char *params)

{
	printf("<script type=\"text/javascript\">\n<!--\nwindow.open('help.html', '_blank')\n// -->\n</script>");
	fflush(stdout);
}


/*****************************************************************************
	function:	command_mark
	purpose:	marks a horizontal line on the browser with optional text
	returns:	nothing
*****************************************************************************/
void command_mark(char *command, char *params)
{
	to_browser("\n<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\"><tr><td class=\"mark\"><font color=\"black\">[</font>__mark<font color=\"black\">]</font> %s</td></tr></table>\n", params);
}


/*****************************************************************************
	function:	command_spodlist
	purpose:	opens a new browser window looking up a user on spodlist.org
	returns:	nothing
*****************************************************************************/
void command_spodlist(char *command, char *params)
{
	if (params)
	{
		system_message("looking up '%s' on %s", params, html_anchor("spodlist.org"));
		printf("<script type=\"text/javascript\">\n<!--\nwindow.open('http://spodlist.org/%s', '_blank')\n// -->\n</script>", params);
		fflush(stdout);
	}
	else
		system_message("usage  : %c%s &lt;user&gt;", CONSOLE_PREFIX, command);
}


/*****************************************************************************
	function:	command_status
	purpose:	displays status of webspod and setting of current session
				variables.
	returns:	nothing
*****************************************************************************/
void command_status(char *command, char *params)
{
	time_t	bt;
	char	*temp;

	bt = filetime("output.cgi");
	temp = asctime(localtime(&bt));
	trim_cr(temp);

	system_message("----------------- current status -----------------");
	system_message("code version        : %s", VERSION_MSG);
	debug_message ("PID                 : %d", pid);
	debug_message("lock file           : %s%d-lock", TEMP_PATH, pid);
	debug_message("build date          : %s", temp);
	if (ircemu)
		system_message("IRC emulation mode  : %s", ewe_editor ? "EWE editor" : "normal");
	debug_message("client IP address   : %s", client_ip ? html_anchor(client_ip) : "unavailable");
	debug_message("client broswer      : %s", client);
	if (CHECK_IDENT)
	{
		system_message("ident               : %s@%s", ident, html_anchor(client_ip));
		system_message("ident forwarding    : %s", FORWARD_IDENT ? "on" : "off");
	}
	else
		system_message("ident checking      : off");
	system_message("terminal emulation  : %s", TERMINAL_TYPE);
	debug_message("receive buffer size : %d bytes", BUFFER_SIZE);
	if (connected)
	{
		debug_message("received packets    : %d", net_packets);
		debug_message("average packet size : %d bytes", net_average);
		debug_message("server sends IAC GA : %s", iacga ? "yes" : "no");
		system_message("connected to server : %s:%d", html_anchor(hostname), port);
		if (talkername[0])
			system_message("talker name         : %s", talkername);
		temp = asctime(localtime(&connection_time));
		trim_cr(temp);
		system_message("connected since     : %s", temp);
	}
	else
		system_message("connection          : not connected");
	temp = asctime(localtime(&session_time));
	trim_cr(temp);
	debug_message("session started     : %s", temp);
	system_message(HR);
}


/*****************************************************************************
	function:	command_telnet
	purpose:	connects to a host on port 23
	returns:	nothing
*****************************************************************************/
void command_telnet(char *command, char *params)
{
	int check;
	check = sscanf(params, "%s %d", hostname, &port);
	if (check == 1)
		port = 23;
	if (check > 0)
		connect_talker();
	else
		system_message("usage  : %c%s &lt;hostname&gt; [port]", CONSOLE_PREFIX, command);
}


/*****************************************************************************
	function:	command_version
	purpose:	displays version and build information
	returns:	nothing
*****************************************************************************/
void command_version(char *command, char *params)
{
	time_t	bt;
	char	*temp;

	bt = filetime("output.cgi");
	temp = ctime(&bt);
	trim_cr(temp);
	system_message("version: webspod %s built %s", VERSION_MSG, temp);
	temp = getenv("SERVER_SOFTWARE");
	if (temp)
		debug_message("version: running on %s", temp);
}


/*****************************************************************************
	function:	command_world
	purpose:	connects to a new world by name or address
	returns:	nothing
*****************************************************************************/
void command_world(char *command, char *params)
{
	if (sscanf(params, "%s %d", hostname, &port) == 2)
		connect_talker();
	else if (!connect_talker_by_name(params))
		system_message("usage  : %c%s &lt;talker name&gt; or %c%s &lt;hostname&gt; &lt;port&gt;", CONSOLE_PREFIX, command, CONSOLE_PREFIX, command);
}


/*****************************************************************************
	function:	irc_helper
	purpose:	interprets console commands as IRC commands ONLY IF IRC
				emulation is turned on
	returns:	1 on command match, 0 on no match
*****************************************************************************/
int irc_helper(char *command, char *params)
{
	if (!ircemu)
		return 0;

	if (!strcmp(command, "ignore"))
	{
		if (params)
			to_talker("ignore %s\n", params);
		else
			system_message("IRCemu : format: %c%s &lt;username&gt;", CONSOLE_PREFIX, command);
		return 1;
	}
	if (!strcmp(command, "join"))
	{
		if (params)
			to_talker("join %s\n", params + 1);
		else
			system_message("IRCemu : format: %c%s #<channel>", CONSOLE_PREFIX, command);
		return 1;
	}
	if (!strcmp(command, "list"))
	{
		system_message("IRCemu : no simple translation for '%c%s'", CONSOLE_PREFIX, command);
		system_message("IRCemu : talkers don't have a standard multi-channel format");
		return 1;
	}
	if (!strcmp(command, "me"))
	{
		if (params)
			to_talker("emote %s\n", params);
		else
			system_message("IRCemu : format: %c%s &lt;action&gt;", CONSOLE_PREFIX, command);
		return 1;
	}
	if (!strcmp(command, "motd"))
	{
		to_talker("motd\n");
		return 1;
	}
	if (!strcmp(command, "m") || !strcmp(command, "msg") || !strcasecmp(command, "query"))
	{
		if (strchr(params, ' '))
			to_talker("tell %s\n", params);
		else
			system_message("IRCemu : format: %c%s &lt;username&gt; &lt;message&gt;", CONSOLE_PREFIX, command);
		return 1;
	}
	if (!strcmp(command, "names"))
	{
		to_talker("swho\n");
		return 1;
	}
	if ((!strcmp(command, "news") || !strcmp(command, "mail")) && (strstr(params, "post")
		|| strstr(params, "reply") || strstr(params, "forward") || strstr(params, "follow")
		|| !params))
	{
		system_message("IRCemu : WARNING: use of the %s editor while in IRC emulation mode", command);
		system_message("IRCemu : may produce unexpected results. It is recommended you use");
		system_message("IRCemu : \"/set ircemu off\" and  \"/set ircemu on\" to get round this.");
		debug_message("IRCemu : entering ewe editor mode");
		ewe_editor = 1;
		to_talker("%s %s\n", command, params);
		return 1;
	}
	if (!strcmp(command, "nick"))
	{
		system_message("IRCemu : no simple translation for '%cnick'", CONSOLE_PREFIX);
		system_message("IRCemu : talkers don't have a standard name-changing format");
		return 1;
	}
	if (!strcmp(command, "notice"))
	{
		if (strchr(params, ' '))
			to_talker("remote %s\n", params);
		else
			system_message("IRCemu : format: %c%s &lt;username&gt; &lt;action&gt;", CONSOLE_PREFIX, command);
		return 1;
	}
	if (!strcmp(command, "part"))
	{
		to_talker("zc\n");
		return 1;
	}
	if (!strcmp(command, "quit"))
	{
		to_talker("quit\n");
		return 1;
	}
	if (!strcmp(command, "say"))
	{
		if (params)
			to_talker("say %s\n", params);
		else
			system_message("IRCemu : format: %c%s &lt;message&gt;", CONSOLE_PREFIX, command);
		return 1;
	}
	if (!strcmp(command, "time"))
	{
		to_talker("time\n");
		return 1;
	}
	if (!strcmp(command, "topic"))
	{
		to_talker("session %s\n", params);
		return 1;
	}
	if (!strcmp(command, "whois"))
	{
		if (params)
			to_talker("examine %s\n", params);
		else
			system_message("IRCemu : format: %c%s &lt;username&gt;", CONSOLE_PREFIX, command);
		return 1;
	}
	if (!strcmp(command, "whowas"))
	{
		if (params)
			to_talker("finger %s\n", params);
		else
			system_message("IRCemu : format: %c%s &lt;username&gt;", CONSOLE_PREFIX, command);
		return 1;
	}

	return 0;
}


/*****************************************************************************
	function:	meta_handler
	purpose:	handles all the other signals that might get thrown our way :)
	returns:	nothing
*****************************************************************************/
void meta_handler(int sig)
{
	signal(sig, SIG_IGN);
	switch(sig)
	{
		case SIGALRM:	missed_timeouts++;
						if (missed_timeouts > MAX_MISSED_TIMEOUTS)
							fatal_error("connection from browser pinger frame lost");
						send_telopt(NOP, NOP);
						to_browser(HTTP_KEEP_ALIVE);
/* refresh the pinger frame just in case that's where the problem lies */
						printf("<script type=\"text/javascript\">top.pinger.location='pinger.cgi?pid=%d'</script>", pid);
						alarm(KEEP_ALIVE_INTERVAL);
						break;
		case SIGHUP:	system_message("console: you have been disconnected.");
						system_message("console: the system is probably going down for reboot/upgrade NOW");
						to_browser("<hr/>\n");
						if (sockfd >= 0)
							shutdown(sockfd, 2);
						disable_input_window();
						bell();
						printf("%s</font></pre>\n</body>\n</html>\n", SCROLL_SCRIPT);
						fflush(stdout);
						delete_pidfile();
						exit(0);
		case SIGPING:	missed_timeouts = 0;
						break;
		case SIGPIPE:	system_message("network: connection to talker lost (broken pipe)");
						disconnect_talker();
						break;
		case SIGQUIT:
		case SIGSEGV:
		case SIGTERM:
		case SIGXCPU:
		case SIGXFSZ:	fatal_error("%s", strsignal(sig));
		default:		debug_message("signal : &lt;-- %s", strsignal(sig));
	}
	signal(sig, meta_handler);
}


/*****************************************************************************
	function:	input_handler
	purpose:	called when there is a command waiting. Interprets and runs
				console command or sends the command to the talker, as
				appropriate.
	returns:	nothing
*****************************************************************************/
void input_handler(int sig)
{
	static int	started = 0;
	char		filename[MAX_VARLEN], line[MAX_VARLEN], *temp, command[MAX_VARLEN], *params;
	int			i, state = 0;
	FILE		*fp;

	signal(SIGSUBMIT, SIG_IGN);
	missed_timeouts = 0;
	alarm(KEEP_ALIVE_INTERVAL);

/* open and read from the command transfer file */
	debug_message("signal : &lt;-- SIGSUBMIT");
	sprintf(filename, "%s%d", TEMP_PATH, pid);
	fp = fopen(filename, "r");
	if (!fp)
	{
		debug_message("error : couldn't open '%s'", filename);
		if (USE_SYSLOG)
			syslog(LOG_ERR, "received input signal but couldn't open %s", filename);
		signal(SIGSUBMIT, input_handler);
		return;
	}

	if (!fgets(line, MAX_VARLEN, fp))
	{
		fclose(fp);
		unlink(filename);
		debug_message("error : couldn't read from '%s'", filename);
		if (USE_SYSLOG)
			syslog(LOG_ERR, "received input signal but couldn't read from %s", filename);
		signal(SIGSUBMIT, input_handler);
		return;
	}
	fclose(fp);

/* trim the input line */
	temp = &(line[0]);
	while (*temp == ' ')
		temp++;

	trim_cr(line);

/* intercept console or IRC emulated commands */
	if (*temp == CONSOLE_PREFIX)
	{
		temp++;
		params = strchr(temp, ' ');
		if (params)
			*params = 0;
		strcpy(command, temp);
		if (params)
		{
			*params = ' ';
			while (*params == ' ')
				params++;
		}

/* force the copy of the command to lowercase */
		temp = &(command[0]);
		while (*temp)
			*temp++ = tolower(*temp);

		
/* loop through console commands looking for a match */
		for (i = state = 0; command_list[i].command; i++)
			if (!strcmp(command_list[i].command, command))
			{
				(command_list[i].function)(command, params);
				state = 1;
				break;
			}
			else if (command_list[i].alias)
				if (!strcmp(command_list[i].alias, command))
				{
					(command_list[i].function)(command, params);
					state = 1;
					break;
				}


/* IRC-command matching */
		if (!state)
		{
			state = irc_helper(command, params);
			if (state)
				local_echo("&lt;IRC-emulated command&gt;");
		}

		if (!state)
			system_message("console: command not recognised: %c%s", CONSOLE_PREFIX, command);
	}
	else	/* non-console command - pass it to the talker */
	{
		if (ircemu && line[1] != CONSOLE_PREFIX && !passwordmode > 1 && !ewe_editor)
			to_talker("say %s\n", line);
		else
			to_talker("%s\n", line);

		local_echo(line);
		
		if (!passwordmode)
			strcpy(lastline, line);

		if (ircemu && ewe_editor && (!strcasecmp(line, "end") || !strcasecmp(line, "quit")
			|| !strcasecmp(line, ".end") || !strcasecmp(line, ".quit")))
		{
			ewe_editor = 0;
			debug_message("IRCemu : leaving ewe editor mode");
		}
		input_counter++;
	}

/* grab the username - it's the first thing sent to each talker, right? :) */
	if (input_counter == 1)
	{
		strcpy(username, line);
		if (USE_SYSLOG)
		{
			if (talkername[0])
				syslog(LOG_INFO, "%s (%s@%s) connected to %s (%s:%d)",
						username, ident, client_ip, talkername, hostname, port);
			else
				syslog(LOG_INFO, "%s (%s@%s) connected to %s:%d",
						username, ident, client_ip, hostname, port);
		}
		update_pidfile();
	}

	unlink(filename);
	signal(SIGSUBMIT, input_handler);
}


/*****************************************************************************
	function:	refresh_theme
	purpose:	refreshes all the frames with a new stylesheet ONLY IF that
				theme exists.
	returns:	1 on success, 0 on failure
*****************************************************************************/
int refresh_theme(char *new_theme)
{
	static int	started = 0;
	FILE		*fp;
	char		filename[MAX_VARLEN];

	if (!strcasecmp(theme, new_theme) && started && !debugging)
	{
		system_message("theme '%s' is already loaded.", theme);
		return 0;
	}

	started = 1;

/* check stylesheet for theme actually exists */
	sprintf(filename, "%s%s/theme.css", THEMES_PATH, new_theme);
	fp = fopen(filename, "r");
	if (!fp)
	{
		debug_message("themes : file not found, '%s'", filename);
		system_message("themes : couldn't load '%s' theme", new_theme);
		system_message("themes : current theme remains '%s'", theme);
		return 0;
	}
	fclose(fp);
	strcpy(theme, new_theme);

	system_message("themes : loading '%s'", theme);

	printf("<link rel=\"stylesheet\" href=\"%s\">", filename);
	printf("<script type=\"text/javascript\">\n<!--\n");
	printf("top.commandprompt.location='input.cgi?pid=%d&theme=%s&passwordmode=%d'\n", pid, theme, passwordmode);
	printf("top.topleft.location='frame.php?position=topleft&theme=%s'\n", theme);
	printf("top.topbit.location='frame.php?position=top&theme=%s&hostname=%s&port=%d'\n", theme, hostname, port);
	printf("top.topright.location='frame.php?position=topright&theme=%s'\n", theme);
	printf("top.leftside.location='frame.php?position=leftside&theme=%s'\n", theme);
	printf("top.rightside.location='frame.php?position=rightside&theme=%s'\n", theme);
	printf("top.bottomleft.location='frame.php?position=bottomleft&theme=%s'\n", theme);
	printf("top.bottom.location='frame.php?position=bottom&theme=%s'\n", theme);
	printf("top.bottomright.location='frame.php?position=bottomright&theme=%s'\n", theme);
	printf("// -->\n</script>");

/* check javascript for theme actually exists */
	if (ENABLE_THEME_SCRIPTS)
	{
		sprintf(filename, "%s%s/theme.js", THEMES_PATH, new_theme);
		fp = fopen(filename, "r");
		if (fp)
		{
			debug_message("themes : loading startup script '%s'", filename);
			printf("<script type=\"text/javascript\" src=\"%s\"></script>", filename);
			fclose(fp);
		}
		else
			debug_message("themes : no startup script avaliable");
	}

	fflush(stdout);
	return 1;
}


/*****************************************************************************
	function:	enable_input_window
	purpose:	loads the 'commandprompt' frame with input.cgi and sets the
				global 'prompt_enabled'
	returns:	nothing
*****************************************************************************/
void enable_input_window()
{
	if (prompt_enabled)
		return;

	system_message("console: enabling command prompt");
	printf("<script type=\"text/javascript\">\n<!--\n");
	printf("top.commandprompt.location='input.cgi?pid=%d&theme=%s';\n", pid, theme);
	printf("top.pinger.location='pinger.cgi?pid=%d';\n", pid);
	printf("\n// -->\n</script>");
	prompt_enabled = 1;
	signal(SIGSUBMIT, input_handler);
}


/*****************************************************************************
	function:	disable_input_window
	purpose:	loads the 'commandprompt' frame with a placeholder and resets
				the global 'prompt_enabled'
	returns:	nothing
*****************************************************************************/
void disable_input_window()
{
	if (!prompt_enabled)
		return;

	signal(SIGSUBMIT, SIG_IGN);
	system_message("console: disabling command prompt");
	system_message("console: refresh your browser or click <a target=\"_top\" href=\"index.php\">here</a> to return to the connection page.");
	debug_message("console: disabling pinger frame");
	printf("<script type=\"text/javascript\">\n<!--\n");
	printf("top.commandprompt.location='loading.php?theme=%s'\n;", theme);
	printf("top.pinger.location='loading.php'\n;");
	printf("// -->\n</script>");
	prompt_enabled = 1;
}


/*****************************************************************************
	function:	set_password_mode
	purpose:	refreshes the command prompt frame to reflect password status
	returns:	nothing
*****************************************************************************/
void set_password_mode(int mode)
{
	if (passwordmode == mode)
		return;

	passwordmode = mode;
	if (passwordmode)
		debug_message("-- entering password-blanking mode");
	else
		debug_message("-- leaving password-blanking mode");
	printf("<script type=\"text/javascript\">top.commandprompt.location='input.cgi?pid=%d&theme=%s&passwordmode=%d'</script>", pid, theme, passwordmode);
	fflush(stdout);
}


/*****************************************************************************
	function:	send_telopt
	purpose:	sends a 3-byte telopt command to the talker
	returns:	nothing
*****************************************************************************/
void send_telopt(unsigned char cmd, unsigned char opt)
{
	char data[3];

	if (!connected)
		return;

	data[0] = IAC;
	data[1] = cmd;
	data[2] = opt;

	send(sockfd, data, (opt == NOP) ? 2 : 3, 0);
	if (debugging)
	{
		if (opt == NOP)
			debug_message("telopt : --&gt; IAC %s", _TELCMD(cmd));
		else
			debug_message("telopt : --&gt; IAC %s %s", _TELCMD(cmd), _TELOPT(opt));
	}
}


/*****************************************************************************
	function:	retitle
	purpose:	changes the browser's window title and the text in the top
				frame to the specifiec formatted text.
	returns:	nothing
*****************************************************************************/
void retitle(char *format, ...)
{
	char buffer[MAX_VARLEN];
	va_list arguments;

	va_start(arguments, format);
	vsprintf(buffer, format, arguments);
	va_end(arguments);
	printf("<script type=\"text/javascript\">\n<!--\ntop.document.title='%s'\ntop.topbit.location='frame.php?position=top&world=%s&hostname=%s&port=%d&theme=%s'\n// -->\n</script>",
			buffer, talkername, hostname, port, theme);
}


/*****************************************************************************
	function:	handle_telopt
	purpose:	extracts and acts on any telopt command at the start of the
				buffer.
	returns:	number of characters used by received telopt command, if any
*****************************************************************************/
int handle_telopt(unsigned char *buffer, char *orig)
{
	char			*temp, outbuffer[MAX_VARLEN];
	unsigned char	cmd, opt;
	int				i;

	if (buffer[0] != IAC)
		return 0;

	cmd = buffer[1];
	opt = buffer[2];

	switch(cmd)
	{
		case IAC:	return 1;
		case AYT:	debug_message("telopt : &lt;-- IAC AYT");
					send_telopt(NOP, NOP);
					break;
		case NOP:	debug_message("telopt : &lt;-- IAC NOP");
					return 2;
		case GA:	for (temp = buffer - 1; *temp != '\n' && temp >= orig; temp--)
						*temp = ' ';
					debug_message("telopt : &lt;-- IAC GA");
					iacga = 1;
					return 2;
		case WILL:	debug_message("telopt : &lt;-- IAC WILL %s", _TELOPT(opt));
					if (opt == TELOPT_ECHO || opt == TELOPT_EOR || opt == TELOPT_SGA)
						send_telopt(DONT, opt);
					if (opt == TELOPT_ECHO)
						set_password_mode(1);
					return 3;
		case WONT:	debug_message("telopt : &lt;-- IAC WONT %s", _TELOPT(opt));
					send_telopt(DONT, opt);
					if (opt == TELOPT_ECHO)
						set_password_mode(0);
					return 3;
		case DO:	debug_message("telopt : &lt;-- IAC DO %s", _TELOPT(opt));
					if (opt == TELOPT_TTYPE || opt == TELOPT_STATUS)
						send_telopt(WILL, opt);
					else
						send_telopt(WONT, opt);
					return 3;
		case DONT:	debug_message("telopt : &lt;-- IAC DONT %s", _TELOPT(opt));
					send_telopt(WONT, opt);
					return 3;
		case SB:	if (opt == TELOPT_TTYPE && buffer[3] == TELQUAL_SEND && buffer[4] == IAC && buffer[5] == SE)
					{
						debug_message("telopt : &lt;-- IAC SB TERMINAL-TYPE SEND IAC SE");
						debug_message("telopt : --&gt; IAC SB TERMINAL-TYPE IS NVT('%s') IAC SE", TERMINAL_TYPE);
						debug_message("telopt : TERMINAL TYPE negotiation is disabled");
/*
						sprintf(outbuffer, "%c%c%c%c%s@%c%c", IAC, SB, TELOPT_TTYPE, TELQUAL_IS, TERMINAL_TYPE, IAC, SE);
						i = strlen(outbuffer);
						temp = strchr(outbuffer, '@');
						*temp = 0;
						send(sockfd, outbuffer, i, 0);
*/
						return 6;
					}
					if (opt == TELOPT_STATUS && buffer[3] == TELQUAL_SEND && buffer[4] == IAC && buffer[5] == SE)
					{
						debug_message("telopt : &lt;-- IAC SB STATUS SEND IAC SE");
						debug_message("telopt : --&gt; IAC SB STATUS IS");
						debug_message("telopt : --&gt; WILL ECHO DONT SUPPRESS-GO-AHEAD WILL STATUS DO STATUS");
						debug_message("telopt : --&gt; IAC SE");
						to_talker("%c%c%c%c%c%c%c%c%c%c%c%c%c%c", IAC, SB, TELOPT_STATUS, TELQUAL_IS,
									WILL, TELOPT_ECHO, DONT, TELOPT_SGA, WILL, TELOPT_STATUS, DO,
									TELOPT_STATUS, IAC, SE);
						return 6;
					}
					for (i = 2; buffer[i] && buffer[i] != SE; i++);
					system_message("telopt : &lt;-- unhandled %d-byte subnegotiation", i);
					return i;
		case SE:	debug_message("telopt : &lt;-- IAC SE without IAC SB!");
					return 2;
		default:	if (debugging)
					{
						if (TELCMD_OK(cmd))
						{
							if (_TELOPT_OK(opt))
								debug_message("telopt : &lt;-- unhandled telopt code: IAC %s %s", _TELCMD(cmd), _TELOPT(opt));
							else
								debug_message("telopt : &lt;-- unknown telopt option: IAC %s %d", _TELCMD(cmd), opt);
						}
						else
							system_message("telopt : &lt;-- unknown telopt code: IAC %s", _TELCMD(cmd));
					}
					return 2;
	}
}


/*****************************************************************************
	function:	replace_colours
	purpose:	extracts and acts on any ANSI colour commands at the start of
				the buffer.
	returns:	number of characters used by received colour command, if any
*****************************************************************************/
int replace_colours(char *buffer)
{
	int	i;

	if (buffer[0] != '\033')
		return 0;

	while (!strchr(buffer, 'm') && !strchr(buffer, 'm') && strlen(buffer) < 8)
		if (!overrun_buffer(buffer))
			break;

/* NOR cases */
	if (!strncmp(buffer, NOR, strlen(NOR)))
	{
		printf("</font><font>");
		return strlen(NOR);
	}
	if (!strncmp(buffer, NOR2, strlen(NOR2)))
	{
		printf("</font><font>");
		return strlen(NOR2);
	}

/* convert ANSI foreground colours to HTML colour codes */
	for (i = 0; ansi_fg_colours[i]; i++)
		if (!strncmp(buffer, ansi_fg_colours[i], strlen(ansi_fg_colours[i])))
		{
			if (colours)
				printf("</font><font style=\"color: %s\">", html_colours[i]);
			return(strlen(ansi_fg_colours[i]));
		}

/* and the same for background colours ... */
	for (i = 0; ansi_bg_colours[i]; i++)
		if (!strncmp(buffer, ansi_bg_colours[i], strlen(ansi_bg_colours[i])))
		{
			if (colours)
				printf("</font><font style=\"background-color: %s\">", html_colours[i]);
			return(strlen(ansi_bg_colours[i]));
		}

/* and the same for other effects ... */
	for (i = 0; ansi_other[i]; i++)
		if (!strncmp(buffer, ansi_other[i], strlen(ansi_other[i])))
		{
			if (colours)
				printf("</font><font style=\"%s\">", html_other[i]);
			return(strlen(ansi_other[i]));
		}

/* muffle unknown -m codes */
	for (i = 1; i < strlen(buffer); i++)
		if (buffer[i] == 'm' || buffer[i] == 'H')
		{
			if (debugging)
			{
				if (buffer[i] == 'H')
					debug_message("ansi   : --&gt; cursor movement command (not supported)");
				else
				{
					buffer[++i] = 0;
					debug_message("ansi   : --&gt; unknown escape sequence: 'ESC%s'", buffer + 1);
				}
			}
			return i;
		}

/* muffle just the escape for completely unknown escape sequences */
	debug_message("ansi   : --&gt; unknown escape sequence. buffer='ESC%s'", buffer + 1);
	return 1;
}


/*****************************************************************************
	function:	replace_swearing
	purpose:	asterisks-out any swearing ONLY IF swearfilter is on.
	returns:	nothing
*****************************************************************************/
void replace_swearing(char *buffer)
{
	int		i;
	char	*pos, *pos2, *tempbuffer;

	if (!swearfilter)
		return;

/* tempbuffer is a lowercase copy of buffer for matching without stristr() */
	tempbuffer = malloc(strlen(buffer));
	if (!tempbuffer)
	{
		debug_message("malloc : couldn't allocate %d bytes in replace_swearing()", strlen(buffer));
		return;
	}

	pos = buffer;
	pos2 = tempbuffer;

	while (*pos)
		*(pos2++) = tolower(*(pos++));

	for (i = 0; banned_words[i]; i++)
		do
		{
			pos = strstr(tempbuffer, banned_words[i]);
			if (pos)
			{
				memset(buffer + (pos - tempbuffer), '*', strlen(banned_words[i]));
				memset(pos, '*', strlen(banned_words[i]));
			}
		}
		while (pos);

	free(tempbuffer);
}


/*****************************************************************************
	function:	replace_urls
	purpose:	extracts and converts-to-HTML any URL or email address at the
				start of the buffer.
	returns:	number of characters used by extracted URL, if any
*****************************************************************************/
int replace_URLs(char *buffer)
{
	int			i;
	static char	temp[MAX_VARLEN];

	if (!HIGHLIGHT_LINKS)
		return 0;

	if (!strncmp(buffer, "http://", 7) || !strncmp(buffer, "www.", 4))
	{
		for (i = 4; buffer[i] && i < (MAX_VARLEN - 7) && !strchr(" \n\t\r,()!\"'<>", buffer[i]); i++);
		strncpy(temp, buffer, i);
		temp[i] = 0;
		to_browser(html_anchor(temp));
		return i;
	}
	return 0;
}


/*****************************************************************************
	function:	replace_username
	purpose:	extracts and highlights any mention of the current username
				at the start of the buffer ONLY IF hinames are turned on.
	returns:	number of characters used by the username, if found
*****************************************************************************/
int replace_username(char *buffer)
{
	int			i;
	static char	temp[MAX_VARLEN];

	if (hinames)
		if (!strncasecmp(buffer, username, strlen(username)))
		{
			printf("<font class=\"hinames\">%s</font>", username);
			return(strlen(username));
		}
	return 0;
}


/*****************************************************************************
	function:	replace_smilies
	purpose:	extracts and converts to graphical icons any smiley at the
				start of the buffer ONLY IF smilies are turned on.
	returns:	number of characters used by ASCII smiley, if any
*****************************************************************************/
int replace_smilies(char *buffer)
{
	int i;

/* this prevents spurious smilies in logon screen ASCII artwork */
	if (!smilies || !username[0])
		return 0;

	for (i = 0; ascii_smilies[i]; i++)
		if (!strncmp(buffer, ascii_smilies[i], strlen(ascii_smilies[i])))
		{
			to_browser("<img src=\"images/%s.gif\" alt=\"%s\">", graphical_smilies[i], ascii_smilies[i]);
			return(strlen(ascii_smilies[i]));
		}
	return 0;
}


/*****************************************************************************
	function:	overrun_buffer
	purpose:	subroutine called by the replace_xxx functions. Receives an
				extra character from the talker and appends it to the
				specified buffer
	returns:	number of characters (0 or 1) added to the buffer
*****************************************************************************/
int overrun_buffer(char *buffer)
{
	fd_set			rfds;
	struct timeval	tv;
	int				i;

	if (!connected)
		return 0;

/* Watch socket to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

/* Wait up to two seconds. */
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	if (select(1, &rfds, NULL, NULL, &tv))
	{
		buffer = strchr(buffer, '\0');
		i = recv(sockfd, buffer, 1, 0);
		if (i)
			*(++buffer) = 0;
		return i;
	}
	return 0;
}


/*****************************************************************************
	function:	main
	purpose:	everything else
	returns:	does not return
*****************************************************************************/
int main(void)
{
	char	buffer[BUFFER_SIZE];
	int		i, j, temp;

	if (USE_SYSLOG)
		openlog("webspod", LOG_PID, LOG_USER);

	session_time = time(0);
	pid = getpid();

	printf("Content-type: text/html\n\n");
	printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	printf("<html>\n<head>\n<title>output frame</title>\n");
	printf("<script text=\"\">\n<!--\nfunction changefocus() {\ntop.commandprompt.inputform.cmd.focus()\n}\ndocument.onkeydown=changefocus\n// -->\n</script>");
	printf("</head>\n<body class=\"main\" onbeforeunload=\"var quit = new Image(); ");
	printf("quit.src = 'input.cgi?pid=%d&cmd=emote+quit+-+disconnected'\">", pid);
/* leave the blank <font> tag - it's for easy colour manipulation */
	printf("<pre style=\"font-face: courier\"><font>\n");

	get_client_info();
	update_pidfile();
	get_form_vars();

/* run this a second time to print out debugging info if required */
	if (debugging)
		get_client_info();

	system_message("Welcome to webspod version %s", VERSION_MSG);
	system_message(COPYRIGHT_MSG);
	system_message("This service comes with ABSOLUTELY NO WARRANTY.");
	printf("<hr/>");
	
	debug_message("debugging mode is on");

	if (!getuid())
		fatal_error("do not run webspod as root");

	if (theme[0])
	{
		if (!refresh_theme(theme))
			refresh_theme(DEFAULT_THEME);
	}
	else
		refresh_theme(DEFAULT_THEME);

	get_ident();

	if (!hostname || !port)
		debug_message("no startup data.");
	else
		connect_talker();

	fixup();

	debug_message("session: smilies: %s, timestamps: %s, localecho: %s, irc emulation: %s", 
					smilies ? "<b>yes</b>" : "no", timestamps ? "<b>yes</b>" : "no", localecho ? "<b>yes</b>" : "no",
					ircemu ? "<b>yes</b>" : "no");
	debug_message("session: hinames: %s, colours: %s", hinames ? "<b>yes</b>" : "no", colours ? "<b>yes</b>" : "no");
	system_message("console: prefix character is '%c' - use '%chelp' for more info", CONSOLE_PREFIX, CONSOLE_PREFIX);
	if (ircemu)
	{
		system_message("ircemu : currently in IRC command emulation mode");
		system_message("ircemu : to revert to normal talker command mode, type '%cset ircemu off'", CONSOLE_PREFIX);
	}

	enable_input_window();

/* set up signal handlers */
	if (signal(SIGPING, meta_handler) == SIG_ERR ||
		signal(SIGALRM, meta_handler) == SIG_ERR ||
		signal(SIGHUP, meta_handler) == SIG_ERR ||
		signal(SIGPIPE, meta_handler) == SIG_ERR ||
		signal(SIGQUIT, meta_handler) == SIG_ERR ||
		signal(SIGTERM, meta_handler) == SIG_ERR ||
		signal(SIGIO, meta_handler) == SIG_ERR ||
		signal(SIGPOLL, meta_handler) == SIG_ERR ||
		signal(SIGSEGV, meta_handler) == SIG_ERR ||
		signal(SIGXCPU, meta_handler) == SIG_ERR ||
		signal(SIGXFSZ, meta_handler) == SIG_ERR)
		fatal_error("unable to set up signal handlers");

	alarm(KEEP_ALIVE_INTERVAL);

/* main loop */
	while(1)
	{
		if (!connected)
			pause();
		else 
		{
		  temp = 1;
		  while (temp > 0) { 
			while((temp = recv(sockfd, buffer, BUFFER_SIZE - 20, 0)) > 0)
			{
				net_bytes += temp;
				net_packets++;
				net_average = net_bytes / net_packets;

				buffer[temp] = 0;

				replace_swearing(buffer);

				for (i = 0; i < strlen(buffer); i++)
				{
					do
					{
						j = i;	
						do
						{
							temp = handle_telopt((char *) &(buffer[i]), buffer);
							i += temp;
						}
						while (temp);
		
						do
						{
							temp = replace_colours((char *) &(buffer[i]));
							i += temp;
						}
						while (temp);

						do
						{
							temp = replace_username((char *) &(buffer[i]));
							i += temp;
							if (temp)
								newline = 0;
						}
						while (temp);

						do
						{
							temp = replace_URLs((char *) &(buffer[i]));
							i += temp;
							if (temp)
								newline = 0;
						}
						while (temp);

						do
						{
							temp = replace_smilies((char *) &(buffer[i]));
							i += temp;
							if (temp)
								newline = 0;
						}
						while (temp);
					}
					while (j < i);

					switch(buffer[i])
					{
						case '<':	printf("&lt;");
									newline = 0;
									break;
						case '>':	printf("&gt;");
									newline = 0;
									break;
						case '\07':	bell();
									break;
						case '\n':	if (!newline)
									{
										putchar(buffer[i]);
										newline = 1;
									}
									printf(SCROLL_SCRIPT);
									break;
						default:	if (buffer[i] >= ' ')
									{
										if (newline)
											to_browser(get_timestamp());
										putchar(buffer[i]);
									}
									newline = 0;
					}
				}
				fflush(stdout);
			}
			if (((temp == -1) && (errno != EINTR)) ||
			    (temp == 0)) {
			  system_message("connection to talker lost");
			  //			  system_message("temp : %d", temp);
			  //			  system_message("errno : %d", errno);
			  disconnect_talker();
			} else {
			  temp = 1;
			  //			  system_message("tempooo : %d", temp);
			}
		  }
		}
	}
}

