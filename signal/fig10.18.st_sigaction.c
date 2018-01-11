#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
 
#define err_sys(fmt, args...)  printf(fmt, ##args)
// 模拟signal函数
typedef	void	Sigfunc(int);


// 关心信号   关心信号掩码  关心信号定义的行为

/**
 * 1. sigaction是以什么为单位的?  以信号整体为单位  不局限于进程和线程
 */
/* Reliable(可靠的) version of signal(), using POSIX sigaction().  */
Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;
	// 1. sigaciton 的初始化

	// 设置信号返回函数 清空的信号
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);  // 通常我们将掩码置空  表示不屏蔽任何系统内核级别的信号处理操作  表示我没呢应用没有其他关心的信号
	act.sa_flags = 0;

	// 设置关心信号
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}

	// 吧新的sigaction 写入  保留旧的信号行为
	// 2. 覆盖sigaction  保留旧的信号 
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}

 