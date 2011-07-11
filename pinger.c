/*	
	Webspod - a browser interface for talkers.

	File: pinger.c
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

/* local prototypes */
void dienice();


/*****************************************************************************
	function:	dienice
	purpose:	prints a W3C-compliant page complaining of a fatal error and
				exits.
	returns:	nothing. Exits with -1 regardless
*****************************************************************************/
void dienice()
{
	printf("<html>\n<head>\n<title>fatal error</title>\n</head>\n<body>\n");
	printf("FATAL ERROR: couldn't extract running PID!\n");
	printf("</body>\n</html>\n");
	exit(-1);
}


/*****************************************************************************
	function:	main
	purpose:	main routine
	returns:	0 - regardless
*****************************************************************************/
int main(void)
{
	static char		temp[7];
	char			*pos, *end;
	int				length;
	unsigned		pid;


	printf("Content-type: text/html\n\n");
	printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");

	pos = (char *) strstr(getenv("QUERY_STRING"), "pid=");
	if (!pos)
		dienice();

	pos += 4;
	length = 0;
	for (end = pos; *end != 0 && *end != '&'; end++)
		length++;
	bzero(temp, 6);
	if (length > 6)
		length = 6;
	strncpy(temp, pos, length);
	pid = atoi(temp);
	if (!pid)
		dienice();

	printf("<html>\n<head>\n");
	printf("<meta http-equiv=\"refresh\" content=\"%d;url=pinger.cgi?pid=%d&dummy=\" />\n", KEEP_ALIVE_INTERVAL, pid, time(0));
	printf("</head>\n</html>\n");
	kill(pid, SIGUSR2);
	exit(0);
}	
