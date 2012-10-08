/*
 *  QuakeBBS - Quake Server Control System
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
#include "qbbsuser.h"
#include <sys/param.h>

void qbbs_exit();
void qbbs_reconfig();
void qbbs_sigtrap(int);

extern QBBSd();
extern QBBSControl();
extern QBBSChat();

extern char *optarg;
extern int optind;

struct qbbs_config config;
struct qbbs_quake_server qserver[MAXQUAKESERVER];
u_char qserverno = 0;

#define NUMCHILDPROC 3

struct {
    char name[16];
    int (*function)();
    pid_t pid;
    int restart;
} child[NUMCHILDPROC] = { 
                            "QBBSd",       QBBSd,       0,  1,
                            "QBBSControl", QBBSControl, 0,  1,
                            "QBBSChat",    QBBSChat,    0,  0
                        };

int msqid;

usage()
{
    fprintf(stderr, "Usage:\t%s [hV][f <configfile>]\n", pname);
    fprintf(stderr, "\n");
    fprintf(stderr, "\th\tThis Message\n");
    fprintf(stderr, "\tV\tPrint version and exit\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "\tf <configfile>\tUse <configfile> for configuration information.  Default is \"quakebbs.conf\"\n");
    fprintf(stderr, "\n");
    return;
}

main(argc, argv)
int argc;
char *argv[];
{
    int opt;

    pname = argv[0];

    bzero(config, sizeof(config));
    bzero(qserver, sizeof(qserver));

    config.opts.conlog = 1;  /*  Turn terminal logging on  */

    if(argc > 1)
    {
        while((opt = getopt(argc, argv, "f:hV")) != -1)
        {
            switch(opt)
            {
                case 'f':
                    memcpy(config.file[0], optarg, strlen(optarg)+1);
                    break;

                case 'V':
                    fprintf(stdout, "QuakeBBS v%s (c) 1996 Donovan Young\n", QBBS_VERSION);
                    exit(0);

                default:
                    usage();
                    exit(opt != 'h');
            }
        }
    }

#ifndef DEBUG
    if(!lockqbbs(0))
        exit(0);
#endif

    get_config();        /* Retrieve our configuration */
    get_servers();        /* Retrieve our supported Quake servers list */
#ifndef DEBUG
    daemonize();        /* Become a daemon */
#endif

    set_signals();

    open_message_queue();
    spawn_children();

    qbbs_watchdog();
}

open_message_queue()
{
    if((msqid = msgget(QBBS_MSGQ_MKEY, QBBS_MSGQ_PERM | IPC_CREAT)) < 0)
        logsyserr("<quakebbs> Unable to open message queue");

    return;
}

spawn_children()
{
    int pid, x;

    for(x = 0; x < NUMCHILDPROC; x++)
    {
        if((pid = fork()) < 0)
            logsyserr("<quakebbs> could not fork %s", child[x].name);

        if(pid)
            child[x].pid = pid;
        else
            (*child[x].function)();

        loginfo("<quakebbs> %s started (PID %d)", child[x].name, child[x].pid);
    }

    return;
}

qbbs_watchdog()
{
    int x, pid;

    for(;;)
    {
        for(x = 0; x < NUMCHILDPROC; x++)
        {
            if(kill(child[x].pid, 0) < 0)
            {
                loginfo("<quakebbs> %s died", child[x].name);
                if(child[x].restart)
                {
                    loginfo("<quakebbs> attempting restart of %s", child[x].name);
                    if((pid = fork()) < 0)
                    {
                        logerr("<quakebbs> could not restart %s", child[x].name);
                        qbbs_exit();
                    }

                    if(pid)
                        child[x].pid = pid;
                    else
                        (*child[x].function)();

                    loginfo("<quakebbs> %s restarted with pid %d", child[x].name, child[x].pid);
                }
                else
                    qbbs_exit();
            }
        }
        sleep(5);
    }
    return;
}

void qbbs_exit()
{
    int x, fd;

    signal(SIGTERM, SIG_IGN);

    loginfo("<quakebbs> shutting down");

    for(x = 0; x < NUMCHILDPROC; x++)
        kill(child[x].pid, SIGTERM);

    if(msgctl(msqid, IPC_RMID, NULL) < 0)
        logerr("<quakebbs> could not remove message queue");

    for (fd = 0; fd < NOFILE; fd++)
        close(fd);

    exit(0);
}

void qbbs_reconfig()
{
    signal(SIGHUP, qbbs_reconfig);

    loginfo("<qbbsconfig> Rereading %s", config.file[0]);
    get_config();
    loginfo("<qbbsconfig> Rereading %s", config.file[2]);
    get_servers();

    return;
}

set_signals()
{
    signal(SIGHUP, qbbs_sigtrap);       /*   1  */
    signal(SIGINT, qbbs_sigtrap);       /*   2  */
    signal(SIGQUIT, qbbs_sigtrap);      /*   3  */
    signal(SIGILL, qbbs_sigtrap);       /*   4  */
    signal(SIGTRAP, qbbs_sigtrap);      /*   5  */
    signal(SIGIOT, SIG_IGN);            /*   6  */
    signal(SIGBUS, qbbs_sigtrap);       /*   7  */
    signal(SIGFPE, qbbs_sigtrap);       /*   8  */
    signal(SIGUSR1, SIG_IGN);           /*  10  */
    signal(SIGSEGV, qbbs_sigtrap);      /*  11  */
    signal(SIGUSR2, SIG_IGN);           /*  12  */
    signal(SIGPIPE, SIG_IGN);           /*  13  */
    signal(SIGALRM, qbbs_sigtrap);      /*  14  */
    signal(SIGTERM, qbbs_exit);         /*  15  */
    signal(SIGSTKFLT, qbbs_sigtrap);    /*  16  */
    signal(SIGCHLD, SIG_IGN);           /*  17  */
    signal(SIGCONT, SIG_IGN);           /*  18  */
    signal(SIGSTOP, SIG_IGN);           /*  19  */
    signal(SIGTSTP, SIG_IGN);           /*  20  */
    signal(SIGTTIN, SIG_IGN);           /*  21  */
    signal(SIGTTOU, SIG_IGN);           /*  22  */
    signal(SIGURG, SIG_IGN);            /*  23  */
    signal(SIGXCPU, qbbs_sigtrap);      /*  24  */
    signal(SIGXFSZ, qbbs_sigtrap);      /*  25  */
    signal(SIGVTALRM, qbbs_sigtrap);    /*  26  */
    signal(SIGPROF, qbbs_sigtrap);      /*  27  */
    signal(SIGWINCH, SIG_IGN);          /*  28  */
    signal(SIGIO, SIG_IGN);             /*  29  */
    signal(SIGPWR, SIG_IGN);            /*  30  */
}

void qbbs_sigtrap(sig)
int sig;
{
    loginfo("<quakebbs> caught signal %d; Terminating", sig);
    qbbs_exit();
}

void qbbs_wait(ms)
int ms;
{
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = ms;
    select(0, NULL, NULL, NULL, &tv);

    return;
}
