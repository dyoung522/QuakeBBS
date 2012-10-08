/*
 *  QuakeBBS User Administration Utility
 *
 *  Copyright 1996 Donovan C. Young
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <time.h>

#include "qbbsuser.h"

#define MAXBUF 256

extern char *optarg;
extern int optind;

struct qbbs_user user;
char userfile[256];
char *pname;

usage()
{
	fprintf(stderr, "Usage:  %s [-F <filename>] [al] [BdefLpQSV <username>]\n", pname);
	return;
}

help()
{
	usage();
	fprintf(stderr, "\n");
	fprintf(stderr, "\ta\tAdd User\n");
	fprintf(stderr, "\tl\tList User\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\tB <username>\tBAN User - prompts for reason [toggle]\n");
	fprintf(stderr, "\tD <username>\tDelete User\n");
	fprintf(stderr, "\te <username>\tEdit User\n");
	fprintf(stderr, "\tf <username>\tFind User\n");
	fprintf(stderr, "\tF <filename>\tUser <filename> as userfile\n");
	fprintf(stderr, "\tL <username>\tLock User - prompts for reason [toggle]\n");
	fprintf(stderr, "\tp <username>\tChange User password - prompts for password\n");
	fprintf(stderr, "\tQ <username>\tQuake Access [toggle]\n");
	fprintf(stderr, "\tS <username>\tSysOp Access [toggle]\n");
	fprintf(stderr, "\tV <username>\tValidate User [toggle]\n");
	fprintf(stderr, "\n");

	return(0);
}

main(argc, argv)
int argc;
char *argv[];
{
	int fd;
	char opt;

	pname = argv[0];
	strcpy(userfile, "data/quakebbs.users");

	if(argc > 1)
	{
		while((opt = getopt(argc, argv, "aB:D:e:f:F:hlL:p:Q:S:V:")) != -1)
		{
			switch(opt)
			{
				case 'a':
					adduser();
					break;
				case 'B':
					banuser(optarg);
					break;
				case 'D':
					deluser(optarg);
					break;
				case 'e':
					edituser(optarg);
					break;
				case 'f':
					showuser(optarg);
					break;
				case 'F':
					strcpy(userfile, optarg);
					break;
				case 'h':
					help();
					break;
				case 'l':
					listuser();
					break;
				case 'L':
					lockuser(optarg);
					break;
				case 'p':
					passwd(optarg);
					break;
				case 'Q':
					quakeaccess(optarg);
					break;
				case 'S':
					sysopaccess(optarg);
					break;
				case 'V':
					validate(optarg);
					break;

				default:
					usage();
					exit(opt != 'h');
			}
		}
		exit(0);
	}
	else
		usage();
}

adduser()
{
	char buf[256];

	while(1)
	{
		getinput(user.uname, "\nUser Name: ");

		if(user.uname[0] == '\0')
			return;

		getinput(user.realname, "Real Name: ");

		if(user.realname[0] == '\0')
			strcpy(user.realname, "Not Given");

		strcpy(user.passwd, crypt(getpass("Password: "), "Qb"));

		getinput(user.email, "EMail Addr: ");

		if(user.email[0] == '\0')
			strcpy(user.email, "NONE");

		getinput(user.url, "WWW URL: ");

		if(user.url[0] == '\0')
			strcpy(user.url, "NONE");

		getinput(user.phone, "Phone: ");

		if(user.phone[0] == '\0')
			strcpy(user.phone, "---");

		getinput(buf, "SEC Level: ");
		user.seclevel = atoi(buf);

		getinput(buf, "Credits: ");
		user.credits = atol(buf);

		fprintf(stderr, "writing userfile...");
		writeuser(0);
		fprintf(stderr, " done\n");
	}
	return;
}

banuser(username)
char *username;
{
	int userno;
	char buf[MAXBUF];

	if((userno = finduser(username)));
	{
		if(user.sec_flags.banned)
			user.sec_flags.banned = 0;
		else
		{
			user.sec_flags.banned = 1;
			getinput(buf, "Reason: ");
			if(*buf == '\0')
				strcpy(user.reason, "No reason given");
			else
				strcpy(user.reason, buf);
		}

		writeuser(userno);

		fprintf(stdout, "User %s %s\n", username, (user.sec_flags.banned ? "banned" : "unbanned"));
		return(0);
	}
	fprintf(stderr, "User %s not found in database\n", username);
	return(1);
}

deluser(username)
char * username;
{
	int rfd, wfd,
	    userno,
		ucount = 1;
	char buf[MAXBUF];

	if(isdigit(*username))
		userno = atoi(username);

	else if(!(userno = finduser(username)))
	{
		fprintf(stderr, "User %s not found in database\n", username);
		return;
	}

	strcpy(buf, "qbbsusers.tmp");

	if((wfd = open(buf, O_WRONLY | O_CREAT, 0600)) < 0)
	{
		fprintf(stderr, "<deluser> error opening %s\n", buf);
		exit(1);
	}

	if((rfd = openuser(O_RDONLY)) < 0 )
	{
		fprintf(stderr, "Error opening userfile\n");
		exit(1);
	}

	while(read(rfd, &user, sizeof(user)) > 0)
	{
		if(userno != ucount++)
		{
			if(write(wfd, &user, sizeof(user)) < 0)
			{
				fprintf(stderr, "<deluser> write error on tmp file\n");
				exit(1);
			}
		}
	}

	if(rename(buf, userfile) < 0)
	{
		perror("<deluser>  could not rename tmp file");
		exit(1);
	}

	close(rfd);
	close(wfd);
	return;
}

edituser(username)
char *username;
{
	char buf[256],
	     command;
	int userno, writeok;

	if(!(userno = finduser(username)))
	{
		fprintf(stderr, "User %s not found in database\n", username);
		return;
	}

	while(1)
	{
		writeok = 0;
		fprintf(stdout, "\n");
		fprintf(stdout, "[U]sername   : \"%s\"\n", user.uname);
		fprintf(stdout, "[R]eal Name  : \"%s\"\n", user.realname);
		fprintf(stdout, "[P]asswd     : %s\n", user.passwd);
		fprintf(stdout, "[E]Mail      : %s\n", user.email);
		fprintf(stdout, "[W]WW        : %s\n", user.url);
		fprintf(stdout, "P[h]one      : %s\n", user.phone);
		fprintf(stdout, "[s]eclevel   : %d\n", user.seclevel);
		fprintf(stdout, "[c]redits    : %ld\n", user.credits);
		fprintf(stdout, "\n");
		fprintf(stdout, "Sa[v]e Record\n");
		fprintf(stdout, "[Q]uit\n\n");

		getinput(buf, "Command: ");
		command = tolower(buf[0]);

		switch(command)
		{
			case 'u':
				getinput(user.uname, "\nUser Name: ");
				break;

			case 'r':
				getinput(user.realname, "Real Name: ");
				break;

			case 'p':
				strcpy(user.passwd, crypt(getpass("Password: "), "Qb"));
				break;

			case 'e':
				getinput(user.email, "EMail Addr: ");
				break;

			case 'w':
				getinput(user.url, "WWW URL: ");
				break;

			case 'h':
				getinput(user.phone, "Phone: ");
				break;

			case 's':
				getinput(buf, "SEC Level: ");
				user.seclevel = atoi(buf);
				break;

			case 'c':
				getinput(buf, "Credits: ");
				user.credits = atol(buf);
				break;

			case 'q':
				return;

			case 'v':
				writeok = 1;
				break;

			case NULL:
				break;

			default:
				fprintf(stderr, "Invalid command '%c'", command);
				break;
		}
		if(writeok)
		{
			fprintf(stderr, "writing userfile...");
			writeuser(userno);
			fprintf(stderr, " done\n");
		}
	}
}

listuser()
{
	int fd,
		x = 1;

	if((fd = openuser(O_RDONLY)) < 0 )
	{
		fprintf(stderr, "Error opening userfile\n");
		exit(1);
	}

	while(read(fd, &user, sizeof(user)) > 0)
	{
		fprintf(stdout, "\nUser %d\n\n", x++);
		dispuser();
		fprintf(stderr, "\nPress RETURN for next record ");
		getchar();
	}

	close(fd);
	return;
}

showuser(username)
char *username;
{
	int userno;

	if((userno = finduser(username)))
	{
		fprintf(stdout, "\nUser %d\n\n", userno);
		dispuser();
		fprintf(stdout, "\n");
	}
	else
		fprintf(stderr, "User %s not found in database\n", username);

	return;
}

dispuser()
{
	fprintf(stdout, "\tUsername   : \"%s\"\n", user.uname);
	fprintf(stdout, "\tReal Name  : \"%s\"\n", user.realname);
	fprintf(stdout, "\tPasswd     : %s\n", user.passwd);
	fprintf(stdout, "\tEMail      : %s\n", user.email);
	fprintf(stdout, "\tWWW        : %s\n", user.url);
	fprintf(stdout, "\tPhone      : %s\n", user.phone);
	fprintf(stdout, "\tseclevel   : %d\n", user.seclevel);
	fprintf(stdout, "\tcredits    : %ld\n", user.credits);
	fprintf(stdout, "\tlast login : %s", ctime(&user.login));
	fprintf(stdout, "\n\t\tValidated: %s\n", (user.sec_flags.validated ? "YES" : "NO"));
	if(user.sec_flags.locked)
		fprintf(stdout, "\t\tLocked   : YES (%s)\n", user.reason);
	else
		fprintf(stdout, "\t\tLocked   : NO\n");

	if(user.sec_flags.banned)
		fprintf(stdout, "\t\tBanned   : YES (%s)\n", user.reason);
	else
		fprintf(stdout, "\t\tBanned   : NO\n");

	fprintf(stdout, "\t\tQuake    : %s\n", (user.sec_flags.quake ? "YES" : "NO"));
	fprintf(stdout, "\t\tSysOp    : %s\n", (user.sec_flags.sysop ? "YES" : "NO"));
	fprintf(stdout, "\n\t\tUser0    : %s\tUser5    : %s\n", (user.sec_flags.user0 ? "YES" : "NO"), (user.sec_flags.user5 ? "YES" : "NO"));
	fprintf(stdout, "\t\tUser1    : %s\tUser6    : %s\n", (user.sec_flags.user1 ? "YES" : "NO"), (user.sec_flags.user6 ? "YES" : "NO"));
	fprintf(stdout, "\t\tUser2    : %s\tUser7    : %s\n", (user.sec_flags.user2 ? "YES" : "NO"), (user.sec_flags.user7 ? "YES" : "NO"));
	fprintf(stdout, "\t\tUser3    : %s\tUser8    : %s\n", (user.sec_flags.user3 ? "YES" : "NO"), (user.sec_flags.user8 ? "YES" : "NO"));
	fprintf(stdout, "\t\tUser4    : %s\tUser9    : %s\n", (user.sec_flags.user4 ? "YES" : "NO"), (user.sec_flags.user9 ? "YES" : "NO"));
}

lockuser(username)
char *username;
{
	int userno;
	char buf[MAXBUF];

	if((userno = finduser(username)))
	{
		if(user.sec_flags.locked)
			user.sec_flags.locked = 0;
		else
		{
			user.sec_flags.locked = 1;
			getinput(buf, "Reason: ");
			if(buf[0] == '\0')
				strcpy(user.reason, "No reason given");
			else
				strcpy(user.reason, buf);
		}

		writeuser(userno);

		fprintf(stdout, "User %s %s\n", username, (user.sec_flags.locked ? "locked" : "unlocked"));
		return(0);
	}
	fprintf(stderr, "User %s not found in database\n", username);
	return(1);
}

passwd(username)
char *username;
{
	int userno;

	if((userno = finduser(username)))
	{
		strcpy(user.passwd, crypt(getpass("Password: "), "Qb"));

		writeuser(userno);

		return(0);
	}
	fprintf(stderr, "User %s not found in database\n", username);
	return(1);
}

quakeaccess(username)
char *username;
{
	int userno;

	if((userno = finduser(username)))
	{
		user.sec_flags.quake = (user.sec_flags.quake ? 0 : 1);

		writeuser(userno);

		fprintf(stdout, "User %s %s\n", username, (user.sec_flags.quake ? "granted Quake Access" : "denied Quake Access"));
		return(0);
	}
	fprintf(stderr, "User %s not found in database\n", username);
	return(1);
}

sysopaccess(username)
char *username;
{
	int userno;

	if((userno = finduser(username)))
	{
		user.sec_flags.sysop = (user.sec_flags.sysop ? 0 : 1);

		writeuser(userno);

		fprintf(stdout, "User %s %s\n", username, (user.sec_flags.sysop ? "granted SysOp Access" : "denied SysOp Access"));
		return(0);
	}
	fprintf(stderr, "User %s not found in database\n", username);
	return(1);
}

validate(username)
char *username;
{
	int userno;

	if((userno = finduser(username)))
	{
		user.sec_flags.validated = (user.sec_flags.validated ? 0 : 1);

		writeuser(userno);

		fprintf(stdout, "User %s %s\n", username, (user.sec_flags.validated ? "validated" : "unvalidated"));
		return(0);
	}
	fprintf(stderr, "User %s not found in database\n", username);
	return(1);
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

writeuser(userno)
int userno;
{
	int fuser;

	if((fuser = openuser(O_RDWR)) < 0)
	{
		fprintf(stderr, "<writeuser> could not open userfile\n");
		exit(1);
	}

	lseek(fuser, 0L, 0);
	if(lockf(fuser, F_LOCK, 0L) < 0)
	{
		fprintf(stderr, "<writeuser> can't lock userfile\n");
		exit(1);
	}

	if(userno > 0)
	{
		/*  Seek to just before userno */
		if(lseek(fuser, (long)((sizeof(user)*userno)-sizeof(user)), SEEK_SET) < 0)
		{
			fprintf(stderr, "<writeuser> seek error\n");
			exit(1);
		}
	}
	else	/* 0 = New Record */
		if(lseek(fuser, 0L, SEEK_END) < 0)
		{
			fprintf(stderr, "<writeuser> seek error\n");
			exit(1);
		}

	if(write(fuser, &user, sizeof(user)) < 0)
	{
		fprintf(stderr, "<writeuser> write error on userfile\n");
		exit(1);
	}

	lseek(fuser, 0L, 0);
	if(lockf(fuser, F_ULOCK, 0L) < 0)
	{
		fprintf(stderr, "<writeuser> can't unlock userfile\n");
		exit(1);
	}

	close(fuser);
	return;
}

readuser(userno)
int userno;
{
	int status = 0,
		fuser;

	if((fuser = openuser(O_RDONLY)) < 0)
	{
		fprintf(stderr, "<readuser> could not open userfile\n");
		exit(1);
	}

	/*  Seek to just before userno */
	lseek(fuser, (long)((sizeof(user)*userno)-sizeof(user)), SEEK_SET);

	if((status = read(fuser, &user, sizeof(user))) < 0)
	{
		fprintf(stderr, "<readuser> read error on userfile\n");
		exit(1);
	}

	close(fuser);
	return(status);
}

finduser(username)
char *username;
{
	int userno = 0,
		status = 0,
		fuser;
	char luname[256],
	     lusername[256];

	if((fuser = openuser(O_RDONLY)) < 0)
	{
		fprintf(stderr, "<finduser> could not open userfile\n");
		exit(1);
	}

	strlow(lusername, username);

	while((status = read(fuser, &user, sizeof(user))) > 0)
	{
		userno++;
		strlow(luname, user.uname);
		if(memcmp(luname, lusername, strlen(username)) == 0)
		{
			close(fuser);
			return(userno);
		}
	}
	if(status < 0)
	{
		fprintf(stderr, "<finduser> read error on userfile\n");
		exit(1);
	}

	close(fuser);
	return(0);
}

openuser(mode)
int mode;
{
	int fuser;

	if((fuser = open(userfile, mode)) < 0)
	{
		fprintf(stderr, "<openuser> error opening %s\n", userfile);
	}

	return(fuser);
}

getinput(buf, prompt)
char *buf;
char *prompt;
{
	fprintf(stdout, prompt);

	memset(buf, NULL, sizeof(&buf));
	gets(buf);

	return;
}
