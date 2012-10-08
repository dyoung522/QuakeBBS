#ifndef QUAKEBBSH
#define QUAKEBBSH

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <netinet/in.h>

typedef unsigned char byte;
extern int errno;
char *pname;

#define writeline(fd, ptr)   writen((fd), (ptr), (strlen(ptr)))

#define QBBS_VERSION            ".99B6.2 (Linux Beta)"

#define QBBSPASSWDSALT          "Qb"
#define QBBSLOCKFILE            "/var/run/quakebbs.pid"

#define QBBS_FILE_NOCONNECTUDP  "noconnect.udp"
#define QBBS_FILE_PRELOGIN      "prelogin.mesg"
#define QBBS_FILE_WELCOME       "welcome.mesg"
#define QBBS_FILE_HELP          "quakebbs.help"
#define QBBS_FILE_BULLETIN      "quakebbs.bulletin"
#define QBBS_FILE_SYSOPHELP     "quakebbs.sysop.help"
#define QBBS_FILE_QUAKEAUTH     "quakeauth.mesg"
#define QBBS_FILE_SYSINFO       "system.info"

#define QBBS_PROT_VER           0x01    /* Our current protocol version     */

#define QBBSIDLE                3600    /* Allowable idle time in secs      */

#define QBBS_LOGIN_WAIT         15      /* secs to wait for login packet    */
#define QBBS_LOGIN_TIMEOUT      15      /* login timeout in secs            */
#define QBBS_LOGIN_TRIES        3       /* login tries before disconnect    */

#define QBBS_CONTROL_WAIT       300     /* wait for client connect (secs)   */
#define QBBS_CONT_ACCEPT        0x01    /* Quake Authorization Accepted     */
#define QBBS_CONT_RFULL         0x02    /* Quake Authorization Rejected     */
#define QBBS_CONT_RAUTH         0x03    /* Quake Authorization Rejected     */
#define QBBS_CONT_RWAIT         0x04    /* Quake Authorization Rejected     */
#define QBBS_CONT_REMOVE        0x05    /* Quake Authorization Removed      */
#define QBBS_CONT_KICK          0x06    /* kicked from Quake Authorization  */
#define QBBS_CONT_LIST          0x07    /* Quake player list packet         */
#define QBBS_CONT_DOWN          0x08    /* Quake Server is Unavailable      */

#define QBBS_MAXPLAYER          16      /* Maximum number of Quake Players  */

#define QBBS_SOCKET_RETRY       10      /* Number of open tries             */
#define QBBS_SOCKET_WAIT        10      /* Secs to wait between tries       */

#define QBBSNAMELEN             26
#define QBBSADDRLEN             16
#define QBBSFILELEN             256
#define QBBSSHORTLEN            56
#define QBBSDESCLEN             128
#define QBBSSTATLEN             56

#define QBBSMAXBUF              1024
#define QBBSMAXINPUT            512
#define QBBSMAXPKT              1032

/*
 *  Call Accept and Reject codes
 */

#define QBBS_REJECT             0x01    /* Generic rejection                */
#define QBBS_REJECT_INVAL       0x02    /* Invalid username                 */
#define QBBS_REJECT_DUP         0x03    /* Duplicate username as newuser    */
#define QBBS_REJECT_UNKNO       0x04    /* Unknown username                 */
#define QBBS_REJECT_UNVAL       0x05    /* User unvalidated                 */
#define QBBS_REJECT_LOCK        0x06    /* User is locked out               */
#define QBBS_REJECT_BAN         0x07    /* User is banned                   */
#define QBBS_REJECT_PASS        0x08    /* Wrong password supplied          */
#define QBBS_REJECT_TIME        0x09    /* Timeout on login                 */

#define QBBS_ACCEPT             0x01    /* Generic accept                   */
#define QBBS_ACCEPT_NEW         0x02    /* New user                         */

#define QBBS_AUTH_DEL           0x01    /* Add to authorized list           */
#define QBBS_AUTH_ADD           0x02    /* Del from authorized list         */
#define QBBS_AUTH_KICK          0x03    /* Kick from authorized list        */
#define QBBS_AUTH_LIST          0x04    /* Request for player listing       */

#define QBBS_CHAT_DEL           0x01    /* Add to chat list                 */
#define QBBS_CHAT_ADD           0x02    /* Del from chat list               */
#define QBBS_CHAT_CHANGE        0x03    /* Change user status               */
#define QBBS_CHAT_BCAST         0x04    /* Broadcast to chat list           */

#define MAXQUAKESERVER          15

#define QBBSMAXSYS              4
#define QBBSMAXMISC             1
#define QBBSMAXDIRS             6
#define QBBSMAXFILE             5
#define QBBSMAXSERV             1

#define MAXMESGDATA             (4096-16)
#define MESGHDRSIZE             (sizeof(Mesg) - MAXMESGDATA)
#define QBBS_MSGQ_MKEY          666L
#define QBBS_MSGQ_PERM          0660

/*
 * id's defs
 */

#define CCREQ_CONNECT           0x01
#define CCREQ_SERVER_INFO       0x02
#define CCREQ_PLAYER_INFO       0x03
#define CCREQ_RULE_INFO         0x04

#define NET_PROTOCOL_VERSION    0x03
#define CONTROL_HEADER          0x08

#define CCREP_ACCEPT            0x81
#define CCREP_REJECT            0x82
#define CCREP_SERVER_INFO       0x83
#define CCREP_PLAYER_INFO       0x84
#define CCREP_RULE_INFO         0x85

/*
 *  Common Structures
 */

typedef struct {
  int mesg_len;                 /* #bytes in mesg_data, can be 0 or > 0     */
  long mesg_type;               /* message type, must be > 0                */
  char mesg_data[MAXMESGDATA];
} Mesg;

struct qbbs_server
{
    unsigned long addr;
    unsigned int port[2];
};

struct qbbs_quake_server
{
    char name[QBBSNAMELEN];
    unsigned long addr;
    unsigned int port;
    char qshort[QBBSSHORTLEN];
    char desc[QBBSDESCLEN];
};

struct qbbs_auth_list
{
    u_char command;                 /*  QBBSAUTHADD or QBBSAUTHDEL          */
    char name[QBBSNAMELEN];         /*  Player name                         */
    char addr[QBBSADDRLEN];         /*  Player IP                           */
    long type;                      /*  The clients mesg_type               */
    unsigned long tallow;           /*  Time allowed in server              */
};

struct qbbs_chat_list
{
    u_char command;                 /*  Add, Del, or Broadcast              */
    char name[QBBSNAMELEN];         /*  User name                           */
    long type;                      /*  The clients mesg_type               */
    char data[QBBSMAXBUF];          /*  Data                                */
};

struct qbbs_config
{
    char sys[QBBSMAXSYS][256];      /*  0 = qbbs_sys_name                   */
                                    /*  1 = qbbs_sys_sysop                  */
                                    /*  2 = qbbs_sys_email                  */
                                    /*  3 = qbbs_sys_url                    */

    int misc[QBBSMAXMISC];          /*  0 = qbbs_control_wait               */

    char dirs[QBBSMAXDIRS][256];    /*  0 = qbbs_home_dir                   */
                                    /*  1 = qbbs_data_dir                   */
                                    /*  2 = qbbs_text_dir                   */
                                    /*  3 = qbbs_logs_dir                   */
                                    /*  4 = qbbs_mesg_dir                   */
                                    /*  5 = qbbs_file_dir                   */

    char file[QBBSMAXFILE][256];    /*  0 = qbbs_conf_file                  */
                                    /*  1 = qbbs_user_file                  */
                                    /*  2 = quake_server_file               */
                                    /*  3 = qbbs_string_file                */
                                    /*  4 = qbbs_log_file                   */

    struct qbbs_server 
            serv[QBBSMAXSERV];      /*  0 = Main server                     */

    struct
    {
        unsigned conlog     : 1;    /*  Log to terminal                     */
        unsigned syslog     : 1;    /*  Set to use syslog                   */
        unsigned logfile    : 1;    /*  Set to use file logging             */
        unsigned forcename  : 1;    /*  If set, qbbscontrol will check name
                                        as well as IP                       */
    } opts;

};

#endif /* QUAKEBBSH */

