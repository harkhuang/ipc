#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>



// 模拟signal函数
typedef	void	Sigfunc(int);

/* Reliable version of signal(), using POSIX sigaction().  */
Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;


// 设置信号返回函数
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
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
	// sigaction 的引入让信号变得更加生动
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}


int main(){
	return 0;
}