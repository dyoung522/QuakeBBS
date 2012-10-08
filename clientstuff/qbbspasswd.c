
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>

#include "../common/qbbsuser.h"

extern char *optarg;
extern int optind;

int fd;

usage(pname)
char *pname;
{
	fprintf(stderr, "Usage:  %s <username>\n", pname);
	return;
}

main(argc, argv)
int argc;
char *argv[];
{
	char opt;

	if(argc != 2)
	{
		usage(argv[0]);
		exit(1);
	}

	if((fd = open("data/quakebbs.users", O_RDWR, 0666)) < 0)
	{
		fprintf(stderr, "Error reading quakebbs.users file\n");
		exit(-1);
	}

	while(read(fd, user, sizeof(user)) > 0)
	{
		if(memcmp(argv[1], user.uname, strlen(user.uname)) == 0)
		{
			passwd();
			exit(0);
		}
	}

	fprintf(stderr, "User %s not found in database\n", argv[1]);
	exit(0);
}

passwd()
{
	strcpy(user.passwd, crypt(getpass("Password: "), QBBS_LOGIN_SALT));

	lseek(fd, -((long)sizeof(user)), SEEK_CUR);
	if(write(fd, user, sizeof(user)) != sizeof(user))
	{
		fprintf(stderr, "Error writing user file\n");
		exit(-1);
	}

	exit(0);
}

