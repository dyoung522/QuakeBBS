/*
 *  QuakeBBS Server Daemon
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

#ifndef DEBUG

#include "quakebbs.h"
#include <sys/wait.h>
#include <sys/param.h>

extern struct qbbs_config config;

daemonize()
{
	register int	childpid, fd;

	loginfo("Changing to run as daemon");

	loginit(pname);

	if (getppid() == 1)
  		goto out;

	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	if ( (childpid = fork()) < 0)
		logsyserr("can't fork first child");
	else if (childpid > 0)
		exit(0);	/* parent */

	if (setpgrp() == -1)
		logsyserr("can't change process group");

	signal(SIGHUP, SIG_IGN);	/* immune from pgrp leader death */

	if ( (childpid = fork()) < 0)
		logsyserr("can't fork second child");
	else if (childpid > 0)
		exit(0);	/* first child */

out:

	for (fd = 0; fd < NOFILE; fd++)
		close(fd);

	errno = 0;		/* probably got set to EBADF from a close */

	/*
	 * Move the current directory to the QuakeBBS home
	 */

	chdir(config.dirs[0]);

	umask(0);

	/*
	 * Ignore exit status of any children
	 */

	signal(SIGCLD, SIG_IGN);

	config.opts.conlog = 0;  /*  Turn terminal logging off  */

	/*  Actually set the processes lock  */
	if(!lockqbbs(1))
		exit(1);	/*  Another program started between since last check */
}

lockqbbs(mode)
int mode;
{
	int lfd;
	char pid[10];

	/*  Check to see if our program is already running  */

	if((lfd = open(QBBSLOCKFILE, O_WRONLY | O_CREAT, 0644)) < 0)
		logsyserr("<qbbsdaemon> Could not open lockfile %s", QBBSLOCKFILE);

	if(flock(lfd, LOCK_EX | LOCK_NB) < 0)
	{
		if(errno == EWOULDBLOCK)
		{
			loginfo("%s already running", pname);
			return(0);
		}
		logsyserr("Could not lock %s", QBBSLOCKFILE);
	}

	if(mode)	/* mode == 1, write our pid to the lockfile */
	{	
		sprintf(pid, "%d", getpid());

		if(write(lfd, pid, strlen(pid)) < 0)
			logsyserr("<lockqbbs> write error");
	}

	return(1);
}

#endif /* DEBUG */
