#include <sys/types.h>		/* some systems still require this */
#include <sys/stat.h>
#include <sys/termios.h>	/* for winsize */
#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif
#include <stdio.h>		/* for convenience */
#include <stdlib.h>		/* for convenience */
#include <stddef.h>		/* for offsetof */
#include <string.h>		/* for convenience */
#include <unistd.h>		/* for convenience */
#include <signal.h>		/* for SIG_ERR */
#include <sys/types.h>
#include <pwd.h>
#include <uuid/uuid.h>
#include <stdarg.h>

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char	buf[11];

	vsnprintf(buf, 11, fmt, ap);
	if (errnoflag)
		snprintf(buf+strlen(buf), 11-strlen(buf), ": %s",
		  strerror(error));
	strcat(buf, "\n");
	fflush(stdout);		/* in case stdout and stderr are the same */
	fputs(buf, stderr);
	fflush(NULL);		/* flushes all stdio output streams */
}


ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft, nwritten;
	const char	*ptr;

	ptr = (char *)vptr;	/* can't do pointer arithmetic on void* */
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
			return(nwritten);		/* error */

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}


void
err_sys(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, 1, fmt, ap);
	va_end(ap);
	exit(1);
}


static void
my_alarm(int signo)
{
	struct passwd	*rootptr;

	printf("in signal handler\n");
	if ((rootptr = getpwnam("root")) == NULL)
			err_sys("getpwnam(root) error");
	alarm(1);
}

int main(void)
{
	struct passwd	*ptr;
	signal(SIGALRM, my_alarm);
	alarm(1);

	for ( ; ; ) {
		if ((ptr = getpwnam("sar")) == NULL)
			err_sys("getpwnam error");
		if (strcmp(ptr->pw_name, "sar") != 0)
			printf("return value corrupted!, pw_name = %s\n", ptr->pw_name);
	}	
}
