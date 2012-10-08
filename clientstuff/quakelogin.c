/*
 * Example of client using UDP protocol.
 */

#include	"../common/quakebbs.h"

main(argc, argv)
int	argc;
char	*argv[];
{
	int			sockfd;
	char sendline[MAXPKT];
	struct sockaddr_in	cli_addr, serv_addr;

	/*
	 * Fill in the structure "serv_addr" with the address of the
	 * server that we want to send to.
	 */

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = htons(26000);

	/*
	 * Open a UDP socket (an Internet datagram socket).
	 */

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("client: can't open datagram socket");
		exit(1);
	}

	/*
	 * Bind any local address for us.
	 */

	bzero((char *) &cli_addr, sizeof(cli_addr));	/* zero out */
	cli_addr.sin_family      = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port        = htons(0);
	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
	{
		perror("client: can't bind local address");
		exit(1);
	}

    sendline[0] = 0x80;
	sendline[1] = 0x00;
	sendline[2] = 0x00;
	sendline[3] = 0x0c;
	sendline[4] = 0x01;
	sendline[5] = 'Q';
	sendline[6] = 'U';
	sendline[7] = 'A';
	sendline[8] = 'K';
	sendline[9] = 'E';
	sendline[10] = 0x00;
	sendline[11] = 0x03;

    sendto(sockfd, sendline, sendline[3], 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	close(sockfd);
	exit(0);
}
