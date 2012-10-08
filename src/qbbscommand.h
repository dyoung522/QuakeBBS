/*
 *  QuakeBBS Command structures
 */

#ifndef QBBSCOMMANDH
#define QBBSCOMMANDH

/*  Commands  */

int qbbs_chat();
int qbbs_help();
int qbbs_kick();
int qbbs_who();

struct 
{
	char *name;
	int (*function)();
} qcommand[] = 
	{
		{ "chat",	qbbs_chat	},
		{ "help",	qbbs_help	},
		{ "kick", 	qbbs_kick	},
		{ "who",	qbbs_who	},
		{ NULL }
	};

#endif /* QBBSCOMMANDH */
