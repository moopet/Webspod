/*	
	Webspod - a browser interface for talkers.

	File: input.c
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

/* global variables */
char		theme[MAX_VARLEN], cmd[MAX_VARLEN];
char		filename[MAX_VARLEN];
int			passwordmode, submitted;
unsigned	pid;

/* local prototypes */
void	get_form_vars(void);
char	*extract_var(char *, char *);
char	unescapeURLchar(char, char);


/*****************************************************************************
	function:	unescapeURLchar
	purpose:	subroutine to unescape a single character
	returns:	ASCII version of escaped character
*****************************************************************************/
char unescapeURLchar(char c1, char c2)
{
	register char c;
	
	c = (c1 >= 'A') ? ((c1 & 0xdf) - 'A') + 10 : (c1 - '0');
	c*= 16;
	c+= (c2 >= 'A') ? ((c2 & 0xdf) - 'A') + 10 : (c2 - '0');
	
	return c;
}


/*****************************************************************************
	function:	get_form_vars
	purpose:	Runs through the submitted browser form variables and calls
				extract_var() as needed to assign values to globals.
	notes:		currently a duplicate of the same function in output.c, with
				different variable names to extract
	returns:	nothing.
*****************************************************************************/
void get_form_vars(void)
{
	char	vars[2048], request[10], *temp;
	int		length;

	if(!getenv("REQUEST_METHOD"))
		exit(0);

	strncpy(request, getenv("REQUEST_METHOD"), 9);
	request[9] = 0;
	
	if(!strlen(request))
		exit(0);

	if(strncmp(request, "GET", 3) == 0)
	{
		strncpy(vars, getenv("QUERY_STRING"), 2048);
		vars[2047] = 0;
		if(!strlen(vars))
			exit(0);
	}
	else if(strncmp(request, "POST", 4) == 0)
	{
		if(!getenv("CONTENT_LENGTH"))
			exit(0);
		length = atoi(getenv("CONTENT_LENGTH"));
		if(!length || length == 0)
			exit(0);
		fread(vars, length > 2048 ? 2048 : length, 1, stdin);
		vars[length] = 0;
	}
	else
		exit(0);

	temp = extract_var(vars, "pid");
	if (temp)
		pid = atoi(temp);
	else
	{
		printf("<html>\n<head>\n<title>fatal error</title>\n</head>\n<body>\n");
		printf("FATAL ERROR: couldn't extract running PID!\n");
		printf("</body>\n</html>\n");
		exit(0);
	}
	strcpy(filename, TEMP_PATH);
	strcat(filename, temp);
	temp = extract_var(vars, "passwordmode");
	if (temp)
		passwordmode = atoi(temp);
	temp = extract_var(vars, "submitted");
	if (temp)
		submitted = atoi(temp);
	temp = extract_var(vars, "theme");
	if (temp)
		strncpy(theme, temp, MAX_VARLEN);
	else
		strcpy(theme, DEFAULT_THEME);
	temp = extract_var(vars, "cmd");
	if (temp)
		strncpy(cmd, temp, MAX_VARLEN);
	else
		cmd[0] = 0;
}


/*****************************************************************************
	function:	extract_var
	purpose:	subroutine called by get_form_vars() for string manipulation
	notes:		currently a duplicate of the same function in output.c
	returns:	pointer to string value of specified form variable, or null.
*****************************************************************************/
char *extract_var(char *vars, char *name)
{
	static char		temp[MAX_VARLEN];
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
	bzero(temp, MAX_VARLEN - 1);
	if (length >= MAX_VARLEN)
		length = MAX_VARLEN - 1;
	strncpy(temp, pos, length);
	return (char *) temp;
}


/*****************************************************************************
	function:	main
	purpose:	main routine
	returns:	0 - regardless of success or failure
*****************************************************************************/
int main(void)
{
	FILE	*fp;
	int		i, cmdlen;

	printf("Content-type: text/html\n\n");
	printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	get_form_vars();

	if (submitted)
	{
		cmdlen = strlen(cmd);
		for (i = 0; i < cmdlen; i++)
		{
			if (cmd[i] == '+')
				cmd[i] = ' ';
			else if (cmd[i] == '%' && i + 2 < cmdlen)
			{
				cmd[i] = unescapeURLchar(cmd[i + 1], cmd[i + 2]);
				strcpy((char *) &(cmd[i + 1]), (char *) &(cmd[i + 3]));
				cmdlen -= 2;
			}
		}
		
		fp = fopen(filename, "w+");
		if (fp)
		{
			fputs(cmd, fp);
			fputc('\n', fp);
			fclose(fp);
			kill(pid, SIGUSR1);
		}
		else
		{
			printf("FATAL ERROR: couldn't write to transfer file '%s'!\n", filename);
			printf("</head>\n</html>\n");
			exit(0);
		}
	}

/* HTML for the entire input form frame */

	printf("<html><head><link rel=\"stylesheet\" href=\"%s%s/theme.css\">\n", THEMES_PATH, theme);
	printf("<script type=\"text/javascript\">\n<!--\n");
	printf("var idletime = 0\n");
	printf("function UpdateClock()\n{\n");
	printf("idletime++\nvar hours = Math.round(idletime / 3600)\nvar minutes = Math.round((idletime / 60) % 60)\nvar seconds = Math.round(idletime % 60)\n");
	printf("var temp = \"\" + ((hours < 10) ? (\"0\" + hours) : hours)\n");
    printf("temp += ((minutes < 10) ? \":0\" : \":\") + minutes\n");
	printf("temp += ((seconds < 10) ? \":0\" : \":\") + seconds\n");
	printf("document.forms[1].idle.value = temp\nsetTimeout(\"UpdateClock()\", 1000)\n");
	printf("}\n");
	printf("function submitform()\n{\n");
	printf("document.forms[0].submit()\n");
	printf("document.forms[0].cmd.value = ''\n");
	printf("document.forms[0].cmd.focus()\n");
	printf("return false\n");
	printf("}\n");
	printf("//-->\n</script>\n");
	printf("</head>");
	printf("<body class=\"input\" onLoad=\"UpdateClock(); document.forms[0].cmd.focus()\">");
	printf("<table width=\"100%\" border=\"0\"><tr><form name=\"inputform\" method=\"post\" action=\"input.cgi\" onsubmit=\"return submitform()\"><td>");
	printf("<input type=\"hidden\" name=\"pid\" value=\"%d\">", pid);
	printf("<input type=\"hidden\" name=\"theme\" value=\"%s\">", theme);
	printf("<input type=\"hidden\" name=\"submitted\" value=\"1\">");
	printf("<input type=\"hidden\" name=\"passwordmode\" value=\"%d\">", passwordmode);
	printf("<input %s class=\"cmd_prompt\" type=\"%s\" name=\"cmd\" value=\"\" size=\"80\" maxlength=\"%d\" autocomplete=\"off\" />", CATCH_ALL_INPUT ? "onblur=\"this.focus()\"" :" ", passwordmode ? "password" : "text", MAX_VARLEN);
	printf("</td></form><form name=\"clock\"><td align=\"right\" class=\"clock\">idle time:&nbsp;<input readonly class=\"clock\" type=\"text\" name=\"idle\" />");
	printf("</td></form></tr></table></body></html>");
	exit(0);
}
