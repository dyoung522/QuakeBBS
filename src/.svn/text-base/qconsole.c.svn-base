/*
 *  Simple utility to use Quake with FIFO
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
#define VERSION "1.5"

FILE *qin, *qout;

main(argc, argv)
int argc;
char *argv[];
{
	int fini = 0;
	char iname[128],
	     oname[128],
	     buf[256],
	     c;
	fd_set fdmask;

	printf("\nQConsole v.%s (c) 1996 Donovan Young\n\n", VERSION);

    if(argc != 3)
    {
        puts("Usage:  QConsole <quakebbs_home_dir> <Quake_Shortname>\n");
        exit(1);
    }

    getconfig(argv[1], argv[2]);

	fseek(qout, 0L, SEEK_END);

	for(;!fini;)
	{
		fflush(stdin);
		printf("Quake Command >> ");
		gets(buf);

		if(*buf != '\0')
		{
			if(strcmp(buf, "quit") == 0)
			{
                printf("\nNote:  To quit QConsole ONLY, use '/quit'\n");
				printf("\nSend 'quit' command to Quake Server?  [y/N] ");
				c = tolower(getchar());
				if(tolower(c) != 'y')
                {
					continue;
                }
				else
					fini = 1;
			}
			if(strcmp(buf, "/quit") == 0)
                exit(0);

			fprintf(qin, "%s\n", buf);
			fflush(qin);

			sleep(1);
		}

		while(fgets(buf, sizeof(buf), qout) != NULL)
			printf(buf);
		fflush(stdout);
	}
	fclose(qin);
	fclose(qout);
}

getconfig(qbbsdir, qshort)
char *qbbsdir;
char *qshort;
{
    FILE *cf, *sf;
    char qbbsconfig[128],
         qbbsserver[128],
         buf[256],
         item[20],
         value[129],
	     iname[128],
	     oname[128],
         *tok;

    struct qbbs_quake_server qserver;

    sprintf(qbbsserver, "%s/quake.servers", qbbsdir);

    if((sf = fopen(qbbsserver, "r")) == NULL)
    {
        printf("Could not open Quake Server configuration file:  %s\n\n", qbbsserver);
        exit(1);
    }

    while(fgets(buf, sizeof(buf), sf) != NULL)
    {
        if((buf[0] != '#') && (buf[0] != '\n'))
        {
            if((tok = (char *)strtok(buf, ":\n")) != NULL)
                strcpy(qserver.name, tok);
            if((tok = (char *)strtok(NULL, ":\n")) != NULL)
                qserver.addr = inet_addr(tok);
            if((tok = (char *)strtok(NULL, ":\n")) != NULL)
                qserver.port = atoi(tok);
            if((tok = (char *)strtok(NULL, ":\n")) != NULL)
                strcpy(qserver.qshort, tok);
            if((tok = (char *)strtok(NULL, ":\n")) != NULL)
                strcpy(qserver.desc, tok);
            if(strcmp(qshort, qserver.qshort) == 0)
                break;
        }
    }

    if(strcmp(qshort, qserver.qshort) != 0)
    {
        printf("%s not found in %s\n", qshort, qbbsserver);
        exit(1);
    }

    sprintf(qbbsconfig, "%s/quakebbs.conf", qbbsdir);

    if((cf = fopen(qbbsconfig, "r")) == NULL)
    {
        printf("Could not open QuakeBBS configuration file:  %s\n\n", qbbsconfig);
        exit(1);
    }

    while(fgets(buf, sizeof(buf), cf) != NULL)
    {
        if((buf[0] != '#') && (buf[0] != '\n'))
        {
            sscanf(buf, "%s %s", &item, &value);

            if(!strcmp("QBBS_DATA_DIR", item))
            {
                sprintf(iname, "%s/%s.in", value, qshort);
                continue;
            }

            if(!strcmp("QBBS_LOGS_DIR", item))
            {
                sprintf(oname, "%s/%s.log", value, qshort);
                continue;
            }
        }
    }

	if((qin = fopen(iname, "w")) == NULL)
	{
		sprintf(buf, "Error opening %s", iname);
		perror(buf);
		exit(1);
	}

	if((qout = fopen(oname, "r")) == NULL)
	{
		sprintf(buf, "Error opening %s", oname);
		perror(buf);
		exit(1);
	}

    /* startquake(iname, qserver.port); */

    return;
}

startquake(pipe, port)
char *pipe;
long port;
{
    if(fork() == 0)
    {
        execl("./startserver", "startserver", "-i", pipe, "-p", port, NULL);
        perror("./startserver failed");
        exit(1);
    }
    return;
}

