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













// 不可重入函数getpwnam在使用signal函数时候的一些不可预期的调用
// 不安全的调用会引起一些不期望的预期发生
//上面都是一些容错 重点看下面这部分代码


////输出
//get hk name ok
//in signal handler
//get root name ok
//return value corrupted!, pw_name = root
//get hk name ok
//in signal handler
//get root name ok
//return value corrupted!, pw_name = root
//get hk name ok
//in signal handler
//get root name ok
//return value corrupted!, pw_name = root
//get hk name ok

static void
my_alarm(int signo)
{
	struct passwd	*rootptr; //定义请求密码的系统调用的函数返回结构体

	printf("in signal handler\n"); // 打印我正在回调函数
	if ((rootptr = getpwnam("root")) == NULL) //  root密码查询
	{
		err_sys("getpwnam(root) error");//错误输出
	}
	else {
		
		printf("get root name ok\n");
	}

	alarm(2);    //  信号回调中再次发送信号 此处相当于用信号做了循环
}


// 程序目的  回调和主调同事调用不安全的函数getpwnam获取不同的结果
int main(void)
{
	struct passwd	*ptr;  
	signal(SIGALRM, my_alarm);  

	
	alarm(2);

	for ( ; ; ) {   
		if ((ptr = getpwnam("hk")) == NULL){
			err_sys("getpwnam error");
		}else{
			printf("get hk name ok\n");
		}
			
		sleep(2);

		// 意图是打印hk的内容  但是却打印了root的内容  因为信号回调到调用了共享的数据
		if (strcmp(ptr->pw_name, "hk") != 0)
			printf("return value corrupted!, pw_name = %s\n", ptr->pw_name);
			else{
				//printf("hk return value = 0 \n");
			}
	}	
}
