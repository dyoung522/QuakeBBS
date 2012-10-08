/*
 * Example of QuakeBBS Client
 */

#include	"../common/quakebbs.h"

#define MAXBUF 1024

struct
{
	struct
	{
		unsigned conlog : 1;
	} opts;
} config;

int logfile;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	sockfd;
	struct sockaddr_in serv_addr;

	config.opts.conlog = 1;  /*  log to console  */

	/*
	 * Fill in the structure "serv_addr" with the address of the
	 * server that we want to send to.
	 */

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port        = htons(26000);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		logdump("client: can't open stream socket");

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		logsyserr("client: can't connect to server");
	
	get_connect(sockfd);
	do_command(sockfd);		/* We're in */
		
	close(sockfd);		/*  should never execute  */
	exit(1);
}

get_connect(sock)
int sock;
{
	char buf[MAXBUF],
	     salt[3];
	int control,
		tries = 0;
	struct qbbs_chello chello;
	struct qbbs_shello shello;
	struct qbbs_login login;

/*
 *  Throw away any data recieved before we send our chello packet
 */

	recv(sock, buf, sizeof(buf), 0);

	bzero((char *)&shello, sizeof(chello));

	bzero((char *)&chello, sizeof(chello));
	strcpy(chello.start, "QClient");
	chello.ver = 0x01;

    if(send(sock, (char *)&chello, sizeof(chello), 0) < 0)
		logsyserr("client:  error sending chello");
		
    if(recv(sock, &salt, sizeof(salt), 0) < 0)
		logsyserr("client: recv error");

	printf("\nLogin: ");
	fgets(login.uname, 81, stdin);

	/* Strip off \n */

	login.uname[strlen(login.uname)-1] = '\0';

	strcpy(login.passwd, (char *)crypt(getpass("Password: "), salt));

	login.ver = QBBS_PROT_VER;

    if(send(sock, (char *)&login, sizeof(login), 0) < 0)
		logsyserr("client:  error sending login");
		
    if(recv(sock, &shello, sizeof(shello), 0) < 0)
		logsyserr("client: recv error");

	if(shello.status == QBBS_LOGIN_ACCEPT)
	{
		printf("%s\n", shello.msg);
		return(1);
	}

	else if(shello.status == QBBS_LOGIN_REJECT)
		loginfo("Login Rejected:  %s", shello.msg);

	else
	{
		logerr("Didn't receive a valid reply from Remote");
		close(sock);
		exit(0);
	}

	close(sock);
	exit(0);
}

do_command(sock)
int sock;
{
	int status;
	fd_set fdmask;
	char comreq[QBBSMAXCOMMAND],
		 *prompt = "Command > ";
	struct qbbs_command command;

	putchar('\n');
	printf(prompt);
	fflush(stdout);

	while(1)
	{
		FD_ZERO(&fdmask);
		FD_SET(0, &fdmask);
		FD_SET(sock, &fdmask);

	    if(select(sock+1, &fdmask, (fd_set *)0, (fd_set *)0, 0) < 0)
	        logsyserr("select error");

		if(FD_ISSET(0, &fdmask))
		{
			gets(comreq);

			if((status = send(sock, comreq, strlen(comreq), 0)) < 0)
				logsyserr("send error");

		}
		if(FD_ISSET(sock, &fdmask))
		{
			*command.msg = '\0';
			if((status = recv(sock, &command, sizeof(command), 0)) < 0)
				logsyserr("recv error");

			if(!status)
				logquit("Remote disconnect");

			switch(command.com)
			{
				case QBBS_COMREPLY_QUIT:
					printf("\n%s\n", command.msg);
					exit(0);
					break;
				case QBBS_COMREPLY_QUAKE:
					start_quake(&command);
					printf(prompt);
					fflush(stdout);
					break;
				default:
					printf("\n%s\n\n%s", command.msg, prompt);
					fflush(stdout);
					break;
			}
		}
	}
}

start_quake(command)
struct qbbs_command *command;
{
	char comline[256],
	     qname[256],
	     qaddr[16],
		 qport[10];

	fprintf(stderr, "command.msg = \"%s\"\n", command->msg);

	sscanf(command->msg, "%[^:]:%[^:]:%s", qname, qaddr, qport);

	sprintf(comline, "+connect %s -udpport %s", qaddr, qport);

	printf("\nAuthorized on %s, start Quake with '%s'\n\n", qname, comline);

	return;
}

