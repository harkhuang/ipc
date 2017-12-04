#include "apue.h"

static void	sig_usr(int);	/* one handler for both signals */
static void	sig_usr1( int);

/*

hk@ubuntu14:/mnt/ipcframe/signal$ kill -l
 1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL       5) SIGTRAP
 6) SIGABRT      7) SIGBUS       8) SIGFPE       9) SIGKILL     10) SIGUSR1
11) SIGSEGV     12) SIGUSR2     13) SIGPIPE     14) SIGALRM     15) SIGTERM
16) SIGSTKFLT   17) SIGCHLD     18) SIGCONT     19) SIGSTOP     20) SIGTSTP
21) SIGTTIN     22) SIGTTOU     23) SIGURG      24) SIGXCPU     25) SIGXFSZ
26) SIGVTALRM   27) SIGPROF     28) SIGWINCH    29) SIGIO       30) SIGPWR
31) SIGSYS      34) SIGRTMIN    35) SIGRTMIN+1  36) SIGRTMIN+2  37) SIGRTMIN+3
38) SIGRTMIN+4  39) SIGRTMIN+5  40) SIGRTMIN+6  41) SIGRTMIN+7  42) SIGRTMIN+8
43) SIGRTMIN+9  44) SIGRTMIN+10 45) SIGRTMIN+11 46) SIGRTMIN+12 47) SIGRTMIN+13
48) SIGRTMIN+14 49) SIGRTMIN+15 50) SIGRTMAX-14 51) SIGRTMAX-13 52) SIGRTMAX-12
53) SIGRTMAX-11 54) SIGRTMAX-10 55) SIGRTMAX-9  56) SIGRTMAX-8  57) SIGRTMAX-7
58) SIGRTMAX-6  59) SIGRTMAX-5  60) SIGRTMAX-4  61) SIGRTMAX-3  62) SIGRTMAX-2
63) SIGRTMAX-1  64) SIGRTMAX


*/

/*
简单介绍: 这是一个信号定义的函数 
1.回调函数必须有参数
2.后面定义的某个信号的行为会覆盖前面定义的信号行为
3.创建了子进程 这些信号的定义显得更有意义
4.目前只有信号定义 没有发生信号的动作  paue 十个阻塞状态 等待信号来临






*/





/*

使用说明:g++编译

//# ./a.out &
[1] 2809
# ps -aux |grep a.out 
root      2809  0.0  0.0   2028   276 pts/9    S    14:34   0:00 ./a.out
root      2811  0.0  0.0   6108   864 pts/9    S+   14:34   0:00 grep --color=auto a.out
# kill -10 2809
# received SIGUSR1   10  
# ps -aux |grep a.out 
//root      2809  0.0  0.0   2032   276 pts/9    S    14:34   0:00 ./a.out
root      2813  0.0  0.0   6108   868 pts/9    S+   14:34   0:00 grep --color=auto a.out

*/
int
main(void)
{
	
	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
		err_sys("can't catch SIGUSR2");
	if (signal(SIGUSR1, sig_usr1) == SIG_ERR)
		err_sys("can't catch SIGUSR1");
	if (signal(SIGUSR2, sig_usr1) == SIG_ERR) //覆盖了  SIGUSR2回调函数
		err_sys("can't catch SIGUSR2");
	if (signal(SIGUSR1, sig_usr) == SIG_ERR) //覆盖了  的SIGUSR1回调函数
	err_sys("can't catch SIGUSR1");
	for ( ; ; )  // 循环等待某个信号 
		pause();  //直到有信号来临  才中断本次循环
}


// 定义两种信号函数的行为  

//  参数是默认被传递进来的  这个参数是信号signo  就是信号的值
static void
sig_usr(int signo)		/* argument is signal number */
{
	if (signo == SIGUSR1)
		printf("received SIGUSR1   %d  \n",signo);
	else if (signo == SIGUSR2)
		printf("received SIGUSR2  %d  \n",signo);
	else
		err_dump("received signal %d\n", signo);
}

static void
sig_usr1(int musthavapara )		/* argument is signal number */
{
	if (musthavapara == SIGUSR1)
		printf("received SIGUSR1   in sig_usr1 \n");
	else if (musthavapara == SIGUSR2)
		printf("received SIGUSR2 in sig_usr1 \n");
	else
		err_dump("received signal in sig_usr1\n");
}

