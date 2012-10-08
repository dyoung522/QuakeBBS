/*
 *  QCHATd - Chat server module, Part of the QuakeBBS system
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
#include "qbbscommand.h"
#include <sys/param.h>

extern struct qbbs_config config;
extern struct qbbs_quake_server qserver[MAXQUAKESERVER];
extern int msqid;

void qbbs_chat_exit();

struct chat_list
{
    struct qbbs_chat_list chat;
    byte available;
    struct chat_list *next;
} *chatlist = NULL, *chatlist_top = NULL;

QBBSChat()
{
    signal(SIGINT, qbbs_chat_exit);
    signal(SIGTERM, qbbs_chat_exit);
    signal(SIGHUP, qbbs_chat_exit);

    for(;;)
    {
        build_chatlist();
        /* check_msgqueue(0); */

        qbbs_wait(500);
    }
}

void qbbs_chat_exit()
{
    int fd;
    struct chat_list *ptr;

    loginfo("<qbbschat> shutting down");

    for (fd = 0; fd < NOFILE; fd++)
        close(fd);

    chatlist = chatlist_top;

    while(chatlist != NULL)
    {
        ptr = chatlist->next;
        free(chatlist);
        chatlist = ptr;
    }

    exit(0);
}
    
build_chatlist()
{
    int n;
    struct
    {
        long mtype;
        struct qbbs_chat_list listdata;
    } mesg;

    mesg.mtype = 1L;

    while((n = msgrcv(msqid, (struct msgbuf *)&mesg, sizeof(struct qbbs_chat_list), mesg.mtype, IPC_NOWAIT)) > 0)
    {
        switch(mesg.listdata.command)
        {
            case QBBS_CHAT_ADD:
                chatlist_add(&mesg.listdata);        /* Add data to List */
                break;
            case QBBS_CHAT_DEL:
                chatlist_del(&mesg.listdata);        /* Add data to List */
                break;
            case QBBS_CHAT_CHANGE:
                chatlist_change(&mesg.listdata);    /* Add data to List */
                break;
            default:
                broadcast(&mesg.listdata);
                break;
        }
    }
    if(n < 0 && errno != EIDRM && errno != EINTR && errno != ENOMSG)
        logsyserr("<qbbschat> msgrcv error");

    return;
}

check_msgqueue(purge)
int purge;
{
    struct
    {
        long mtype;
        char msg[QBBSMAXBUF];
    } mesg;

    chatlist = chatlist_top;

    while(chatlist != NULL)
    {
        if(kill((pid_t)chatlist->chat.type, SIGUSR1) < 0)
        {
            mesg.mtype = chatlist->chat.type;
            if(mesg.mtype > 1)
                while(msgrcv(msqid, (struct msgbuf *)&mesg, QBBSMAXBUF, mesg.mtype, IPC_NOWAIT | MSG_NOERROR) > 0);
        }
        chatlist = chatlist->next;
    }
    
    if(purge)
    {
        loginfo("<qbbschat> purging message queue");
        mesg.mtype = 0L;
        msgrcv(msqid, (struct msgbuf *)&mesg, QBBSMAXBUF, mesg.mtype, IPC_NOWAIT | MSG_NOERROR);
    }

    return;
}

chatlist_add(data)
struct qbbs_chat_list *data;
{
    struct chat_list *new;
    char buf[QBBSMAXBUF];

    if(onlist(data))
        return;        /* User already on list */

    loginfo("<qbbschat> Adding %s to list", data->name);

    sprintf(buf, "<%s has joined>", data->name);
    sendtoother(data->type, buf);

    if((new = (struct chat_list *)malloc(sizeof(struct chat_list))) == NULL)
        logerr("<qbbschat> malloc error");

    strcpy(new->chat.name, data->name);
    new->chat.type = data->type;
    strcpy(new->chat.data, data->data);
    new->available = 1;
    new->next = NULL;

    chatlist = chatlist_top;

    if(chatlist == NULL)
    {
        chatlist = new;
        chatlist_top = chatlist;
    }
    else
    {
        while(chatlist->next != NULL)
            chatlist = chatlist->next;
        chatlist->next = new;
    }

    return;
}

onlist(data)
struct qbbs_chat_list *data;
{
    chatlist = chatlist_top;

    while((chatlist != NULL) && ((strcmp(chatlist->chat.name, data->name) != 0) 
                             || (chatlist->chat.type != data->type)))
        chatlist = chatlist->next;
    
    if(chatlist == NULL)
        return(0);    /* User not on list  */
    else
        return(1);    /* User already on list */
}

chatlist_del(data)
struct qbbs_chat_list *data;
{
    struct chat_list *ptr;
    char buf[QBBSMAXBUF];
    struct
    {
        long mtype;
        struct qbbs_chat_list listdata;
    } mesg;

    chatlist = chatlist_top;
    ptr = chatlist;

    if(chatlist == NULL)
        return;

    loginfo("<qbbschat> Deleting %s from list", data->name);

    if(chatlist->next == NULL)
    {
        mesg.mtype = data->type;
        if(mesg.mtype > 1)
            while(msgrcv(msqid, (struct msgbuf *)&mesg, sizeof(mesg.listdata), mesg.mtype, IPC_NOWAIT | MSG_NOERROR) > 0);

        free(chatlist);
        chatlist_top = NULL;
        return;
    }

    while((chatlist != NULL) && ((strcmp(chatlist->chat.name, data->name) != 0)
                             ||         (chatlist->chat.type != data->type)))
    {
        ptr = chatlist;
        chatlist = chatlist->next;
    }

    if(chatlist == NULL)
        return;
    else
    {
        if(chatlist_top == chatlist)
            chatlist_top = chatlist->next;
        else
            ptr->next = chatlist->next;

        mesg.mtype = data->type;
        if(mesg.mtype > 1)
            while(msgrcv(msqid, (struct msgbuf *)&mesg, sizeof(mesg.listdata), mesg.mtype, IPC_NOWAIT | MSG_NOERROR) > 0);

        free(chatlist);
    }

    sprintf(buf, "<%s has left>", data->name);
    sendtoother(data->type, buf);

    return;
}

chatlist_change(data)
struct qbbs_chat_list *data;
{
    if(!onlist(data))
        return(0);        /* user not on list */
    
    strcpy(chatlist->chat.data, data->data);

/*
    if((memcmp(chatlist->chat.data, "In Quake", 8) == 0) ||
       (strcmp(chatlist->chat.data, "Unavailable") == 0))
        chatlist->available = 0;
    else
        chatlist->available = 1;
*/
    
    return(1);
}

broadcast(data)
struct qbbs_chat_list *data;
{
    struct chat_list *ptr;
    char buf[QBBSMAXBUF];
    char *combuf;

    chatlist = chatlist_top;

    combuf = data->data;

#ifdef TRACE
    logdebug("<qbbschat> received message from type %ld", data->type);
    logdebug("           data = \"%s\"", data->data);
#endif

    if(*combuf == '/')        /*  It's a command  */
    {
        combuf++;
        if(runcommand(combuf, data->type) < 0)
        {
            loginfo("<qbbschat> (%s) attempted invalid command [%s]", data->name, combuf);
            sprintf(buf, "Invalid Command:  %s", combuf);

            sendtouser(data->type, buf);
        }
        return;
    }
    sprintf(buf, "[%s] %s", data->name, data->data);
    sendtoall(data->type, buf);
    return;
}

runcommand(command, type)
char *command;
long type;
{
    int x,
        status = -1;

    for(x = 0; qcommand[x].name != NULL; ++x)
    {
     /* if(memcmp(command, qcommand[x].name, strlen(qcommand[x].name)) == 0) */
        if(memcmp(command, qcommand[x].name, 3) == 0)
        {
#ifdef TRACE
            logdebug("<qbbschat> Running command %s", command);
#endif
            status = (*qcommand[x].function)(type, command);
        }
    }

    return(status);
}

sendtouser(type, buf)
long type;
char *buf;
{
    struct
    {
        long mtype;
        char msg[QBBSMAXBUF];
    } mesg;

    mesg.mtype = type;
    strcpy(mesg.msg, buf);

    if(type > 1)
    {
        if(kill((pid_t)type, 0) == 0)
        {

msgsnd:

#ifdef TRACE
    logdebug("<qbbschat> sending type %ld message", mesg.mtype);
    logdebug("           msg = \"%s\"", mesg.msg);
#endif

            if(msgsnd(msqid, (struct msgbuf *)&mesg, (strlen(mesg.msg)+1), IPC_NOWAIT) < 0)
            {
                if(errno == EAGAIN)
                {
                    loginfo("<qbbschat> message queue full");
                    /* check_msgqueue(1); */
                    sleep(1);
                    goto msgsnd;
                }
                else
                {
                    logerr("<qbbschat> msgsnd error");
                    return(0);
                }
            }

            kill((pid_t)type, SIGUSR1);
            qbbs_wait(500);

            return(1);
        }
    }

    return(0);
}

sendtoall(type, buf)
char *buf;
{
    chatlist = chatlist_top;

    while(chatlist != NULL)
    {
#ifdef DEBUG
        logdebug("<qbbschat> sending \"%s\" to %ld", buf, chatlist->chat.type);
#endif
        if(chatlist->available)
            sendtouser(chatlist->chat.type, buf);
        chatlist = chatlist->next;
    }
}

sendtoother(type, buf)
char *buf;
{
    chatlist = chatlist_top;

    while(chatlist != NULL)
    {
#ifdef DEBUG
        logdebug("<qbbschat> sending \"%s\" to %ld", buf, chatlist->chat.type);
#endif
        if(chatlist->chat.type != type)
            if(chatlist->available)
                sendtouser(chatlist->chat.type, buf);
        chatlist = chatlist->next;
    }
}

qbbs_who(type, arg)
long type;
char *arg;
{
    char buf[QBBSMAXBUF];

    chatlist = chatlist_top;

    sendtouser(type, "\r\nUsers Online:\r\n");

    while(chatlist != NULL)
    {
        sprintf(buf, "   %c%-25.25s (%s)\0", chatlist->available ? ' ' : '*',
                                             chatlist->chat.name,
                                             chatlist->chat.data);
        sendtouser(type, buf);
        chatlist = chatlist->next;
    }
    sendtouser(type, "\r\n* = Not receiving messages");

    return;
}

qbbs_chat(type, arg)
long type;
char *arg;
{
    char buf[QBBSMAXBUF];

    chatlist = chatlist_top;

    while(chatlist->chat.type != type && chatlist != NULL)
        chatlist = chatlist->next;

    if(chatlist == NULL)
        return;

    chatlist->available = (chatlist->available == 0);

    sprintf(buf, "Chat Messages now %s", chatlist->available ? "ON" : "OFF");
    sendtouser(type, buf);

    return;
}

qbbs_kick(type, arg)
long type;
char *arg;
{
    char buf[QBBSMAXBUF],
         *kicker, *kickee;

    if(list_findbytype(type))
        kicker = chatlist->chat.name;

    while(*arg != '\0' && !isspace(*arg))
        arg++;

    while(isspace(*arg))
        arg++;

    if(*arg != '\0')
    {
        if(list_findbyname(arg))
        {
            sprintf(buf, "*** You've been kicked by %s ***", kicker);
            sendtouser(chatlist->chat.type, buf);

            kickee = chatlist->chat.name;

            /*  Give some time for message to be delivered to user  */
            qbbs_wait(500);

            if(kill((pid_t)chatlist->chat.type, SIGTERM) != 0)
            {
                sprintf(buf, "Unable to kill %s", kickee);
                sendtouser(type, buf);
            }
            else
            {
                sprintf(buf, "<%s was kicked by %s>", kickee, kicker);
                sendtoall(type, buf);

                loginfo("<qbbschat> %s was kicked by %s", kickee, kicker);
            }
        }
        else
        {
            sprintf(buf, "%s not found on system", arg);
            sendtouser(type, buf);
        }
    }
    else
        sendtouser(type, "Syntax:  /kick <username>");

    return;
}

list_findbytype(type)
long type;
{
    chatlist = chatlist_top;

    while(chatlist != NULL)
    {
        if(chatlist->chat.type == type)
            return(1);
        chatlist = chatlist->next;
    }
    return(0);
}

list_findbyname(username)
char *username;
{
    chatlist = chatlist_top;

    while(chatlist != NULL)
    {
        if(strcmp(chatlist->chat.name, username) == 0)
            return(1);
        chatlist = chatlist->next;
    }
    return(0);
}
