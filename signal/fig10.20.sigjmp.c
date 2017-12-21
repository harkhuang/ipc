#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>

#define err_sys(fmt, args...)  printf(fmt, ##args)

static void						sig_usr1(int), sig_alrm(int);
static sigjmp_buf				jmpbuf;
static volatile sig_atomic_t	canjump;


// 信号集探测函数
void
pr_mask(const char *str)
{
	sigset_t	sigset;//  保存信号掩码的结构体
	int			errno_save; // 

	errno_save = errno;		/* we can be called by signal handlers */
	if (sigprocmask(0, NULL, &sigset) < 0)  // 掩码的容错处理
		printf("sigprocmask error");

	printf("%s", str);

	// 探测信号集中是否包含此函数
	if (sigismember(&sigset, SIGINT))   printf("SIGINT ");
	if (sigismember(&sigset, SIGQUIT))  printf("SIGQUIT ");
	if (sigismember(&sigset, SIGUSR1))  printf("SIGUSR1 ");
	if (sigismember(&sigset, SIGALRM))  printf("SIGALRM ");

	/* remaining signals can go here  */

	printf("\n");
	errno = errno_save;
}

static void sig_usr1(int signo)
{
	time_t	starttime;   // 时间变量结构体

	if (canjump == 0)   // 未知信号   做忽略处理
		return;		/* unexpected signal, ignore */

	pr_mask("starting sig_usr1: ");   // 探测usr1时信号集
	alarm(3);				/* SIGALRM in 3 seconds */  
	starttime = time(NULL);
	for ( ; ; )				/* busy wait for 5 seconds */
		if (time(NULL) > starttime + 5)
			break;
	pr_mask("finishing sig_usr1: ");

	canjump = 0;
	siglongjmp(jmpbuf, 1);	/* jump back to main, don't return */
}



static void
sig_alrm(int signo)
{
	pr_mask("in sig_alrm: ");
}


int
main(void)
{
	if (signal(SIGUSR1, sig_usr1) == SIG_ERR)
		err_sys("signal(SIGUSR1) error");
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)   // 时钟信号做默认探测处理
		err_sys("signal(SIGALRM) error");
	pr_mask("starting main: ");		/* {Prog prmask} */

	if (sigsetjmp(jmpbuf, 1)) {
		pr_mask("ending main: ");
		exit(0);
	}
	canjump = 1;	/* now sigsetjmp() is OK */

	for ( ; ; )
		pause();
}
