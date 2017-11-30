/*
 * Copyright (c) 1993 W. Richard Stevens.  All rights reserved.
 * Permission to use or modify this software and its documentation only for
 * educational purposes and without fee is hereby granted, provided that
 * the above copyright notice appear in all copies.  The author makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 */

#include	"unp.h"

#include	<sys/uio.h>
#ifdef	__bsdi__
#include	<machine/endian.h>	/* required before tcp.h, for BYTE_ORDER */
#endif
#include	<netinet/tcp.h>		/* TCP_NODELAY */
#include	<netdb.h>			/* getservbyname(), gethostbyname() */

				/* declare global variables */
extern int		bindport;
extern int		broadcast;
extern int		cbreak;
extern int		chunkwrite;
extern int		client;
extern int		connectudp;
extern int		crlf;
extern int		debug;
extern int		dofork;
extern int		dontroute;
extern char		foreignip[];
extern int		foreignport;
extern int		halfclose;
extern int		ignorewerr;
extern int		iptos;
extern int		ipttl;
extern char		joinip[];
extern int		keepalive;
extern long		linger;
extern int		listenq;
extern char		localip[];
extern int		maxseg;
extern int		mcastttl;
extern int		msgpeek;
extern int		nodelay;
extern int		nbuf;
extern int		onesbcast;
extern int		pauseclose;
extern int		pauseinit;
extern int		pauselisten;
extern int		pauserw;
extern int		reuseaddr;
extern int		reuseport;
extern int		readlen;
extern int		writelen;
extern int		recvdstaddr;
extern int		rcvbuflen;
extern int		sndbuflen;
extern long		rcvtimeo;
extern long		sndtimeo;
extern char	   *rbuf;
extern char	   *wbuf;
extern int		server;
extern int		sigio;
extern int		sourcesink;
extern int		sroute_cnt;
extern int		udp;
extern int		urgwrite;
extern int		verbose;
extern int		usewritev;

extern struct sockaddr_in	cliaddr, servaddr;

/* Earlier versions of gcc under SunOS 4.x have problems passing arguments
   that are structs (as opposed to pointers to structs).  This shows up
   with inet_ntoa, whose argument is a "struct in_addr". */

#if	defined(sun) && defined(__GNUC__) && defined(GCC_STRUCT_PROBLEM)
#define	INET_NTOA(foo)	inet_ntoa(&foo)
#else
#define	INET_NTOA(foo)	inet_ntoa(foo)
#endif

				/* function prototypes */
void	buffers(int);
int		cliopen(char *, char *);
int		crlf_add(char *, int, const char *, int);
int		crlf_strip(char *, int, const char *, int);
void	join_mcast(int, struct sockaddr_in *);
void	loop_tcp(int);
void	loop_udp(int);
void	pattern(char *, int);
int		servopen(char *, char *);
void	sink_tcp(int);
void	sink_udp(int);
void	source_tcp(int);
void	source_udp(int);
void	sroute_doopt(int, char *);
void	sroute_set(int);
void	sleep_us(unsigned int);
void	sockopts(int, int);
ssize_t	dowrite(int, const void *, size_t);

void	TELL_WAIT(void);
void	TELL_PARENT(pid_t);
void	WAIT_PARENT(void);
void	TELL_CHILD(pid_t);
void	WAIT_CHILD(void);

int
crlf_add(char *dst, int dstsize, const char *src, int lenin)
{
	int 	lenout;
	char	c;

	if ( (lenout = lenin) > dstsize)
			err_quit("crlf_add: destination not big enough");

	for ( ; lenin > 0; lenin--) {
		if ( (c = *src++) == '\n') {
			if (++lenout >= dstsize)
				err_quit("crlf_add: destination not big enough");
			*dst++ = '\r';
		}
		*dst++ = c;
	}

	return(lenout);
}

int
crlf_strip(char *dst, int dstsize, const char *src, int lenin)
{
	int		lenout;
	char	c;

	for (lenout = 0; lenin > 0; lenin--) { 
		if ( (c = *src++) != '\r') {
			if (++lenout >= dstsize)
				err_quit("crlf_strip: destination not big enough");
			*dst++ = c;
		}
	}

	return(lenout);
}





#include <errno.h>		/* for definition of errno */
#include <stdarg.h>		/* ISO C variable aruments */

static void	err_doit(int, int, const char *, va_list);

/*
 * Nonfatal error related to a system call.
 * Print a message and return.
 */
void
err_ret(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void
err_sys(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

/*
 * Fatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and terminate.
 */
void
err_exit(int error, const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, error, fmt, ap);
	va_end(ap);
	exit(1);
}

/*
 * Fatal error related to a system call.
 * Print a message, dump core, and terminate.
 */
void
err_dump(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	abort();		/* dump core and terminate */
	exit(1);		/* shouldn't get here */
}

/*
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void
err_msg(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void
err_quit(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag".
 */
static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char	buf[MAXLINE];

	vsnprintf(buf, MAXLINE, fmt, ap);
	if (errnoflag)
		snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s",
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

	ptr = vptr;	/* can't do pointer arithmetic on void* */
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
			return(nwritten);		/* error */

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
