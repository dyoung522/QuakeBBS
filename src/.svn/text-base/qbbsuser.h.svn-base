/*
 *  qbbsuser.h - QuakeBBS User Database structures.
 *
 *  Copyright 1996 Donovan C. Young
 *
 *  Contains the user structure used in the quakebbs.users file.  Feel to
 *  develop your own user database utilities.  I only ask that if you create
 *  something nice, you share it with me.  :-)
 */

#ifndef QBBSUSERH
#define QBBSUSERH

#define QBBSUSERUNAME	26
#define QBBSUSERRNAME	256
#define QBBSUSERPASSWD	15
#define QBBSUSEREMAIL	256
#define QBBSUSERURL		256
#define QBBSUSERPHONE	26
#define QBBSUSERREASON	256
#define QBBSUSERCOMMENT	512

struct qbbs_user
{
	char uname[QBBSUSERUNAME];		/* User Name (Handle)			*/
	char realname[QBBSUSERRNAME];	/* Users Real Name				*/
	char passwd[QBBSUSERPASSWD];	/* Encrypted Password			*/
	char email[QBBSUSEREMAIL];		/* EMail address				*/
	char url[QBBSUSERURL];			/* URL (if any)					*/
	char phone[QBBSUSERPHONE];		/* Phone Number					*/
	int seclevel;					/* Security Level				*/
	long credits;					/* Credits						*/
	time_t login;					/* date/time of last login		*/
	struct							/* Security Flags				*/
	{
		unsigned validated	: 1;	/* Set if user is validated		*/
		unsigned locked		: 1;	/* Set if user is locked out	*/
		unsigned banned		: 1;	/* Set if user is banned		*/
		unsigned quake		: 1;	/* Set for Quake server access	*/
		unsigned sysop		: 1;	/* Set if user has SysOp access	*/
		unsigned res4		: 1;	/* Reserved						*/
		unsigned user0		: 1;	/* User Defined					*/
		unsigned user1		: 1;	/* User Defined					*/

		unsigned user2		: 1;	/* User Defined					*/
		unsigned user3		: 1;	/* User Defined					*/
		unsigned user4		: 1;	/* User Defined					*/
		unsigned user5		: 1;	/* User Defined					*/
		unsigned user6		: 1;	/* User Defined					*/
		unsigned user7		: 1;	/* User Defined					*/
		unsigned user8		: 1;	/* User Defined					*/
		unsigned user9		: 1;	/* User Defined					*/
	} sec_flags;
	char reason[QBBSUSERREASON];	/* Reason for ban or lock, etc	*/
	char comment[QBBSUSERCOMMENT];	/* Free form comment field		*/
	char junk[1024];				/* For future expandability		*/
};

struct qbbs_user_status
{
	unsigned long addr;				/* Current IP ADDR				*/
	struct {
		unsigned login	: 1;		/* Set once user is logged in	*/
		unsigned new 	: 1;		/* Set if a new user			*/
		unsigned guest 	: 1;		/* Set if a guest user			*/
		unsigned quake	: 1;		/* Set if authorized on QServer */
		unsigned qauth	: 1;		/* Set if authorized on QServer */
	} flags;
};

#endif /* QBBSUSERH */
