/*
 *  QuakeBBS Login Routines
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
#include <arpa/telnet.h>
#include <setjmp.h>
#include <varargs.h>

void getchatpkt(void);
void qbbs_logoff(void);

int userno;
int sock;
int kicksent = 0;

struct {
    time_t time;
    u_int counter;
} quake_attempts;

struct qbbs_user user;
struct qbbs_user_status ustatus;
extern struct qbbs_config config;
extern struct qbbs_quake_server qserver[MAXQUAKESERVER];
extern int msqid;

jmp_buf jmpenv;

int quakebbs_login(socket)
int socket;
{
    int n,
        retry,
        iacreply;
    char uname[QBBSUSERUNAME],
         *unamep,
         passwd[QBBSUSERPASSWD],
         *passwdp,
         filename[QBBSFILELEN],
         buf[QBBSMAXBUF];
    byte echo_on[3] = { IAC, WONT, TELOPT_ECHO },
         echo_off[3]  = { IAC, WILL, TELOPT_ECHO },
         echo_rep[3];

    sock = socket;

    /*  Tell TELNET client to echo locally  */

    send(sock, echo_on, 3, 0);

    /*  Send copyright notice  */

    writeuser("\r\nQuakeBBS version %s (c) 1996 Donovan Young\r\n", QBBS_VERSION);

    /*  Display prelogin.mesg to user */

    sprintf(filename, "%s/%s", config.dirs[2], QBBS_FILE_PRELOGIN);

    displayfile(filename);

    for(retry=0;retry < QBBS_LOGIN_TRIES;retry++)
    {
        bzero(buf, sizeof(buf));
        bzero(uname, sizeof(uname));

        if((n = getline("Username: ", uname, sizeof(uname), QBBS_LOGIN_TIMEOUT)) == -1)
            logerr("<qbbslogin> getline error");

        if(!n)
            continue;

        if(n == -2)
            logquit("<qbbslogin> login timeout");

        if(iacreply = ((u_char)uname[0] == IAC))
            unamep = &uname[3];
        else
            unamep = uname;

        send(sock, echo_off, 3, 0);

        bzero(buf, sizeof(buf));
        bzero(passwd, sizeof(passwd));

        if((n = getline("Password: ", buf, sizeof(buf), QBBS_LOGIN_TIMEOUT)) == -1)
            logerr("<qbbslogin> getline error");

        if(n == -2)
            logquit("<qbbslogin> login timeout");

        if(iacreply = ((u_char)buf[0] == IAC))
            passwdp = &buf[3];
        else
            passwdp = buf;

        send(sock, echo_on, 3, 0);

        /*  If we got reply before assume we'll get a reply again */

        if(iacreply)
            recv(sock, echo_rep, 3, 0);

        writeline(sock, "\r\n");    /*  Send a newline after password prompt  */

        strcpy(passwd, crypt(passwdp, QBBSPASSWDSALT));

        if(get_user(unamep, passwd))
            quakebbs_user();
    }

    return(0);
}

get_user(uname, passwd)
char *uname;
char *passwd;
{
    register int n, retry;
    char luname[256],
         *ptr;

    userno = 0;

    for(n = 0; uname[n] != '\0'; n++)
        if(!(isalnum(uname[n])) && uname[n] != '.')
            return(reject_user(QBBS_REJECT_INVAL, uname));

    strlow(luname, uname);

    ptr = &luname[strlen(luname)-4];

    if((memcmp(ptr, ".new", 4) == 0) || (strcmp(luname, "new") == 0))
    {
        /*  Read in first user record -- new user record  */

        userread(1);

        if(strcmp(luname, "new") != 0)
        {
            memcpy(user.uname, uname, strlen(uname)-4);
        }
        else
        {
            for(retry = 1; retry == 1;)
            {
                if((n = getline("Enter the Username you wish to use: ", user.uname, sizeof(user.uname), QBBS_LOGIN_TIMEOUT*2)) == -1)
                    logerr("<qbbslogin> getline error");
                if(n == -2)
                    logquit("<qbbslogin> login timeout");
                if(retry > 3)
                    logquit("<qbbslogin> New user didn't supply username");
                if(!n)
                    writeuser("You MUST supply a Username");
                else
                    retry = 0;
            }
        }

        if(finduser(user.uname))
            return(reject_user(QBBS_REJECT_DUP, user.uname));

        memcpy(user.passwd, passwd, strlen(passwd)+1);

        for(retry = 1; retry == 1;)
        {
            if((n = getline("Real Name: ", user.realname, sizeof(user.realname), QBBS_LOGIN_TIMEOUT*2)) == -1)
                logerr("<qbbslogin> getline error");
            if(n == -2)
                logquit("<qbbslogin> login timeout");
            if(retry > 3)
                logquit("<qbbslogin> New user didn't supply real name");
            if(!n)
                writeuser("You MUST supply a Real Name");
            else
                retry = 0;
        }

        if((n = getline("Email Address: ", user.email, sizeof(user.email), QBBS_LOGIN_TIMEOUT*2)) == -1)
            logerr("<qbbslogin> getline error");
        if(n == -2)
            logquit("<qbbslogin> login timeout");
        if(!n)
            strcpy(user.email, "None Given");

        if((n = getline("WWW URL: ", user.url, sizeof(user.url), QBBS_LOGIN_TIMEOUT*2)) == -1)
            logerr("<qbbslogin> getline error");
        if(n == -2)
            logquit("<qbbslogin> login timeout");
        if(!n)
            strcpy(user.url, "None Given");

        if((n = getline("Voice Phone: ", user.phone, sizeof(user.phone), QBBS_LOGIN_TIMEOUT*2)) == -1)
            logerr("<qbbslogin> getline error");
        if(n == -2)
            logquit("<qbbslogin> login timeout");
        if(!n)
            strcpy(user.phone, "---");

        time(&user.login);

        userwrite(0);
        return(accept_user(QBBS_ACCEPT_NEW));
    }

    if(!(userno = finduser(uname)))
        return(reject_user(QBBS_REJECT_UNKNO, uname));

    userread(userno);

    if(strcmp(passwd, user.passwd) == 0)
    {
        if(!user.sec_flags.validated)        /* Account not validated */
            return(reject_user(QBBS_REJECT_UNVAL, user.uname));

        if(user.sec_flags.locked)            /* Account locked */
            return(reject_user(QBBS_REJECT_LOCK, user.uname));

        if(user.sec_flags.banned)            /* Account Banned */
            return(reject_user(QBBS_REJECT_BAN, user.uname));

        /* They're OK... let 'em in! */

        return(accept_user(QBBS_ACCEPT));

    }
    else    /* Passwd doesn't match */
        return(reject_user(QBBS_REJECT_PASS, user.uname));

    /* should never execute */
    return(reject_user(QBBS_REJECT, user.uname));
}

accept_user(code)
byte code;
{
    int clilen;
    char filename[QBBSFILELEN];
    struct sockaddr_in cliaddr;

    clilen = sizeof(cliaddr);

    sprintf(filename, "%s/%s", config.dirs[2], QBBS_FILE_WELCOME);

    displayfile(filename);

    if(userno)
    {
        time(&user.login);
        userwrite(userno);
    }

    if(getpeername(sock, (struct sockaddr *)&cliaddr, &clilen) < 0)
        logsyserr("<qbbslogin> getpeername");

    bzero(ustatus, sizeof(ustatus));
    ustatus.addr = cliaddr.sin_addr.s_addr;

    if(code == QBBS_ACCEPT_NEW)
    {
        ustatus.flags.new = 1;
        loginfo("<qbbslogin> New user (%s @ %s)", user.uname, inet_ntoa(ustatus.addr));
    }
    else
        loginfo("<qbbslogin> login accepted from %s (%s)", 
                 inet_ntoa(ustatus.addr), user.uname);

    return(1);
}

reject_user(code, username)
byte code;
char *username;
{
    char msg[256];

    switch(code)
    {
        case QBBS_REJECT_INVAL:
            strcpy(msg, "Invalid Username");
            break;
        case QBBS_REJECT_DUP:
            strcpy(msg, "Username already in use");
            break;
        case QBBS_REJECT_UNKNO:
            strcpy(msg, "Username not found in database");
            break;
        case QBBS_REJECT_UNVAL:
            strcpy(msg, "User unvalidated");
            break;
        case QBBS_REJECT_LOCK:
            strcpy(msg, "Account LOCKED");
            break;
        case QBBS_REJECT_BAN:
            strcpy(msg, "Account BANNED");
            break;
        case QBBS_REJECT_PASS:
            strcpy(msg, "Incorrect password");
            break;
        case QBBS_REJECT_TIME:
            strcpy(msg, "Login timeout");
            break;
        default:
            strcpy(msg, "Login rejected (reason unknown)");
            break;
    }

    writeuser(msg);

    if(username != NULL)
        loginfo("<qbbslogin> login rejected (%s): %s", username, msg);
    else
        loginfo("<qbbslogin> login rejected: %s", msg);

    return(0);
}

strlow(newbuf, buf)
char *newbuf;
char *buf;
{
    while(*buf != '\0')
        *newbuf++ = tolower(*buf++);

    *newbuf = '\0';

    return;
}

userwrite(userno)
int userno;
{
    int fuser;

    if((fuser = openuser(O_RDWR)) < 0)
        logsyserr("<qbbslogin> could not open userfile");

    lseek(fuser, 0L, 0);
    if(lockf(fuser, F_LOCK, 0L) < 0)
        logsyserr("<qbbslogin> can't lock userfile");

    if(userno > 0)
    {
        /*  Seek to just before userno */
        if(lseek(fuser, (long)((sizeof(user)*userno)-sizeof(user)), SEEK_SET) < 0)
            logsyserr("<qbbslogin> seek error");
    }
    else    /* 0 = New Record */
        if(lseek(fuser, 0L, SEEK_END) < 0)
            logsyserr("<qbbslogin> seek error");

    if(write(fuser, &user, sizeof(user)) < 0)
        logsyserr("<qbbslogin> write error on userfile");

    lseek(fuser, 0L, 0);
    if(lockf(fuser, F_ULOCK, 0L) < 0)
        logsyserr("<qbbslogin> can't unlock userfile");

    close(fuser);
    return;
}

userread(userno)
int userno;
{
    int status = 0,
        fuser;

    if((fuser = openuser(O_RDONLY)) < 0)
        logsyserr("<qbbslogin> could not open userfile");

    /*  Seek to just before userno */
    lseek(fuser, (long)((sizeof(user)*userno)-sizeof(user)), SEEK_SET);

    if((status = read(fuser, &user, sizeof(user))) < 0)
        logsyserr("<qbbslogin> read error on userfile");

    close(fuser);
    return(status);
}

finduser(username)
char *username;
{
    int userno = 0,
        status = 0,
        fuser;
    struct qbbs_user tuser;
    char luname[256],
         lusername[256];

    if((fuser = openuser(O_RDONLY)) < 0)
        logsyserr("<qbbslogin> could not open userfile");

    strlow(lusername, username);

    while((status = read(fuser, &tuser, sizeof(tuser))) > 0)
    {
        userno++;
        strlow(luname, tuser.uname);
        if(memcmp(luname, lusername, strlen(lusername)) == 0)
        {
            close(fuser);
            return(userno);
        }
    }
    if(status < 0)
        logsyserr("<qbbslogin> read error on userfile");

    close(fuser);
    return(0);
}

openuser(mode)
int mode;
{
    int fuser;

    if((fuser = open(config.file[1], mode)) < 0)
        logsyserr("<qbbslogin> error opening %s", config.file[1]);

    return(fuser);
}

/*
 * Write "n" bytes to a descriptor.
 * Use in place of write() when fd is a stream socket.
 */

int writen(fd, ptr, nbytes)
register int    fd;
register char    *ptr;
register int    nbytes;
{
    int    nleft, nwritten;

    nleft = nbytes;
    while (nleft > 0) {
        if((nwritten = write(fd, ptr, nleft)) <= 0)
            return(nwritten);        /* error */

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(nbytes - nleft);
}

getline(prompt, buf, len, timeout)
char *prompt;
char *buf;
int len;
int timeout;
{
    int n;
    struct timeval tv;
    fd_set fdmask;

    if(*prompt)
        writeline(sock, prompt);

    FD_ZERO(&fdmask);
    FD_SET(sock, &fdmask);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    if((n = select(sock+1, &fdmask, (fd_set *)0, (fd_set *)0, &tv)) < 0)
    {
        logerr("<qbbslogin> select error");
        return(-1);
    }

    if(!n)  /* timeout */
        return(-2);

    if((n = sreadline(buf, len)) < 0)
    {
        logerr("<qbbslogin> sreadline error");
        return(-1);
    }

    if(!n)
        qbbs_disconnect();

    return(strlen(buf));
}

int quakebbs_user()
{
    int status;
    static char buf[QBBSMAXBUF] = { 0 };

    sendchatpkt(QBBS_CHAT_ADD, "In Chat");

    ustatus.flags.login = 1;

    for(;;)
    {
        setjmp(jmpenv);

        signal(SIGHUP, (void *)qbbs_logoff);
        signal(SIGTERM, (void *)qbbs_logoff);
        signal(SIGUSR1, (void *)getchatpkt);

        signal(SIGIO, SIG_IGN);
        signal(SIGURG, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);

        sigsetmask(0);

        if((status = sreadline(&buf, QBBSMAXINPUT)) < 0)
        {
            logerr("<qbbslogin> sreadline error");
            return(-1);
        }

        buf[status] = '\0';

        if(strlen(buf) > QBBSMAXINPUT)
            buf[QBBSMAXINPUT] = '\0';

        sigblock(sigmask(SIGUSR1));

        if((status != 0) && (strlen(buf) != 0))
        {
            sendchatpkt(QBBS_CHAT_BCAST, buf);
            buf[0] = '\0';
        }
        else if(status == 0)
            qbbs_disconnect();

    }

    qbbs_logoff();
}

void getchatpkt()
{
    int n;
    u_char servno;
    struct
    {
        long mtype;
        char msg[QBBSMAXBUF];
    } mesg;
    char filename[QBBSFILELEN],
         buf[QBBSMAXBUF],
         *reason = &mesg.msg[2];

    mesg.mtype = getpid();

    while((n = msgrcv(msqid, (struct msgbuf *)&mesg, QBBSMAXBUF-1, mesg.mtype, IPC_NOWAIT | MSG_NOERROR)) > 0)
    {
        /* reason[n] = '\0'; */
        switch(mesg.msg[0])
        {
            case QBBS_CONT_ACCEPT:
                servno = (mesg.msg[1] - 1);
                writeuser("<< Authorization accepted on %s >>", qserver[servno].name);
                writeuser("*** Start your Quake Client using \"+connect %s -udpport %d\"", inet_ntoa(qserver[servno].addr), qserver[servno].port);

                sendchatpkt(QBBS_CHAT_CHANGE, "In Quake!");
                sprintf(filename, "%s/%s", config.dirs[2], QBBS_FILE_QUAKEAUTH);
                displayfile(filename);
                ustatus.flags.quake = 1;
                ustatus.flags.qauth = 1;
                break;

            case QBBS_CONT_RFULL:
                writeuser("<< Quake Server Currently Full -- Adding you to QuakeWait >>");
                sendchatpkt(QBBS_CHAT_CHANGE, "In Chat <On QuakeWait List>");
                ustatus.flags.quake = 1;
                ustatus.flags.qauth = 0;
                break;

            case QBBS_CONT_REMOVE:
                writeuser("<< Quake Authorization Revoked (%s) >>", reason);
                sendchatpkt(QBBS_CHAT_CHANGE, "In Chat");
                ustatus.flags.quake = 0;
                ustatus.flags.qauth = 0;
                break;

            case QBBS_CONT_KICK:
                if(!kicksent)
                {
                    sendchatpkt(QBBS_CHAT_CHANGE, "In Chat");
                    ustatus.flags.quake = 0;
                    ustatus.flags.qauth = 0;
                    sprintf(buf, "<< Quake Authorization Revoked (Kicked) >>");
                }
                else
                {
                    kicksent = 0;
                    sprintf(buf, "<< %s >>", reason);
                }
                writeuser(buf);
                break;

            case QBBS_CONT_RWAIT:
                sendchatpkt(QBBS_CHAT_CHANGE, "In Chat <On QuakeWait List>");
                ustatus.flags.quake = 1;
                ustatus.flags.qauth = 0;
                writeuser("<< %s >>", reason);
                break;

            case QBBS_CONT_RAUTH:
                servno = (mesg.msg[1] - 1);
                sendchatpkt(QBBS_CHAT_CHANGE, "In Quake!");
                ustatus.flags.quake = 1;
                ustatus.flags.qauth = 1;
                writeuser("<< %s >>", reason);
                writeuser("Start your Quake Client using \"+connect %s -udpport %d\"", inet_ntoa(qserver[servno].addr), qserver[servno].port);
                break;

            case QBBS_CONT_DOWN:
                servno = (mesg.msg[1] - 1);
                sendchatpkt(QBBS_CHAT_CHANGE, "In Chat");
                ustatus.flags.quake = 0;
                ustatus.flags.qauth = 0;
                writeuser("<< Quake Authorization Denied (%s) >>", reason);
                break;

            case QBBS_CONT_LIST:
                writeuser(reason);
                break;

            default:
                writeuser(mesg.msg);
                break;
        }
    }

    if(n < 0 && errno != ENOMSG && errno != EINTR)
        logsyserr("<qbbslogin> msgrcv error");
    
    longjmp(jmpenv, 0);
}

sendchatpkt(command, data)
byte command;
char *data;
{
    int status;
    struct
    {
        long mtype;
        struct qbbs_chat_list list;
    } mesg;

    /*  Locally handled commands  */

    if(memcmp(data, "/quake", 4) == 0)
    {
        qbbs_quake();
        return;
    }
    if(memcmp(data, "/kick", 4) == 0)
    {
        if(!user.sec_flags.sysop)
        {
            loginfo("<qbbslogin> %s attempted privledged command \"%s\"", 
                     user.uname, data);
            writeuser("Sorry, you do not have sufficient access for this command.");
            return;
        }
    }
    if(memcmp(data, "/qkick", 4) == 0)
    {
        if(!user.sec_flags.sysop)
        {
            loginfo("<qbbslogin> %s attempted privledged command \"%s\"", 
                     user.uname, data);
            writeuser("Sorry, you do not have sufficient access for this command.");
            return;
        }
        else
        {
            qbbs_qkick(data);
            return;
        }
    }
    if(memcmp(data, "/ver", 4) == 0)
    {
        writeuser("\r\nQuakeBBS version %s\r\nCopyright (c) 1996 Donovan Young\r\nAll Rights Reserved\r\n", QBBS_VERSION);
        writeuser("With Special Thanks to Sean Allen, Rick Owen, and Justin Hamilton\r\nThanks Guys!\r\n");
        return;
    }
    if(memcmp(data, "/help", 4) == 0 || memcmp(data, "/?", 2) == 0)
    {
        qbbs_help(data);
        return;
    }
    if(memcmp(data, "/bulletin", 4) == 0)
    {
        qbbs_bulletin(data);
        return;
    }
    if(memcmp(data, "/passw", 4) == 0)
    {
        qbbs_passwd(data);
        return;
    }
    if(memcmp(data, "/sys", 4) == 0)
    {
        qbbs_system();
        return;
    }
    if(memcmp(data, "/slist", 4) == 0)
    {
        qbbs_slist();
        return;
    }
    if(memcmp(data, "/qlist", 4) == 0)
    {
        sendcontrolpkt(QBBS_AUTH_LIST);
        return;
    }
    if(memcmp(data, "/quit", 4) == 0)
        qbbs_quit();

    mesg.mtype = 1L;
    mesg.list.command = command;
    strcpy(mesg.list.name, user.uname);
    mesg.list.type = (long)getpid();
    strcpy(mesg.list.data, data);

#ifdef TRACE
    logdebug("<qbbslogin> sending type %ld mesg", mesg.mtype);
    logdebug("            list.command = 0x%02x", mesg.list.command);
    logdebug("            list.name    = \"%s\"", mesg.list.name);
    logdebug("            list.type    = %ld", mesg.list.type);
    logdebug("            data         = \"%s\"", mesg.list.data);
#endif

/*
    if(strlen(mesg.list.data) > QBBSMAXINPUT)
        mesg.list.data[QBBSMAXINPUT] = '\0';
*/

    status = msgsnd(msqid, (struct msgbuf *)&mesg, sizeof(struct qbbs_chat_list), IPC_NOWAIT);
    if(status < 0)
    {
        if(errno == EAGAIN)
        {
            loginfo("<qbbslogin> message queue full condition occurred");
            writeuser("<< Message Queue full, please resend data >>");
        }
        else
            logerr("<qbbslogin> chatpkt msgsnd error");
    }
    /* qbbs_wait(1000); */

    return;
}

qbbs_passwd(arg)
char *arg;
{
    char *passwd;

    while(*arg != '\0' && !isspace(*arg))
        arg++;

    while(isspace(*arg))
        arg++;

    if(*arg == '\0')
    {
        writeuser("Syntax:  /passwd <newpasswd>");
        return;
    }

    passwd = arg;

    while(!isspace(*arg))
        arg++;

    *arg = '\0';

    /* *(passwd+(arg - passwd)) = '\0'; */

    if(*passwd != '\0')
    {
        strcpy(user.passwd, crypt(passwd, QBBSPASSWDSALT));
        userwrite(userno);
        writeuser("*** Password has been successfully changed");
        loginfo("<qbbslogin> %s changed password", user.uname);
    }
    return;
}
    
qbbs_help(arg)
char *arg;
{
    char *topic;
    char filename[QBBSFILELEN];

    while(*arg != '\0' && !isspace(*arg))
        arg++;

    while(isspace(*arg))
        arg++;

    topic = arg;

    if(*topic == '\0')
    {
        sprintf(filename, "%s/%s", config.dirs[2], QBBS_FILE_HELP);
        displayfile(filename);

        if(user.sec_flags.sysop)
        {
            sprintf(filename, "%s/%s", config.dirs[2], QBBS_FILE_SYSOPHELP);
            displayfile(filename);
        }
    }
    else
    {
        sprintf(filename, "%s/%s.help", config.dirs[2], topic);
        if(!displayfile(filename))
            writeuser("Sorry, no help available for %s, try /help with no topic for a list", topic);
    }

    return;
}

qbbs_bulletin(arg)
char *arg;
{
    char *number;
    char filename[QBBSFILELEN];

    while(*arg != '\0' && !isspace(*arg))
        arg++;

    while(isspace(*arg))
        arg++;

    number = arg;

    if(*number == '\0')
    {
        sprintf(filename, "%s/%s", config.dirs[2], QBBS_FILE_BULLETIN);
        displayfile(filename);
    }
    else
    {
        sprintf(filename, "%s/%s.bulletin", config.dirs[2], number);
        if(!displayfile(filename))
            writeuser("Sorry, bulletin %s not available, try /bulletin with no number for a list", number);
    }

    return;
}

qbbs_system()
{
    char filename[QBBSFILELEN];

    writeuser("\r\nSystem Name:  %s", config.sys[0]);
    writeuser("SysOp:        %s", config.sys[1]);
    writeuser("Email Addr:   %s", config.sys[2]);
    writeuser("WWW URL:      %s", config.sys[3]);

    writeuser("\r\nRunning QuakeBBS version %s\r\n", QBBS_VERSION);

    sprintf(filename, "%s/%s", config.dirs[2], QBBS_FILE_SYSINFO);
    displayfile(filename);

    return;
}

qbbs_slist()
{
    int x;

    writeuser("\r\n    #    Server Name               Description");
    writeuser("    ~~   ~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    for(x=0; qserver[x].name[0] != '\0'; x++)
        writeuser("    %-2d   %-25.25s %s", x+1, qserver[x].name, qserver[x].desc);

    return;
}

qbbs_quit()
{
    writeuser("Goodbye");
    qbbs_logoff();
}

void qbbs_logoff()
{
    if(!ustatus.flags.login)
    {
        close(sock);
        exit(0);
    }

    sendchatpkt(QBBS_CHAT_DEL, "");

    loginfo("<qbbslogin> (%s) logged off", user.uname);

/*
    if(ustatus.flags.quake)
        sendcontrolpkt(QBBS_AUTH_DEL);
*/

    close(sock);
    exit(0);
}

qbbs_disconnect()
{
    if(ustatus.flags.login)
    {
        logerr("<qbbslogin> User %s disconnected", user.uname);
        qbbs_logoff();
    }
    close(sock);
    exit(1);
}

int sreadline(ptr, maxlen)
char *ptr;
int maxlen;
{
    register int rc,
                 n = 1,
                 x = 0;
    static int flag;
    char c;

    if((flag == 1) && (strlen(ptr) != 0))    /*  We were interrupted  */
    {
        n = strlen(ptr);
        ptr += n;
    }
    else
        memset(ptr, NULL, sizeof(&ptr)); /*  Insure we have a clean buffer  */
    
    for(flag = 1; n < maxlen; n++)
    {
        *ptr = 0;
        if((rc = read(sock, &c, 1)) == 1)
        {
            if(isprint(c) == 0)
            {
                if(c == '\r')
                {
                    *ptr = '\0';
                    flag = 0;
                    break;
                }
                if(c == 0x08 && n > 0)
                    --ptr;
            }
            else
                *(ptr++) = c;
        }
        else if(rc == 0)
        {
            if(n == 1)
                n = 0;        /* EOF, no data read */

            break;
        }
        else
            n = -1;            /* error */
    }

    *ptr = '\0';
    
    return(n);
}

qbbs_quake()
{
    static time_t lasttime;

    if(!user.sec_flags.quake)
    {
        writeuser("Sorry.  You have not yet been authorized to play Quake here.");
        return;
    }

    if(difftime(time(NULL), lasttime) < config.misc[0])
    {
        loginfo("<qbbslogin> excess /quake commands sent by %s", user.uname);
        writeuser("<< Please wait for a reply first >>");
        return;
    }

    time(&lasttime);

    if(!ustatus.flags.quake)
    {
        ustatus.flags.quake = 1;
        sendcontrolpkt(QBBS_AUTH_ADD);
    }
    else
    {
        ustatus.flags.quake = 0;
        sendcontrolpkt(QBBS_AUTH_DEL);
        if(ustatus.flags.qauth)
            writeuser("Removing you from Quake Authorization List");
        else
            writeuser("Removing you from QuakeWait Queue");
    }

    return;
}

qbbs_qkick(arg)
char *arg;
{
    int status;
    struct
    {
        long mtype;
        struct qbbs_auth_list list;
    } mesg;

    while(*arg != '\0' && !isspace(*arg))
        arg++;

    while(isspace(*arg))
        arg++;

    if(*arg != '\0')
    {
        mesg.mtype = 2L;
        mesg.list.command = QBBS_AUTH_KICK;
        strcpy(mesg.list.name, arg);
        *mesg.list.addr = '\0';
        mesg.list.type = getpid();
        mesg.list.tallow = 0L;

        status = msgsnd(msqid, (struct msgbuf *)&mesg, sizeof(struct qbbs_auth_list), IPC_NOWAIT);
        if(status < 0)
        {
            if(errno == EAGAIN)
            {
                loginfo("<qbbslogin> message queue full condition occurred");
                writeuser("<< Message Queue full, please resend data >>");
            }
            else
                logerr("<qbbslogin> chatpkt msgsnd error");
        }
        else
            kicksent = 1;

    }
    else
        writeuser("Syntax:  /qkick <username>");

    return;
}

sendcontrolpkt(command)
byte command;
{
    int status;
    struct
    {
        long mtype;
        struct qbbs_auth_list list;
    } mesg;

    mesg.mtype = 2L;
    mesg.list.command = command;
    strcpy(mesg.list.name, user.uname);
    strcpy(mesg.list.addr, (char *)inet_ntoa(ustatus.addr));
    mesg.list.type = getpid();
    mesg.list.tallow = 0L;

    status = msgsnd(msqid, (struct msgbuf *)&mesg, sizeof(struct qbbs_auth_list), IPC_NOWAIT);
    if(status < 0)
    {
        if(errno == EAGAIN)
        {
            loginfo("<qbbslogin> message queue full condition occurred");
            writeuser("<< Message Queue full, please resend data >>");
        }
        else
            logerr("<qbbslogin> chatpkt msgsnd error");
    }
    else if(command == QBBS_AUTH_ADD)
    {
        sendchatpkt(QBBS_CHAT_CHANGE, "Waiting for Quake Authorization");
        writeuser("Request for authorization has been sent, Please wait for reply");
    }

    return;
}

writeuser(va_alist)
va_dcl
{
    va_list args;
    char buf[QBBSMAXBUF],
         *fmt;

    va_start(args);
    fmt = va_arg(args, char *);
    vsprintf(buf, fmt, args);
    va_end(args);

    if(writeline(sock, buf) < 0)
        logerr("<qbbslogin> write error");
    if(writeline(sock, "\r\n") < 0)
        logerr("<qbbslogin> write error");

    return;
}

/*
writeuser(buf)
char *buf;
{
    if(writeline(sock, buf) < 0)
        logerr("<qbbslogin> write error");
    if(writeline(sock, "\r\n") < 0)
        logerr("<qbbslogin> write error");

    return;
}
*/

displayfile(filename)
char *filename;
{
    FILE *fp;
    char buf[QBBSMAXBUF];
    
    if((fp = fopen(filename, "r")) != NULL)
    {
        while(fgets(buf, sizeof(buf), fp) != (char *)NULL)
        {
            if(*(buf+(strlen(buf)-1)) == '\n')
                *(buf+(strlen(buf)-1)) = '\0';

            writeuser(buf);
        }
        fclose(fp);
        return(1);
    }
    return(0);
}
