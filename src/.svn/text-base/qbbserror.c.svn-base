/*
 * Error handling routines.
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

#include	"quakebbs.h"
#include	<varargs.h>
#include	<syslog.h>

#ifndef	NULL
#define	NULL	('\0')
#endif

extern int	errno;		/* Unix error number */
extern int	sys_nerr;	/* # of error message strings in sys table */
extern char	*sys_errlist[];	/* the system error message table */

extern char *pname;
extern struct qbbs_config config;

char	emesgstr[QBBSMAXBUF+1] = {0};	/* used by all server routines */

/*
 * Identify ourself, for syslog() messages.
 *
 * LOG_PID is an option that says prepend each message with our pid.
 * LOG_CONS is an option that says write to console if unable to send
 * the message to syslogd.
 * LOG_DAEMON is our facility.
 */

loginit(ident)
char	*ident;
{
	openlog(ident, (LOG_PID | LOG_CONS), LOG_DAEMON);
}

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

	if(config.opts.conlog)
		fprintf(stderr, "%s\n", emesgstr);
	else
	{
		if(config.opts.logfile)
			logtofile(emesgstr);
		if(config.opts.syslog)
			syslog(LOG_ERR, emesgstr);
	}

	exit(0);
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

	if(config.opts.conlog)
		fprintf(stderr, "%s\n", emesgstr);
	else
	{
		if(config.opts.logfile)
			logtofile(emesgstr);
		if(config.opts.syslog)
			syslog(LOG_ERR, emesgstr);
	}

	err_exit(1);
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

	if(config.opts.conlog)
		fprintf(stderr, "%s\n", emesgstr);
	else
	{
		if(config.opts.logfile)
			logtofile(emesgstr);
		if(config.opts.syslog)
			syslog(LOG_ERR, emesgstr);
	}

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

	if(config.opts.conlog)
		fprintf(stderr, "%s\n", emesgstr);
	else
	{
		if(config.opts.logfile)
			logtofile(emesgstr);
		if(config.opts.syslog)
			syslog(LOG_ERR, emesgstr);
	}

	abort();		/* dump core and terminate */
	err_exit(1);		/* shouldn't get here */
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

	if(config.opts.conlog)
		fprintf(stderr, "%s\n", emesgstr);
	else
	{
		if(config.opts.logfile)
			logtofile(emesgstr);
		if(config.opts.syslog)
			syslog(LOG_INFO, emesgstr);
	}

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

	if(config.opts.conlog)
		fprintf(stderr, "%s\n", emesgstr);
	else
        logtofile(emesgstr);

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

logtofile(buf)
char *buf;
{
	FILE *logfile;
	time_t now;
	struct tm *timep;

	if((logfile = fopen(config.file[4], "a")) == NULL)
	{
		config.opts.conlog = 1;   /*  Need this to avoid catch-22  */
		config.opts.logfile = 0;
		logsyserr("<get_config> error opening %s", config.file[4]);
	}

	time(&now);
	timep = localtime(&now);
	fprintf(logfile, "[%02d/%02d/%02d %02d:%02d:%02d] %s\n",
	                  timep->tm_mon + 1, timep->tm_mday, timep->tm_year,
					  timep->tm_hour, timep->tm_min, timep->tm_sec,
					  buf);
	fflush(logfile);

	fclose(logfile);
	return;
}

logtodebug(buf)
char *buf;
{
	FILE *dfile;
	time_t now;
	struct tm *timep;
    char fname[QBBSFILELEN];

    sprintf(fname, "%s/quakebbs.debug", config.dirs[3]);

	if((dfile = fopen(fname, "a")) == NULL)
		logsyserr("<get_config> error opening %s", fname);

	time(&now);
	timep = localtime(&now);
	fprintf(dfile, "[%02d/%02d/%02d %02d:%02d:%02d] %s\n",
	                  timep->tm_mon + 1, timep->tm_mday, timep->tm_year,
					  timep->tm_hour, timep->tm_min, timep->tm_sec,
					  buf);

	fflush(dfile);

	fclose(dfile);
	return;
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

err_exit(errcode)
int errcode;
{
	loginfo("<err_exit> (PID %d) Fatal Error", getpid());
    kill(getpid(), SIGTERM);
    sleep(5);
    loginfo("<err_exit> didn't catch KILL() ???");
    exit(errcode);
}
