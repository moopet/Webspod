/*	
	Webspod - a browser interface for talkers.

	File: wsstatus.c
	Copyright 2003 Ben Sinclair (ben@moopet.net)

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your actionion) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "webspod.h"

#define INACTIVE		"<inactive>"

#define QUIET			0
#define VERBOSE			1

#define LIST_ALL		0
#define LIST_USER		1
#define KICK_ALL		2
#define KICK_USER		3
#define KICK_INACTIVE	4

/* global variables */
char	*progname, *progpath;

/* local prototypes */
int		trim_cr(char *);
void	usage(void);
void	version(void);


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
	function:	usage
	purpose:	displays help/usage information and exits
	returns:	nothing
*****************************************************************************/
void usage()
{
	printf("usage: %s [-hkKqv] [username]\n", progname);
	printf("-c, --clear                  drop all INACTIVE connections.\n");
	printf("-h, --help                   displays this page.\n");
	printf("-k, --kick username          drop all connections for 'username'.\n");
	printf("-K, --kickall                drop ALL connections.\n");
	printf("-q, --quiet                  quiet mode.\n");
	printf("-v, --version                show version information.\n");
	exit(0);
}


/*****************************************************************************
	function:	version
	purpose:	displays version number and build time and exits
	returns:	nothing
*****************************************************************************/
void version()
{
	time_t			bt = 0;
	struct stat		fs;

	if (stat(progpath, &fs) >= 0)
		bt = fs.st_mtime;

	if (bt)
		printf("%s for webspod version %s built %s\n", progname, VERSION_MSG, ctime(&bt));
	else
		printf("%s for webspod version %s\n", progname, VERSION_MSG);
	exit(0);
}


/*****************************************************************************
	function:	main
	purpose:	main routine
	returns:	number of connections affected / listed
*****************************************************************************/
int main(int argc, char *argv[])
{
	FILE			*fp;
	DIR				*tempdir;
	struct dirent	*entry;
	int				i = 0, temppid, action = LIST_ALL, mode = VERBOSE;
	char			path[MAX_VARLEN], filename[MAX_VARLEN], temp[MAX_VARLEN], temp2[MAX_VARLEN];
	char			*s, *t, *username, *searchname = 0;
	time_t			tt = 0;
	struct stat		fs;
	struct tm		*ft;

	progname = progpath = argv[0];
	if (strrchr(progname, '/'))
		progname = strrchr(progname, '/') + 1;

	strcpy(path, TEMP_PATH);
	s = strrchr(path, '/');
	if (s)
		if (s - strlen(path) + 1 < (char *) path)
			*s++ = 0;

	tempdir = opendir(path);
	
	for (i = 1; i < argc; i++)
	{
		if (*argv[i] != '-')
		{
			searchname = argv[i];
			if (action == LIST_ALL)
				action = LIST_USER;
		}
		else if (!strcmp(argv[i], "-c") || !strcasecmp(argv[i], "--clear"))
			action = KICK_INACTIVE;
		else if (!strcmp(argv[i], "-v") || !strcasecmp(argv[i], "--version"))
			version();
		else if (!strcmp(argv[i], "-h") || !strcasecmp(argv[i], "--help"))
			usage();
		else if (!strcmp(argv[i], "-q") || !strcasecmp(argv[i], "--quiet"))
			mode = QUIET;
		else if (!strcmp(argv[i], "-k") || !strcasecmp(argv[i], "--kick"))
			action = KICK_USER;
		else if (!strcmp(argv[i], "-K") || !strcasecmp(argv[i], "--kickall"))
			action = KICK_ALL;
		else
		{
			printf("%s: unknown option '%s'.\n", progname, argv[i]);
			return(-1);
		}
	}

	if ((action == KICK_USER && !searchname) || (action == KICK_ALL && searchname))
		usage();

	if (!tempdir)
	{
		printf("%s: couldn't open temporary folder: '%s'\n", progname, path);
		return(0);
	}

	i = temp[0] = 0;

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

/* if we're root, we can use this opportunity to delete orphanned files */
		if (!getuid())
			if (kill(temppid, SIGPING) < 0)
			{
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
			continue;

		fgets(temp2, MAX_VARLEN, fp);
		trim_cr(temp2);

		username = strrchr(temp2, ' ');
		if (username)
			username++;
		if (!strlen(username))
			username = (char *) INACTIVE;

		if (action == LIST_ALL || (action == LIST_USER && !strcasecmp(username, searchname)))
		{
			if (!i && mode == VERBOSE)
			{
				printf(" ------------------------------- webspod users --------------------------------\n\n");
				printf("%s %-16s  %-24s  %-5s  %-5s  %-19s%s\n", BOLD, "username", "hostname", "port", "since", "ident", NOR);
			}

			printf(" %-16s ", username);
			fgets(temp2, MAX_VARLEN, fp);
			trim_cr(temp2);
			printf("%-24s  ", strrchr(temp2, ' '));
			fgets(temp2, MAX_VARLEN, fp);
			trim_cr(temp2);
			printf("%-5s  ", strrchr(temp2, ' '));
			if (tt)
				printf(" %2d:%02d", ft->tm_hour, ft->tm_min);
			else
				printf(" ??:??");
			fgets(temp2, MAX_VARLEN, fp);
			trim_cr(temp2);
			printf("  %-19s\n", strrchr(temp2, ' '));
			fclose(fp);
			i++;
		}
		else if ((action == KICK_USER && !strcasecmp(username, searchname)) ||
				 (action == KICK_INACTIVE && !strcmp(username, INACTIVE))   ||
				  action == KICK_ALL)
		{
			sprintf(temp2, "%s/%s", path, filename);
			if (unlink(temp2) != 0)
			{
				printf("%s: %s.\n", progname, strerror(errno));
				exit(-1);
			}
/*
	SIGHUP makes the client exit nicely. If it doesn't work, could use SIGKILL :) */
			kill(temppid, SIGHUP);
			i++;
		}
	}
	closedir(tempdir);

	if (mode == VERBOSE)
	{
		if (i)
		{
			switch(action)
			{
			case LIST_ALL:		printf("\n ------------------------------ %02d users connected ----------------------------\n", i);
								break;
			case LIST_USER:		printf("\n ---------------------------- connected to %02d hosts ---------------------------\n", i);
								break;
			case KICK_INACTIVE:	
			case KICK_ALL:		printf("%s: dropped %d connection%s\n", progname, i, i == 1 ? "." : "s.");
								break;
			case KICK_USER:		printf("%s: kicked '%s' from %d host%s\n", progname, searchname, i, i == 1 ? "." : "s.");
			default:			break;
			}
		}
		else
			switch(action)
			{
			case KICK_ALL:
			case LIST_ALL:		printf("%s: no users connected.\n", progname);
								break;
			case KICK_USER:	
			case LIST_USER:		printf("%s: user '%s' not connected.\n", progname, searchname);
								break;
			case KICK_INACTIVE:	printf("%s: no inactive sessions connected.\n", progname);
			default:			break;
			}
	}
	
	return(i);
}

