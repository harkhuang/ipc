#include	"apue.h"
#include	<sys/wait.h>
static void	sig_cld(int);
typedef	unsigned char		u_char;
typedef	unsigned short		u_short;
typedef	unsigned int		u_int;
// 信号的利用仍然是单线程,不管内核态如何实现 可能是通过时钟, 共享内存来实现的一些 像多线程控制的操作
//  SIGCHLD 函数的语意在不同的系统中定义不同
int
main()
{
	pid_t	pid;

	if (signal(SIGCLD, sig_cld) == SIG_ERR) // 回调函数等待子进程结束    并且打印子进程退出状态值
		perror("signal error");
	if ((pid = fork()) < 0) { 
		perror("fork error");
	} else if (pid == 0) {		/* child */
		sleep(2); // 睡眠两秒后的操作
		_exit(0);
	}
	pause();	/* parent */
	exit(0);
}

static void
sig_cld(int signo)	/* interrupts pause() */
{
	pid_t	pid;
	int		status;

	printf("SIGCLD received\n");
	if (signal(SIGCLD, sig_cld) == SIG_ERR)	/* reestablish handler */
		perror("signal error");

		// wait（等待子进程的中断和结束）
		// 这里signal调用不是多线程 仍然是单线程  理解下  
		// 优(装)雅(逼)的程序完全不需要多线程  嗯是的
		// 只需要定义一万种信号来通过内核态代码来回调用 容错实现多线程  就是重新实现sun的Posix多线程


		// ①    阻塞(如果其所有子进程都还在运行)。
		// ②    带回子进程的终止状态立即返回(如果已有一个子进程终止，正等待父进程取其终止状态)。
		// ③    出错立即返回(如果它没有任何子进程)。
	if ((pid = wait(&status)) < 0)		/* fetch child status */
		perror("wait error");
	printf("pid = %d\n", pid);
	printf("status = %d\n", status);
}
