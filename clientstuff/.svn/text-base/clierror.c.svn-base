/*
 * Error handling routines.
 */

#include	<stdio.h>
#include	<varargs.h>

#ifndef	NULL
#define	NULL	('\0')
#endif

extern int	errno;		/* Unix error number */
extern int	sys_nerr;	/* # of error message strings in sys table */
extern char	*sys_errlist[];	/* the system error message table */

extern char *pname;

char	emesgstr[255] = {0};	/* used by all server routines */

/*
 * Fatal error.  Print a message and terminate.
 * Don't print the system's errno value.
 *
 *	logquit(str, arg1, arg2, ...)
 *
 * The string "str" must specify the conversion specification for any args.
 */

logquit(va_alist)
va_dcl
{
	va_list		args;
	char		*fmt;

	va_start(args);
	fmt = va_arg(args, char *);
	vsprintf(emesgstr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n%s\n", emesgstr);

	exit(1);
}

/*
 * Fatal error related to a system call.  Print a message and terminate.
 * Don't dump core, but do print the system's errno value and its
 * associated message.
 *
 *	logsyserr(str, arg1, arg2, ...)
 *
 * The string "str" must specify the conversion specification for any args.
 */

logsyserr(va_alist)
va_dcl
{
	va_list		args;
	char		*fmt;

	va_start(args);
	fmt = va_arg(args, char *);
	vsprintf(emesgstr, fmt, args);
	va_end(args);

	my_perror();

	fprintf(stderr, "\n%s\n", emesgstr);

	exit(1);
}

/*
 * Recoverable error.  Print a message, and return to caller.
 *
 *	logerr(str, arg1, arg2, ...)
 *
 * The string "str" must specify the conversion specification for any args.
 */

logerr(va_alist)
va_dcl
{
	va_list		args;
	char		*fmt;

	va_start(args);
	fmt = va_arg(args, char *);
	vsprintf(emesgstr, fmt, args);
	va_end(args);

	my_perror();

	fprintf(stderr, "\n%s\n", emesgstr);

	return(-1);
}

/*
 * Fatal error.  Print a message, dump core (for debugging) and terminate.
 *
 *	logdump(str, arg1, arg2, ...)
 *
 * The string "str" must specify the conversion specification for any args.
 */

logdump(va_alist)
va_dcl
{
	va_list		args;
	char		*fmt;

	va_start(args);
	fmt = va_arg(args, char *);
	vsprintf(emesgstr, fmt, args);
	va_end(args);

	my_perror();

	fprintf(stderr, "\n%s\n", emesgstr);

	abort();		/* dump core and terminate */
	exit(1);		/* shouldn't get here */
}

/*
 * Log a LOG_INFO message and return to caller.
 *
 *	loginfo(str, arg1, arg2, ...)
 *
 * The string "str" must specify the conversion specification for any args.
 */

loginfo(va_alist)
va_dcl
{
	va_list		args;
	char		*fmt;

	va_start(args);
	fmt = va_arg(args, char *);
	vsprintf(emesgstr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n%s\n", emesgstr);

	return;
}

/*
 * Log a LOG_DEBUG message and return to caller.
 *
 *	logdebug(str, arg1, arg2, ...)
 *
 * The string "str" must specify the conversion specification for any args.
 */

logdebug(va_alist)
va_dcl
{
	va_list		args;
	char		*fmt;

	va_start(args);
	fmt = va_arg(args, char *);
	vsprintf(emesgstr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n%s\n", emesgstr);

	return;
}

/*
 * Print the UNIX errno value.
 * We just append it to the end of the emesgstr[] array.
 */

my_perror()
{
	register int	len;
	char		*sys_err_str();

	len = strlen(emesgstr);
	sprintf(emesgstr + len, " %s", sys_err_str());
}

/*
 * Return a string containing some additional operating-system
 * dependent information.
 * Note that different versions of UNIX assign different meanings
 * to the same value of "errno" (compare errno's starting with 35
 * between System V and BSD, for example).  This means that if an error
 * condition is being sent to another UNIX system, we must interpret
 * the errno value on the system that generated the error, and not
 * just send the decimal value of errno to the other system.
 */

char *
sys_err_str()
{
	static char	msgstr[200];

	if (errno != 0) {
		if (errno > 0 && errno < sys_nerr)
			sprintf(msgstr, "(%s)", sys_errlist[errno]);
		else
			sprintf(msgstr, "(errno = %d)", errno);
	} else {
		msgstr[0] = '\0';
	}

	return(msgstr);
}
