#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>

static void	sig_int(int);

#define err_sys(fmt, args...)  printf(fmt, ##args)

// 信号集   探测函数
void
pr_mask(const char *str)
{
	sigset_t	sigset;//  保存信号掩码的结构体
	int			errno_save; //   



	// 保存当前系统的错误码  由于信号的存在可能存在多种内核调用问题出现   用此errno记录
	errno_save = errno;		/* we can be called by signal handlers */
	if (sigprocmask(0, NULL, &sigset) < 0)  // 掩码的容错处理 取掩码到sigset里面去哦
		printf("sigprocmask error");

	printf("%s", str);

	// 探测信号集中是否包含此函数
	if (sigismember(&sigset, SIGINT))   printf("SIGINT ");  //逐个探测 信号是否在该信号集中存在
	if (sigismember(&sigset, SIGQUIT))  printf("SIGQUIT ");
	if (sigismember(&sigset, SIGUSR1))  printf("SIGUSR1 ");
	if (sigismember(&sigset, SIGALRM))  printf("SIGALRM ");

	/* remaining signals can go here  */

	printf("\n");
	errno = errno_save;
}


int
main(void)
{

	// 其实sigset_t define long long  sigset_t
	sigset_t	newmask, oldmask, waitmask; // 定义几个需要初始化的信号

	pr_mask("program start: ");  // 探测当前进程信号集

	if (signal(SIGINT, sig_int) == SIG_ERR)
		err_sys("signal(SIGINT) error");
	sigemptyset(&waitmask);
	sigaddset(&waitmask, SIGUSR1);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGINT);

	/*
	 * Block SIGINT and save current signal mask.
	 */
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		err_sys("SIG_BLOCK error");

	/*
	 * Critical region of code.
	 */
	pr_mask("in critical region: ");

	/*
	 * Pause, allowing all signals except SIGUSR1.
	 */
	if (sigsuspend(&waitmask) != -1)
		err_sys("sigsuspend error");

	pr_mask("after return from sigsuspend: ");

	/*
	 * Reset signal mask which unblocks SIGINT.
	 */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("SIG_SETMASK error");

	/*
	 * And continue processing ...
	 */
	pr_mask("program exit: ");

	exit(0);
}

static void
sig_int(int signo)
{
	pr_mask("\nin sig_int: ");
}
