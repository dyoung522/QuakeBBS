/*
 *  QuakeBBS Control Daemon
 *
 *  Monitors and kills unauthorized connects on Quake server
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

#include    "quakebbs.h"
#include    "qbbsuser.h"
#include    <sys/param.h>
#include    <sys/stat.h>

void qbbs_control_exit();

char *readstr(char *, char *);
char *readlong(char *, long *);

struct ccrep_player_info
{
    int num;
    char name[QBBSNAMELEN];
    unsigned int pantcolor;
    unsigned int shirtcolor;
    long frags;
    unsigned long contime;
    char addr[QBBSADDRLEN+10];
} qplayer[QBBS_MAXPLAYER];

struct ccrep_server_info
{
    char addr[QBBSADDRLEN+10];
    char name[QBBSNAMELEN];
    char level[QBBSNAMELEN];
    byte numplayers;
    byte maxplayers;
    byte protver;
} serverinfo;

struct auth_list
{
    struct qbbs_auth_list auth;
    time_t starttime;
    struct auth_list *next;
} *authlist = NULL, *authlist_top = NULL;

struct wait_list
{
    struct qbbs_auth_list auth;
    time_t starttime;
    struct wait_list *next;
} *waitlist = NULL, *waitlist_top = NULL;

extern struct qbbs_config config;
extern struct qbbs_quake_server qserver[MAXQUAKESERVER];
extern u_char qserverno;
extern int msqid;

int authnum = 0;

QBBSControl()
{
    int status;
    fd_set fdmask;
    static struct timeval timeout;

    if(config.misc[0] < 1 && config.misc[0] > 300)
        config.misc[0] = 15;        /*  Default sleep time    */

    signal(SIGINT, qbbs_control_exit);
    signal(SIGTERM, qbbs_control_exit);
    signal(SIGHUP, qbbs_control_exit);

    serverstat();

    for(;;sleep(config.misc[0]))
    {
        status = serverstat();/*  Get Server Info (numplayers & maxplayers)  */
        waitlist_check();     /*  Check the Waiting Queue                    */
        buildlist(status);    /*  Process user requests                      */
        if(status == 0)
            checkserver();    /*  Check for unauthorized / invalid players   */

#ifdef DEBUG
        logdebug("<qbbscontrol> Waiting for %d seconds...", config.misc[0]);
#endif
    }
}

buildlist(mode)
int mode;
{
    int n, x, status = 0;
    struct
    {
        long mtype;
        struct qbbs_auth_list listdata;
    } mesg;
    char buf[QBBSMAXBUF],
         *reason = &buf[2];

    mesg.mtype = 2L;

    while((n = msgrcv(msqid, (struct msgbuf *)&mesg, sizeof(mesg.listdata), mesg.mtype, IPC_NOWAIT | MSG_NOERROR)) > 0)
    {
        status = 1;

        switch(mesg.listdata.command)
        {
            case QBBS_AUTH_DEL:
                strcpy(reason, "Per Request");
                buf[0] = QBBS_CONT_REMOVE;
                buf[1] = qserverno + 1;
                sendtouser(mesg.listdata.type, buf);
                loginfo("<qbbscontrol> %s removed: %s", mesg.listdata.name, reason);

                authlist_del(&mesg.listdata);        /* Del data from List */
                if(waitlist_top != NULL)    /*  If there is someone waiting  */
                    waitlist_check();       /*  give them this slot          */
                break;

            case QBBS_AUTH_ADD:
                if(waitlist_top != NULL)    /*  If there is someone waiting  */
                    waitlist_check();       /*  give them first dibs         */

                authlist_add(&mesg.listdata, mode);
                break;

            case QBBS_AUTH_KICK:
                buf[0] = QBBS_CONT_KICK;
                buf[1] = qserverno + 1;
                if(authlist_dup(&mesg.listdata))
                {
                    sprintf(reason, "%s kicked from authorization list", mesg.listdata.name);
                    loginfo("<qbbscontrol> %s kicked", mesg.listdata.name);

                    sendtouser(authlist->auth.type, buf);
                    authlist_del(&mesg.listdata);
                    if(waitlist_top != NULL)
                        waitlist_check();
                }
                else
                    sprintf(reason, "%s not on authorization list", mesg.listdata.name);
                sendtouser(mesg.listdata.type, buf);
                break;

            case QBBS_AUTH_LIST:
                buf[0] = QBBS_CONT_LIST;
                buf[1] = qserverno + 1;
                sprintf(reason, "%s Player list", qserver[qserverno].name);
                sendtouser(mesg.listdata.type, buf);
                for(x=0;x<serverinfo.numplayers;x++)
                {
                    sprintf(reason, "    [%02d] %-25.25s (%02u/%02u)  %3ld  %02u:%02u:%02u",
                            qplayer[x].num + 1,
                            qplayer[x].name,
                            qplayer[x].pantcolor,
                            qplayer[x].shirtcolor,
                            qplayer[x].frags,
                            qplayer[x].contime/3600,
                            qplayer[x].contime/60,
                            qplayer[x].contime%60);

                    sendtouser(mesg.listdata.type, buf);
                }
                if(serverinfo.numplayers == 0)
                {
                    sprintf(reason, "    No Players");
                    sendtouser(mesg.listdata.type, buf);
                }
                break;
        }
    }
    if(n < 0 && errno != ENOMSG)
        logsyserr("<qbbscontrol> msgrcv error");

    return(status);
}

authlist_add(data, fromwait)
struct qbbs_auth_list *data;
register int fromwait;
{
    struct auth_list *new;
    char buf[QBBSMAXBUF],
         *reason = &buf[2];

    if(authlist_dup(data))        /* User already on list */
    {
        strcpy(reason, "Already Authorized");
        buf[0] = QBBS_CONT_RAUTH;
        buf[1] = qserverno + 1;
        sendtouser(data->type, buf);
        loginfo("<qbbscontrol> %s denied: %s", data->name, reason);
        return;
    }

    if(fromwait < 0)        /* Server is down */
    {
        strcpy(reason, "Server Unavailable");
        buf[0] = QBBS_CONT_DOWN;
        buf[1] = qserverno + 1;
        sendtouser(data->type, buf);
        loginfo("<qbbscontrol> %s denied: %s", data->name, reason);
        return;
    }

    if((!fromwait) && (authnum >= serverinfo.maxplayers)) /* Authlist is full */
    {
        strcpy(reason, "Server Full");
        buf[0] = QBBS_CONT_RFULL;
        buf[1] = qserverno + 1;
        sendtouser(data->type, buf);
        loginfo("<qbbscontrol> %s denied: %s", data->name, reason);
        waitlist_add(data);
        return;
    }

    if((new = (struct auth_list *)malloc(sizeof(struct auth_list))) == NULL)
        logsyserr("<qbbscontrol> malloc error");

    strcpy(new->auth.name, data->name);
    strcpy(new->auth.addr, data->addr);
    new->auth.type = data->type;
    new->auth.tallow = data->tallow;
    time(&new->starttime);
    new->next = NULL;

    authlist = authlist_top;

    if(authlist == NULL)
    {
        authlist = new;
        authlist_top = authlist;
    }
    else
    {
        while(authlist->next != NULL)
            authlist = authlist->next;
        authlist->next = new;
    }

    if(!fromwait)
    {
        loginfo("<qbbscontrol> %s@%s Authorized", data->name, data->addr);
        buf[0] = QBBS_CONT_ACCEPT;
        buf[1] = qserverno + 1;
        *reason = '\0';
        sendtouser(data->type, buf);
    }
    authnum++;

    return;
}

authlist_dup(data)
struct qbbs_auth_list *data;
{
    authlist = authlist_top;

    while((authlist != NULL) && (strcmp(authlist->auth.name, data->name) != 0))
        authlist = authlist->next;
    
    if(authlist == NULL)
        return(0);    /* User not on list  */
    else
        return(1);    /* User already on list */
}

authlist_del(data)
struct qbbs_auth_list *data;
{
    struct auth_list *ptr;

    authlist = authlist_top;
    ptr = authlist;

    if(authlist == NULL)
        return;

    loginfo("<qbbscontrol> Deleting %s from list", data->name);

    if(authlist->next == NULL)
    {
        free(authlist);
        authlist_top = NULL;
        authnum = 0;
        return;
    }

    while((authlist != NULL) && (strcmp(authlist->auth.name, data->name) != 0))
    {
        ptr = authlist;
        authlist = authlist->next;
    }

    if(authlist == NULL)
        return;
    else
    {
        if(authlist_top == authlist)
            authlist_top = authlist->next;
        else
            ptr->next = authlist->next;
        free(authlist);
        authnum--;
    }

    return;
}

checkserver()
{
    int n,
        x,
        sockfd,
        found,
        servlen;
    struct sockaddr_in our_addr, serv_addr;
    fd_set fdmask;
    static struct timeval timeout;
    char ccreq[] = { 0x80, 0x00, 0x00, 0x06, 0x03, 0 };
    char pkt[QBBSMAXPKT];
    char buf[QBBSMAXBUF];
    char *reason = &buf[2];

#ifdef TRACE
    logdebug("<qbbscontrol> checking server %s", qserver[qserverno].name);
#endif

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = qserver[qserverno].addr;
    serv_addr.sin_port        = htons(qserver[qserverno].port);
/*
    serv_addr.sin_addr.s_addr = inet_addr("207.48.50.5");
    serv_addr.sin_port        = htons(26000);
*/

    servlen = sizeof(serv_addr);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        logsyserr("<qbbscontrol> can't open datagram socket");

    /*
     * Bind any local address for us.
     */

    bzero((char *) &our_addr, sizeof(our_addr));    /* zero out */
    our_addr.sin_family      = AF_INET;
    our_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    our_addr.sin_port        = htons(0);
    if(bind(sockfd, (struct sockaddr *) &our_addr, sizeof(our_addr)) < 0)
        logsyserr("<qbbscontrol> can't bind local address");
    
    for(x=0;x<serverinfo.numplayers;x++)
    {
        FD_ZERO(&fdmask);
        FD_SET(sockfd, &fdmask);

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        ccreq[5] = x;

        if(sendto(sockfd, ccreq, ccreq[3], 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            logerr("<qbbscontrol> sendto error");
            close(sockfd);
            return;
        }

        if((n = select(sockfd+1, &fdmask, NULL, NULL, &timeout)) < 0)
            logerr("<qbbscontrol> select error");

        if(!n)
        {
            qplayer[x].num = -1;
            loginfo("<qbbscontrol> ccreq player %d timeout", x);
        }

        if(FD_ISSET(sockfd, &fdmask))
        {
            if((n = recvfrom(sockfd, &pkt, QBBSMAXPKT, 0, (struct sockaddr *)&serv_addr, &servlen)) < 0)
            {
                logerr("<qbbscontrol> recvfrom error");
                close(sockfd);
                return;
            }
            
            if(pkt[4] == (char)0x84)
                process_player_info(&pkt, &qplayer[x]);
        }
    }

    close(sockfd);

    for(x = 0; x < serverinfo.numplayers; x++)
        if(qplayer[x].num != -1)
            checkplayer(&qplayer[x]);

#ifdef TRACE
    logdebug("<qbbscontrol> Checking list for timeouts...");
#endif

    authlist = authlist_top;
    while(authlist != NULL)
    {
        for(x = 0, found = 0; (!found) && (x < serverinfo.numplayers); x++)
        {
            if(qplayer[x].num != -1)
            {
                if((strcmp(qplayer[x].addr, authlist->auth.addr) == 0) &&
                   (strcmp(qplayer[x].name, authlist->auth.name) == 0))
                    found = 1;
            }
        }
        if(!found)
        {
#ifdef DEBUG
            logdebug("<qbbscontrol> difftime = %lf", difftime(time(NULL), authlist->starttime));
#endif
            if(difftime(time(NULL), authlist->starttime) > QBBS_CONTROL_WAIT)
            {
                strcpy(reason, "Timeout waiting for connect");
                buf[0] = QBBS_CONT_REMOVE;
                buf[1] = qserverno + 1;
                loginfo("<qbbscontrol> %s revoked: %s", authlist->auth.name, reason);
                sendtouser(authlist->auth.type, &buf);
                authlist_del(&authlist->auth);
            }
        }
        authlist = authlist->next;
    }

    return;
}

serverstat()
{
    int n,
        sockfd,
        servlen;
    struct sockaddr_in our_addr, serv_addr;
    fd_set fdmask;
    static struct timeval timeout;
    char ccreq[] = { 0x80, 0x00, 0x00, 0x0c, 
                     0x02, 'Q',  'U',  'A',
                     'K',  'E',  0x00, 0x03 };
    char pkt[QBBSMAXPKT];

    /*
     * Fill in the structure "serv_addr" with the address of the
     * server that we want to send to.
     */

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = qserver[qserverno].addr;
    serv_addr.sin_port        = htons(qserver[qserverno].port);
/*
    serv_addr.sin_addr.s_addr = inet_addr("207.48.50.5");
    serv_addr.sin_port        = htons(26000);
*/

    servlen = sizeof(serv_addr);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        logsyserr("<qbbscontrol> can't open datagram socket");

    /*
     * Bind any local address for us.
     */

    bzero((char *) &our_addr, sizeof(our_addr));    /* zero out */
    our_addr.sin_family      = AF_INET;
    our_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    our_addr.sin_port        = htons(0);
    if (bind(sockfd, (struct sockaddr *) &our_addr, sizeof(our_addr)) < 0)
        logsyserr("<qbbscontrol> can't bind local address");
    
    FD_ZERO(&fdmask);
    FD_SET(sockfd, &fdmask);

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if(sendto(sockfd, ccreq, ccreq[3], 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        logerr("<qbbscontrol> sendto error");
        close(sockfd);
        return(-1);
    }

    if((n = select(sockfd+1, &fdmask, NULL, NULL, &timeout)) < 0)
        logerr("<qbbscontrol> select error");

    if(!n)
    {
        loginfo("<qbbscontrol> Timeout sending Server Info Request");
        close(sockfd);
        return(-1);
    }

    if(recvfrom(sockfd, &pkt, QBBSMAXPKT, 0, (struct sockaddr *)&serv_addr, &servlen) < 0)
    {
        logerr("<qbbscontrol> recvfrom error");
        close(sockfd);
        return(-1);
    }

    if(pkt[4] == (char)0x83)
        process_server_info(&pkt);

    close(sockfd);
    return(0);
}

checkplayer(player)
struct ccrep_player_info *player;
{
#ifdef DEBUG
    logdebug("<qbbscontrol> checking player %s", player->name);
    printplayer(player);
#endif

    authlist = authlist_top;
    while(authlist != NULL)
    {
        if(config.opts.forcename)
        {
            if(strcmp(player->name, "unconnected") == 0 && player->contime < 30)
                return(1);

            if((strcmp(player->addr, authlist->auth.addr) == 0) &&
               (strcmp(player->name, authlist->auth.name) == 0))
                return(1);
        }
        else
        {
            if(strcmp(player->addr, authlist->auth.addr) == 0)
                return(1);
        }
        authlist = authlist->next;
    }

    kickplayer(player);

    return(0);
}

kickplayer(player)
struct ccrep_player_info *player;
{
    char buf[QBBSMAXBUF];

    loginfo("<qbbscontrol> Kicking unauthorized player (%s@%s)", player->name, player->addr);

    sprintf(buf, "kick %s\n", player->name);

    sendtoqserver(buf, strlen(buf));

    return;
}

printplayer(player)
struct ccrep_player_info *player;
{
    logdebug("<playerinfo> [%d] %s", player->num, player->name);
    logdebug("<playerinfo>      colors:  %u/%u", player->pantcolor, player->shirtcolor);
    logdebug("<playerinfo>      frags:   %ld", player->frags);
    logdebug("<playerinfo>      time:    %lu", player->contime);
    logdebug("<playerinfo>      addr:    %s", player->addr);

    return;
}

process_player_info(pkt, qplayer)
char *pkt;
struct ccrep_player_info *qplayer;
{
    int n;
    char *ptr;
    long colors;

    n = pkt[5];
    qplayer->num = n;

    ptr = &pkt[6];
    strcpy(qplayer->name, ptr);
    ptr += strlen(ptr) + 1;

    ptr = readlong(ptr, &colors);
    qplayer->pantcolor = colors >> 4;
    qplayer->shirtcolor = colors & 0x0F;

    ptr = readlong(ptr, &qplayer->frags);
    ptr = readlong(ptr, &qplayer->contime);

    strcpy(qplayer->addr, ptr);
    strtok(qplayer->addr, ":");

    return;
}

process_server_info(pkt)
char *pkt;
{
    char *ptr;

    ptr = &pkt[5];
    strcpy(serverinfo.addr, ptr);
    strtok(serverinfo.addr, ":");
    ptr += strlen(ptr) + 1;

    strcpy(serverinfo.name, ptr);
    ptr += strlen(ptr) + 1;

    strcpy(serverinfo.level, ptr);
    ptr += strlen(ptr) + 1;

    serverinfo.numplayers = *ptr++;

    serverinfo.maxplayers = *ptr++;

    serverinfo.protver = *ptr;

    return;
}

char *readlong(p, val)
char *p;
long *val;
{
    *val = (*p);
    p++;
    *val += (*p) << 8;
    p++;
    *val += (*p) << 16;
    p++;
    *val += (*p) << 24;
    p++;
    return p;
}

sendtoqserver(buf, len)
char *buf;
{
    int fd;
    char fname[QBBSFILELEN];

    sprintf(fname, "%s/%s.in", config.dirs[1], qserver[0].qshort);

    if((fd = open(fname, O_WRONLY)) < 0)
        logsyserr("<qbbscontrol> error opening %s", fname);

    if(write(fd, buf, len) < 0)
        logsyserr("<qbbscontrol> error sending command to server");
    
    close(fd);

    return;
}

void qbbs_control_exit()
{
    int fd;
    struct auth_list *ptr;

    loginfo("<qbbscontrol> shutting down");

    for (fd = 0; fd < NOFILE; fd++)
        close(fd);

    authlist = authlist_top;

    while(authlist != NULL)
    {
        ptr = authlist->next;
        free(authlist);
        authlist = ptr;
    }

    exit(0);
}

waitlist_add(data)
struct qbbs_auth_list *data;
{
    struct wait_list *new;
    char buf[QBBSMAXBUF],
         *reason = &buf[2];

    if(authlist_dup(data))        /* User already on waiting list */
    {
        strcpy(reason, "Already on QuakeWait Queue");
        buf[0] = QBBS_CONT_RWAIT;
        buf[1] = qserverno + 1;
        sendtouser(data->type, buf);
        loginfo("<qbbscontrol> %s %s", data->name, reason);
        return;
    }

    if((new = (struct wait_list *)malloc(sizeof(struct wait_list))) == NULL)
        logsyserr("<qbbscontrol> malloc error");

    strcpy(new->auth.name, data->name);
    strcpy(new->auth.addr, data->addr);
    new->auth.type = data->type;
    new->auth.tallow = data->tallow;
    time(&new->starttime);
    new->next = NULL;

    waitlist = waitlist_top;

    if(waitlist == NULL)
    {
        waitlist = new;
        waitlist_top = waitlist;
    }
    else
    {
        while(waitlist->next != NULL)
            waitlist = waitlist->next;
        waitlist->next = new;
    }

    loginfo("<qbbscontrol> %s added to QuakeWait", data->name);

    return;
}

waitlist_deltop()
{
    waitlist = waitlist_top;    /*  Make sure we're at the top  */

    if(waitlist == NULL)
        return;
    else
    {
        waitlist_top = waitlist->next;
        free(waitlist);
    }
    waitlist = waitlist_top;    /*  Move pointer back to valid top  */

    return;
}

waitlist_check()
{
    char buf[QBBSMAXBUF],
         *reason = &buf[2];

    if(authnum >= serverinfo.maxplayers) /*  Server is full  */
        return(0);

    waitlist = waitlist_top;

    if(waitlist == NULL)
        return(0);

    while(waitlist != NULL)
    {
        buf[0] = QBBS_CONT_ACCEPT;
        buf[1] = qserverno + 1;
        *reason = '\0';
        if(sendtouser(waitlist->auth.type, buf))
        {
            loginfo("<qbbscontrol> %s Authorized from QuakeWait", waitlist->auth.name);
            authlist_add(&(waitlist->auth), 1);    /* Add user to authlist */
            waitlist_deltop();
            return(1);
        }
        else
        {
            loginfo("<qbbscontrol> %s no longer a valid player, removing from QuakeWait", waitlist->auth.name);

            waitlist_deltop();
        }
    }
    /*  No valid players left on list  */
    return(0);
}
