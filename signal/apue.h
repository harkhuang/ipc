/* Our own header, to be included before all standard system headers */

#ifndef	_APUE_H
#define	_APUE_H

#if defined(SOLARIS)
#define _XOPEN_SOURCE	500	/* Single UNIX Specification, Version 2  for Solaris 9 */
#define CMSG_LEN(x)	_CMSG_DATA_ALIGN(sizeof(struct cmsghdr)+(x))
#elif !defined(BSD)
#define _XOPEN_SOURCE	600	/* Single UNIX Specification, Version 3 */
#endif

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

#define	MAXLINE	4096			/* max line length */

/*
 * Default file access permissions for new files.
 */
#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
 * Default permissions for new directories.
 */
#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

typedef	void	Sigfunc(int);	/* for signal handlers */

#if	defined(SIG_IGN) && !defined(SIG_ERR)
#define	SIG_ERR	((Sigfunc *)-1)
#endif

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

/*
 * Prototypes for our own functions.
 */
char	*path_alloc(int *);				/* {Prog pathalloc} */
long	 open_max(void);				/* {Prog openmax} */
void	 clr_fl(int, int);				/* {Prog setfl} */
void	 set_fl(int, int);				/* {Prog setfl} */
void	 pr_exit(int);					/* {Prog prexit} */
void	 pr_mask(const char *);			/* {Prog prmask} */
Sigfunc	*signal_intr(int, Sigfunc *);	/* {Prog signal_intr_function} */

int		 tty_cbreak(int);				/* {Prog raw} */
int		 tty_raw(int);					/* {Prog raw} */
int		 tty_reset(int);				/* {Prog raw} */
void	 tty_atexit(void);				/* {Prog raw} */
#ifdef	ECHO	/* only if <termios.h> has been included */
struct termios	*tty_termios(void);		/* {Prog raw} */
#endif

void	 sleep_us(unsigned int);			/* {Ex sleepus} */
ssize_t	 readn(int, void *, size_t);		/* {Prog readn_writen} */
ssize_t	 writen(int, const void *, size_t);	/* {Prog readn_writen} */
void	 daemonize(const char *);			/* {Prog daemoninit} */

int		 s_pipe(int *);					/* {Progs streams_spipe sock_spipe} */
int		 recv_fd(int, ssize_t (*func)(int,
		         const void *, size_t));/* {Progs recvfd_streams recvfd_sockets} */
int		 send_fd(int, int);				/* {Progs sendfd_streams sendfd_sockets} */
int		 send_err(int, int,
		          const char *);		/* {Prog senderr} */
int		 serv_listen(const char *);		/* {Progs servlisten_streams servlisten_sockets} */
int		 serv_accept(int, uid_t *);		/* {Progs servaccept_streams servaccept_sockets} */
int		 cli_conn(const char *);		/* {Progs cliconn_streams cliconn_sockets} */
int		 buf_args(char *, int (*func)(int,
		          char **));			/* {Prog bufargs} */

int		 ptym_open(char *, int);	/* {Progs3 ptyopen_streams ptyopen_bsd ptyopen_linux} */
int		 ptys_open(char *);			/* {Progs3 ptyopen_streams ptyopen_bsd ptyopen_linux} */
#ifdef	TIOCGWINSZ
pid_t	 pty_fork(int *, char *, int, const struct termios *,
		          const struct winsize *);		/* {Prog ptyfork} */
#endif

int		lock_reg(int, int, int, off_t, int, off_t); /* {Prog lockreg} */
#define	read_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define	readw_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define	write_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define	writew_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define	un_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))

pid_t	lock_test(int, int, off_t, int, off_t);		/* {Prog locktest} */

#define	is_read_lockable(fd, offset, whence, len) \
			(lock_test((fd), F_RDLCK, (offset), (whence), (len)) == 0)
#define	is_write_lockable(fd, offset, whence, len) \
			(lock_test((fd), F_WRLCK, (offset), (whence), (len)) == 0)

void	err_dump(const char *, ...);		/* {App misc_source} */
void	err_msg(const char *, ...);
void	err_quit(const char *, ...);
void	err_exit(int, const char *, ...);
void	err_ret(const char *, ...);
void	err_sys(const char *, ...);

void	log_msg(const char *, ...);			/* {App misc_source} */
void	log_open(const char *, int, int);
void	log_quit(const char *, ...);
void	log_ret(const char *, ...);
void	log_sys(const char *, ...);

void	TELL_WAIT(void);		/* parent/child from {Sec race_conditions} */
void	TELL_PARENT(pid_t);
void	TELL_CHILD(pid_t);
void	WAIT_PARENT(void);
void	WAIT_CHILD(void);
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

#endif	/* _APUE_H */
