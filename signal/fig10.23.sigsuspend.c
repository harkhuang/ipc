#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>

#define err_sys(fmt, args...)  printf(fmt, ##args)



// 当把变量声明为该类型是，则会保证该变量在使用或赋值时， 无论是在32位还是64位的机器上都能保证操作是原子的， 它会根据机器的类型自动适应。
volatile sig_atomic_t	quitflag;	/* set nonzero by signal handler */

static void
sig_int(int signo)	/* one signal handler for SIGINT and SIGQUIT */
{
	if (signo == SIGINT)
		printf("\ninterrupt\n");
	else if (signo == SIGQUIT)
		quitflag = 1;	/* set flag for main loop */
}

int
main23(void)
{
	sigset_t	newmask, oldmask, zeromask;  //  定义 old信号集   new信号集  0信号集
 
	if (signal(SIGINT, sig_int) == SIG_ERR)  //  定义信号捕获动作  和 容错
		err_sys("signal(SIGINT) error");
	if (signal(SIGQUIT, sig_int) == SIG_ERR)   // 定义信号捕获动作和  容错
		err_sys("signal(SIGQUIT) error");

	sigemptyset(&zeromask);     // 初始化清空信号集
	sigemptyset(&newmask);      // 初始化清空信号集
	sigaddset(&newmask, SIGQUIT); // 添加 SIGQUIT到new信号集

	/*
	 * Block SIGQUIT and save current signal mask.
	 */
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)   //设置新信号集 为阻塞  并且保存了旧信号集
		err_sys("SIG_BLOCK error");

	while (quitflag == 0) // 当钱进程关心的信号集 都被处理完成
		sigsuspend(&zeromask);   // !!!!!这是一个能原子级别恢复信号的操作  不会影响信号的稳定性  表示 allow all singals and  pause

	/*
	 * SIGQUIT has been caught and is now blocked; do whatever.
	 */
	quitflag = 0;

	/*
	 * Reset signal mask which unblocks SIGQUIT.
	 */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("SIG_SETMASK error");

	exit(0);
}
