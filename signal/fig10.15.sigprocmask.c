#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#define   DEBUG_SWITCH 1

#ifdef DEBUG_SWITCH
#define err_sys(fmt, args...)  printf(fmt, ##args)
#else
#define err_sys(fmt, args...) 
#endif

static void	sig_quit(int);

static void
sig_quit(int signo)
{
	printf("caught SIGQUIT\n");
	if (signal(SIGQUIT, SIG_DFL) == SIG_ERR)
		err_sys("can't reset SIGQUIT");
}

int  main(void)
{
	sigset_t	newmask, oldmask, pendmask;

	if (signal(SIGQUIT, sig_quit) == SIG_ERR)
		err_sys("can't catch SIGQUIT");

	/*
	 * Block SIGQUIT and save current signal mask.
	 */
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGQUIT);

	// 这个函数的实现能保存信号暂时的状态  并且进行另外一种信号通信改变系统
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)   // 设置新的信号集为BLOCK   保存旧信号集 
		err_sys("SIG_BLOCK error");

	sleep(5);	/* SIGQUIT here will remain pending */

	if (sigpending(&pendmask) < 0)  //有点困惑这个函数
		err_sys("sigpending error");
	if (sigismember(&pendmask, SIGQUIT))
		printf("\nSIGQUIT pending\n");

	/*
	 * Reset signal mask which unblocks SIGQUIT.
	 */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("SIG_SETMASK error");
	printf("SIGQUIT unblocked\n");

	sleep(5);	/* SIGQUIT here will terminate with core file */
	exit(0);
}
