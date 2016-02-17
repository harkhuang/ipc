#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

#define	TTY1	"/dev/tty11"
#define	TTY2	"/dev/tty12"

enum {
	STATE_R=1,
	STATE_W,
	STATE_Ex,
	STATE_T
};

#define	BUFSIZE	1024
struct fsa_relay_st {
	int state;
	int sfd, dfd;
	char buf[BUFSIZE];
	int len, pos;
	char *errstr;
};



//define my file driver 
static void fsa_driver(struct fsa_relay_st *fsa)
{
	int ret;

	switch (fsa->state) {
		case STATE_R:
			fsa->len = read(fsa->sfd, fsa->buf, BUFSIZE);
			if (fsa->len==0) {
				fsa->state = STATE_T;
			} else if (fsa->len<0) {
				if (errno==EAGAIN) {
					/* Do nothing */
				} else {
					fsa->errstr = "read(sfd)";
					fsa->state = STATE_Ex;
				}
			} else {
				fsa->pos = 0;
				fsa->state = STATE_W;
			}
			break;
		case STATE_W:
			ret = write(fsa->dfd, fsa->buf+fsa->pos, fsa->len);
			if (ret<0) {
				if (errno==EAGAIN) {
					/* Do nothing */
				} else {
					fsa->errstr = "write(dfd)";
					fsa->state = STATE_Ex;
				}
			} else {
				fsa->pos += ret;
				fsa->len -= ret;
				if (fsa->len>0) {
					/* Do nothing */
				} else {
					fsa->state = STATE_R;
				}
			}
			break;
		case STATE_Ex:
			perror(fsa->errstr);
			fsa->state = STATE_T;
			break;
		case STATE_T:
			/* Do nothing */
			break;
		default:
			raise(SIGABRT);
			break;
	}
}

//simple match 
static int max(int a, int b)
{
	if (a<b) {
		return b;
	}
	return a;
}

//中继器   
//define myself file to read or write  
//two file descrpte to  write or read 
static void relay(int fd1, int fd2)
{
	struct fsa_relay_st fsa1, fsa2;
	int fd1_saveflag, fd2_saveflag;


	/* this struct is very important*/
	struct pollfd pfd[2];//this is a pollfd struct 

	fd1_saveflag = fcntl(fd1, F_GETFL);//get file  flag 
	fcntl(fd1, F_SETFL, fd1_saveflag|O_NONBLOCK);//set file flag
	fd2_saveflag = fcntl(fd2, F_GETFL);//get file flag 
	fcntl(fd2, F_SETFL, fd2_saveflag|O_NONBLOCK);//set file flag

	//init file description to read able
	fsa1.sfd = fd1;
	fsa1.dfd = fd2;
	fsa1.state = STATE_R;

	fsa2.sfd = fd2;
	fsa2.dfd = fd1;
	fsa2.state = STATE_R;
	
	//realy  routine 
	while (fsa1.state!=STATE_T || fsa2.state!=STATE_T) {
		pfd[0].fd = fd1;
		pfd[0].events = 0;
		pfd[1].fd = fd2;
		pfd[1].events = 0;
		if (fsa1.state==STATE_R) {
			pfd[0].events |= POLLIN;
		}
		if (fsa2.state==STATE_W) {
			pfd[0].events |= POLLOUT;
		}

		if (fsa1.state==STATE_W) {
			pfd[1].events |= POLLOUT;
		}
		if (fsa2.state==STATE_R) {
			pfd[1].events |= POLLIN;
		}



	/*
		EPOLLLT——水平触发
		EPOLLET——边缘触发
		epoll有EPOLLLT和EPOLLET两种触发模式，
		LT是默认的模式，ET是“高速”模式。LT模式下，
		只要这个fd还有数据可读，
		每次 epoll_wait都会返回它的事件，
		提醒用户程序去操作，而在ET（边缘触发）模式中，
		它只会提示一次，直到下次再有数据流入之前都不会再提示了，
		无 论fd中是否还有数据可读。所以在ET模式下，
		read一个fd的时候一定要把它的buffer读光，
		也就是说一直读到read的返回值小于请求值，或者 遇到EAGAIN错误。
	*/
		// wait to do someting  
		// use poll 
		while (poll(pfd, 2, -1)<0) {
			if (errno!=EINTR) {
				perror("poll()");
				exit(1);
			} else {
				continue;
			}
		}
		//if fd1 change do fsa_driver 
		if (pfd[0].revents&POLLIN || pfd[1].revents&POLLOUT) {
			fsa_driver(&fsa1);
		}
		//if fd2 change do fsa_driver
		if (pfd[1].revents&POLLIN || pfd[0].revents&POLLOUT) {
			fsa_driver(&fsa2);
		}
	}
	// restore the file description 
	fcntl(fd1, F_SETFL, fd1_saveflag);
	fcntl(fd2, F_SETFL, fd2_saveflag);
}

int
main()
{
	int fd1, fd2;

	fd1 = open(TTY1, O_RDWR);
	if (fd1<0) {
		perror("open()");
		exit(1);
	}
	write(fd1, "TTY1\n", 5);

	fd2 = open(TTY2, O_RDWR|O_NONBLOCK);
	if (fd2<0) {
		perror("open()");
		exit(1);
	}
	write(fd2, "TTY2\n", 5);

	
	relay(fd1, fd2);

	close(fd2);
	close(fd1);


	return 0;
}

