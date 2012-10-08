/*
 *  QuakeBBS Configuration
 */

/*
    QuakeBBS - Quake Server Control system
    Copyright (C) 1996  Donovan C. Young

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    To contact me, send mail to donovan@mindspring.com.
*/

#include "quakebbs.h"

extern struct qbbs_config config;
extern struct qbbs_quake_server qserver[MAXQUAKESERVER];

get_config()
{
	int fconfig;
	char buf[256],
		 item[20],
		 value[129];
	
	if(config.file[0][0] == 0)
		strcpy(config.file[0], "quakebbs.conf");

	if((fconfig = open(config.file[0], O_RDONLY)) < 0)
		logsyserr("<get_conf> could not open %s", config.file[0]);

	loginfo("Reading config from %s", config.file[0]);

	while(readline(fconfig, &buf, sizeof(buf)))
	{
		if((buf[0] != '#') && (buf[0] != '\n'))
		{
			sscanf(buf, "%s %s", &item, &value);

			/*  System Info  */

			if(!strcmp("QBBS_SYS_NAME", item))
			{
				memcpy(config.sys[0], value, strlen(value)+1);
				continue;
			}

			if(!strcmp("QBBS_SYS_SYSOP", item))
			{
				memcpy(config.sys[1], value, strlen(value)+1);
				continue;
			}

			if(!strcmp("QBBS_SYS_EMAIL", item))
			{
				memcpy(config.sys[2], value, strlen(value)+1);
				continue;
			}

			if(!strcmp("QBBS_SYS_URL", item))
			{
				memcpy(config.sys[3], value, strlen(value)+1);
				continue;
			}

			/*  Miscellaneous  */

			if(!strcmp("QBBS_CONTROL_WAIT", item))
			{
				config.misc[0] = atoi(value);
				continue;
			}

			/*  Directories  */

			if(!strcmp("QBBS_HOME_DIR", item))
			{
				memcpy(config.dirs[0], value, strlen(value)+1);
				continue;
			}
			if(!strcmp("QBBS_DATA_DIR", item))
			{
				memcpy(config.dirs[1], value, strlen(value)+1);
				continue;
			}
			if(!strcmp("QBBS_TEXT_DIR", item))
			{
				memcpy(config.dirs[2], value, strlen(value)+1);
				continue;
			}
			if(!strcmp("QBBS_LOGS_DIR", item))
			{
				memcpy(config.dirs[3], value, strlen(value)+1);
				continue;
			}
			if(!strcmp("QBBS_MESG_DIR", item))
			{
				memcpy(config.dirs[4], value, strlen(value)+1);
				continue;
			}
			if(!strcmp("QBBS_FILE_DIR", item))
			{
				memcpy(config.dirs[5], value, strlen(value)+1);
				continue;
			}

			/*  Filenames  */

			if(!strcmp("QBBS_USER_FILE", item))
			{
				sprintf(config.file[1], "%s/%s", config.dirs[1], value);
				continue;
			}
			if(!strcmp("QUAKE_SERVER_FILE", item))
			{
				sprintf(config.file[2], "%s/%s", config.dirs[0], value);
				continue;
			}
			if(!strcmp("QBBS_STRING_FILE", item))
			{
				sprintf(config.file[3], "%s/%s", config.dirs[1], value);
				continue;
			}
			if(!strcmp("QBBS_LOG_FILE", item))
			{
				sprintf(config.file[4], "%s/%s", config.dirs[3], value);
				continue;
			}

			/*  Server Info  */

			if(!strcmp("QBBS_ADDR", item))
			{
				config.serv[0].addr = inet_addr(value);
				continue;
			}
			if(!strcmp("QBBS_TCP_PORT", item))
			{
				config.serv[0].port[0] = atoi(value);
				continue;
			}
			if(!strcmp("QBBS_UDP_PORT", item))
			{
				config.serv[0].port[1] = atoi(value);
				continue;
			}

			/*  Runtime Options  */

			if(!strcmp("QBBS_OPT_SYSLOG", item))
			{
				config.opts.syslog = atoi(value);
				continue;
			}
			if(!strcmp("QBBS_OPT_FILELOG", item))
			{
				config.opts.logfile = atoi(value);
				continue;
			}
			if(!strcmp("QBBS_OPT_FORCENAME", item))
			{
				config.opts.forcename = atoi(value);
				continue;
			}
		}
	}
	close(fconfig);

	/*  Open our logfile, if needed  */

	if(config.opts.logfile)
		loginfo("Logging to %s", config.file[4]);

	return(0);
}

get_servers()
{
	int x = 0,
	    fquakes;
	char line[256],
	     *tok;

	if((fquakes = open(config.file[2], O_RDONLY)) < 0)
		logsyserr("<get_conf> could not open %s", config.file[2]);

	loginfo("Reading Quake servers from %s", config.file[2]);

	while(readline(fquakes, &line, sizeof(line)))
	{
		if((line[0] != '#') && (line[0] != '\n'))
		{
			if((tok = (char *)strtok(line, ":\n")) != NULL)
				strcpy(qserver[x].name, tok);
			if((tok = (char *)strtok(NULL, ":\n")) != NULL)
				qserver[x].addr = inet_addr(tok);
			if((tok = (char *)strtok(NULL, ":\n")) != NULL)
				qserver[x].port = atoi(tok);
			if((tok = (char *)strtok(NULL, ":\n")) != NULL)
				strcpy(qserver[x].qshort, tok);
			if((tok = (char *)strtok(NULL, ":\n")) != NULL)
				strcpy(qserver[x].desc, tok);
			x++;
		}
	}

	qserver[x].name[0] = '\0';	/* Terminate the list cleanly */

	loginfo("\nConfigured servers:\n");

	for(x=0 ; x<MAXQUAKESERVER ; x++)
	{
		if(qserver[x].name[0] == '\0')
			break;
		loginfo("\t[%d] %s (%s:%d)\n\t\tinput on %s.in\n\t\toutput to %s.out",
				 x,
		         qserver[x].name,
		         inet_ntoa(qserver[x].addr),
				 qserver[x].port,
				 qserver[x].qshort, qserver[x].qshort);
	}
	putchar('\n');
	return(0);
}

int readline(fd, ptr, maxlen)
register int	fd;
register char	*ptr;
register int	maxlen;
{
	int	n, rc;
	char	c;

	for(n = 1; n < maxlen; n++)
	{
		if((rc = read(fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			if(c == '\r' || c == '\n')
				break;
		}
		else if(rc == 0)
		{
			if(n == 1)
				return(0);	/* EOF, no data read */
			else
				break;		/* EOF, some data was read */
		}
		else
			return(-1);	/* error */
	}

	*ptr = 0;
	return(n);
}
