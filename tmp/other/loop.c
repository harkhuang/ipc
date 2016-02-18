/*
 * Copyright (c) 1993 W. Richard Stevens.  All rights reserved.
 * Permission to use or modify this software and its documentation only for
 * educational purposes and without fee is hereby granted, provided that
 * the above copyright notice appear in all copies.  The author makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 */

//#include	"sock.h"
#include    <unistd.h>
#include    <stdio.h>
#include <sys/select.h> 
#include <sys/time.h>



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

 


/* Copy everything from stdin to "sockfd",
 * and everything from "sockfd" to stdout. */

void	tty_atexit(void);	/* in library */
void	sig_catch(int);		/* my function */

 
/*
 * Copyright (c) 1993 W. Richard Stevens.  All rights reserved.
 * Permission to use or modify this software and its documentation only for
 * educational purposes and without fee is hereby granted, provided that
 * the above copyright notice appear in all copies.  The author makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 */

#include	"sock.h"

/* Convert newline to return/newline. */

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



void
loop(int sockfd)
{


	//add temp value 
  int		bindport;
  int		broadcast;
  int		cbreak;
  int		chunkwrite;
  int		client;
  int		connectudp;
  int		crlf;
  int		debug;
  int		dofork;
  int		dontroute;
  char		foreignip[128];
  int		foreignport;
  int		halfclose;
  int		ignorewerr;
  int		iptos;
  int		ipttl;
  char		joinip[128];
  int		keepalive;
  long		linger;
  int		listenq;
  char		localip[128];
  int		maxseg;
  int		mcastttl;
  int		msgpeek;
  int		nodelay;
  int		nbuf;
  int		onesbcast;
  int		pauseclose;
  int		pauseinit;
  int		pauselisten;
  int		pauserw;
  int		reuseaddr;
  int		reuseport;
  int		readlen;
  int		writelen;
  int		recvdstaddr;
  int		rcvbuflen;
  int		sndbuflen;
  long		rcvtimeo;
  long		sndtimeo;
  char	   *rbuf;
  char	   *wbuf;
  int		server;
  int		sigio;
  int		sourcesink;
  int		sroute_cnt;
  int		udp;
  int		urgwrite;
  int		verbose;
  int		usewritev;

  struct sockaddr_in	 servaddr;




	int						maxfdp1, nread, ntowrite, stdineof, clilen;
	fd_set					rset;
	struct sockaddr_in		cliaddr;		/* for UDP server */

#ifdef	MSG_TRUNC			/* 4.3BSD Reno and later */
	struct iovec			iov[1];
	struct msghdr			msg;

#ifdef	IP_RECVDSTADDR		/* 4.3BSD Reno and later */
	static struct cmsghdr  *cmptr = NULL;	/* malloc'ed */
	struct in_addr			dstinaddr;		/* for UDP server */
#define	CONTROLLEN	(sizeof(struct cmsghdr) + sizeof(struct in_addr))
#endif	/* IP_RECVDSTADDR */

#endif	/* MSG_TRUNC */

#ifdef	notdef	/* following doesn't appear to work */
	/*
	 * This is an attempt to set stdin to cbreak, so that input characters
	 * are delivered one at a time, to see Nagle algorithm in effect
	 * (or disabled).
	 */
	if (cbreak && isatty(STDIN_FILENO)) {
		if (tty_cbreak(STDIN_FILENO) < 0)
			err_sys("tty_cbreak error");
		if (atexit(tty_atexit) < 0)
			err_sys("tty_atexit error");

		if (signal(SIGINT, sig_catch) == SIG_ERR)
			err_sys("signal error");
		if (signal(SIGQUIT, sig_catch) == SIG_ERR)
			err_sys("signal error");
		if (signal(SIGTERM, sig_catch) == SIG_ERR)
			err_sys("signal error");
	}
#endif

	if (pauseinit)
		sleep(pauseinit);	/* intended for server */

	stdineof = 0;
	FD_ZERO(&rset);
	maxfdp1 = sockfd + 1;	/* check descriptors [0..sockfd] */

		/* UDP client issues connect(), so read() and write() are used.
		   Server is harder since cannot issue connect().  We use recvfrom()
		   or recvmsg(), depending on OS. */

	for ( ; ; ) {
		if (stdineof == 0)
			FD_SET(STDIN_FILENO, &rset);
		FD_SET(sockfd, &rset);

		if (select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
			err_sys("select error");

		if (FD_ISSET(STDIN_FILENO, &rset)) {	/* data to read on stdin */
			if ( (nread = read(STDIN_FILENO, rbuf, readlen)) < 0)
				err_sys("read error from stdin");
			else if (nread == 0) {	/* EOF on stdin */
				if (halfclose) {
					if (shutdown(sockfd, 1) < 0)
						err_sys("shutdown() error");

					FD_CLR(STDIN_FILENO, &rset);
					stdineof = 1;	/* don't read stdin anymore */
					continue;		/* back to select() */
				}
				break;		/* default: stdin EOF -> done */
			}

			if (crlf) {
				ntowrite = crlf_add(wbuf, writelen, rbuf, nread);
				if (write(sockfd, wbuf, ntowrite) != ntowrite)
					err_sys("write error");
			} else {
				if (write(sockfd, rbuf, nread) != nread)
					err_sys("write error");
			}
		}

		if (FD_ISSET(sockfd, &rset)) {	/* data to read from socket */
			if (udp && server) {
				clilen = sizeof(cliaddr);
#ifndef	MSG_TRUNC	/* vanilla BSD sockets */
				nread = recvfrom(sockfd, rbuf, readlen, 0,
									(struct sockaddr *) &cliaddr, &clilen);

#else	/* 4.3BSD Reno and later; use recvmsg() to get at MSG_TRUNC flag */
		/* Also lets us get at control information (destination address) */


				iov[0].iov_base = rbuf;
				iov[0].iov_len  = readlen;
				msg.msg_iov          = iov;
				msg.msg_iovlen       = 1;
				msg.msg_name         = (caddr_t) &cliaddr;
				msg.msg_namelen      = clilen;

#ifdef	IP_RECVDSTADDR
				if (cmptr == NULL && (cmptr = malloc(CONTROLLEN)) == NULL)
					err_sys("malloc error for control buffer");

				msg.msg_control      = (caddr_t) cmptr;	/* for dest address */
				msg.msg_controllen   = CONTROLLEN;
#else
				msg.msg_control      = (caddr_t) 0;	/* no ancillary data */
				msg.msg_controllen   = 0;
#endif	/* IP_RECVDSTADDR */
				msg.msg_flags        = 0;			/* flags returned here */

				nread = recvmsg(sockfd, &msg, 0);
#endif	/* MSG_TRUNC */
				if (nread < 0)
					err_sys("datagram receive error");

				if (verbose) {
					printf("from %s", INET_NTOA(cliaddr.sin_addr));
#ifdef	MSG_TRUNC
#ifdef	IP_RECVDSTADDR
					if (recvdstaddr) {
						if (cmptr->cmsg_level != IPPROTO_IP)
							err_quit("control level != IPPROTO_IP");
						if (cmptr->cmsg_type != IP_RECVDSTADDR)
							err_quit("control type != IP_RECVDSTADDR");
						if (cmptr->cmsg_len != CONTROLLEN)
							err_quit("control length (%d) != %d",
									 cmptr->cmsg_len, CONTROLLEN);
						memcpy((char *) &dstinaddr, (char *) CMSG_DATA(cmptr),
							  sizeof(struct in_addr));

						printf(", to %s", INET_NTOA(dstinaddr));
					}
#endif	/* IP_RECVDSTADDR */
#endif	/* MSG_TRUNC */
					printf(": ");
					fflush(stdout);
				}

#ifdef	MSG_TRUNC
				if (msg.msg_flags & MSG_TRUNC)
					printf("(datagram truncated)\n");
#endif

			} else {
				if ( (nread = read(sockfd, rbuf, readlen)) < 0)
					err_sys("read error");
				else if (nread == 0) {
					if (verbose)
						fprintf(stderr, "connection closed by peer\n");
					break;		/* EOF, terminate */
				}
			}

			if (crlf) {
				ntowrite = crlf_strip(wbuf, writelen, rbuf, nread);
				if (writen(STDOUT_FILENO, wbuf, ntowrite) != ntowrite)
					err_sys("writen error to stdout");
			} else {
				if (writen(STDOUT_FILENO, rbuf, nread) != nread)
					err_sys("writen error to stdout");
			}
		}
	}

	if (pauseclose) {
		if (verbose)
				fprintf(stderr, "pausing before close\n");
		sleep(pauseclose);
	}

	if (close(sockfd) < 0)
		err_sys("close error");		/* since SO_LINGER may be set */
}

void
sig_catch(int signo)
{
	exit(0);	/* exit handler will reset tty state */
}


int main()
{


	return 0;
}