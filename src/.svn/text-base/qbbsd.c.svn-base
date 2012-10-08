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

#include "quakebbs.h"
#include "qbbsuser.h"

extern struct qbbs_config config;
extern struct qbbs_quake_server qserver[MAXQUAKESERVER];

QBBSd()
{
	int	udp_sock,
		tcp_sock;
	fd_set fdmask;

	if((udp_sock = udp_open(1)) < 0)
		logsyserr("<getconnect> could not get UDP socket");

	if((tcp_sock = tcp_open(1)) < 0)
		logsyserr("<getconnect> could not get TCP socket");

	loginfo("<quakebbs> QuakeBBS v%s Now Running!", QBBS_VERSION);

	for(;;)
	{
		loginfo("<quakebbs> Waiting for TCP connect on %d or UDP connect on %d", config.serv[0].port[0], config.serv[0].port[1]);

    	FD_ZERO(&fdmask);
		FD_SET(udp_sock, &fdmask);
		FD_SET(tcp_sock, &fdmask);

		if(select(1+((tcp_sock > udp_sock) ? tcp_sock : udp_sock), &fdmask, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
			logerr("<getconnect> select failed");

		if(FD_ISSET(udp_sock, &fdmask))
			udpconnect(udp_sock);

		if(FD_ISSET(tcp_sock, &fdmask))
			tcpconnect(tcp_sock);
	}
}

udp_open(retry)
int retry;
{
	int udp_sock;
	struct sockaddr_in lsrv_addr;

	if(retry > QBBS_SOCKET_RETRY)
		return(-1);

	bzero((char *) &lsrv_addr, sizeof(lsrv_addr));
	lsrv_addr.sin_family		= AF_INET;
	lsrv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	lsrv_addr.sin_port			= htons(config.serv[0].port[1]);

	if((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
		logsyserr("<udp_open> can't open UDP socket");

	if(bind(udp_sock, (struct sockaddr *)&lsrv_addr, sizeof(lsrv_addr))<0)
	{
		if(errno == EADDRINUSE)
		{
			close(udp_sock);
			logerr("<udp_open> couldn't bind socket: retry #%d", retry);
			sleep(QBBS_SOCKET_WAIT);
			return(udp_open(++retry));
		}
		logsyserr("<udp_open> can't bind UDP server socket");
	}

	return(udp_sock);
}

tcp_open(retry)
int retry;
{
	int tcp_sock;
	struct sockaddr_in lsrv_addr;

	if(retry > QBBS_SOCKET_RETRY)
		return(-1);

	bzero((char *) &lsrv_addr, sizeof(lsrv_addr));
	lsrv_addr.sin_family		= AF_INET;
	lsrv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	lsrv_addr.sin_port			= htons(config.serv[0].port[0]);

	if((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		logsyserr("<tcp_open> can't open TCP socket");

	if(bind(tcp_sock, (struct sockaddr *)&lsrv_addr, sizeof(lsrv_addr))<0)
	{
		if(errno == EADDRINUSE)
		{
			close(tcp_sock);
			logerr("<tcp_open> couldn't bind socket: retry #%d", retry);
			sleep(QBBS_SOCKET_WAIT);
			return(tcp_open(++retry));
		}
		logsyserr("<tcp_open> can't bind TCP server socket");
	}

	listen(tcp_sock, 5);

	return(tcp_sock);
}

udpconnect(udpsock)
int udpsock;
{
	int n,
	    clilen,
		pid,
		qsock,
		fd;
	byte pkt[QBBSMAXPKT],
		 rpkt[QBBSMAXPKT],
		 filename[256],
		 buf[QBBSMAXBUF];
	fd_set fdmask;
	static struct timeval timeout;
	struct sockaddr_in rcli_addr;
	struct sockaddr_in qsrv_addr;
	struct sockaddr_in qcli_addr;
	
/*
 *  Get PKT
 */
 	clilen = sizeof(rcli_addr);

	if(recvfrom(udpsock, &pkt, QBBSMAXPKT, 0, (struct sockaddr *)&rcli_addr, &clilen) < 0)
		return(logerr("<udpconnect> recvfrom error"));

	if((pid = fork()) < 0)
		logsyserr("<udpconnect> fork error");

	if(pid) /* parent */
		return(0);

	/*  Child  */

	if(pkt[4] == CCREQ_CONNECT)	/* It's a Quake Client */
	{

/*
 *  Build our CCREP_REJECT packet
 */
		sprintf(buf, "\nQuakeBBS v%s (c)1996 Donovan Young\n\n", QBBS_VERSION);
		strcpy(rpkt, buf);

		/* use TEXT directory */

		sprintf(filename, "%s/%s", config.dirs[2], QBBS_FILE_NOCONNECTUDP);

 		if((fd = open(filename, O_RDONLY)) < 0)
		{
			logerr("<udpconnect> Can't open %s, using generic response", filename);
			strcat(rpkt, "Sorry, this is a private QuakeBBS QuakeSite, Please call back with an authorized QuakeBBS Client.  Thank You.\n");
		}
		else
		{
			while(readline(fd, buf, sizeof(buf)))
				strcat(rpkt, buf);
			if(strlen(rpkt) > 195)	/* Custom packet limited to 195 chars */
				rpkt[195] = '\0';
		}

			
		pkt[0] = 0x80;
		pkt[1] = 0x00;
		pkt[4] = CCREP_REJECT;
		strcpy((pkt+5), rpkt);

		n = strlen(rpkt)+6;
		pkt[3] = n;

		if(sendto(udpsock, pkt, n, 0, (struct sockaddr *)&rcli_addr, sizeof(rcli_addr)) != n)
			return(logerr("<udpconnect> sendto error"));

		loginfo("<quakebbs> CCREP_REJECT packet sent to Quake Client");
	}
	close(udpsock);
	exit(0);
}

tcpconnect(tcpsock)
int tcpsock;
{
	int pid,
		csock,
		clilen;
	struct sockaddr cli_addr;

	clilen = sizeof(cli_addr);

	if((csock = accept(tcpsock, &cli_addr, &clilen)) < 0)
		return(logerr("<tcpconnect> accept error"));
	
	if((pid = fork()) < 0)
		logsyserr("<tcpconnect> could not fork");

	if(!pid)
	{
		close(tcpsock);
		quakebbs_login(csock);
		close(csock);
		exit(0);
	}
	close(csock);
	return(0);
}

