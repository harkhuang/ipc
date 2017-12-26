#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
static void  jishi();
unsigned int	sleep2(unsigned int);
static void		sig_int(int);

static jmp_buf	env_alrm;

// 通过setjmp 返回值不同 和 longjmp来跳转
static void sig_alrm(int signo){
	longjmp(env_alrm, 1);// 通常第二个参数都默认写成1
}
 
unsigned int sleep3(unsigned int nsecs){
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)  
		return(nsecs);
	// 这行代码的意义就是确定了  alarm和pause的先后顺序
	// 1.执行了先完整调用alarm  然后调用pause
	// 2.alarm没有完全调用(即 产生 ->未决->递送)完整的过程 
	if (setjmp(env_alrm) == 0) { // 设置一个longjmp的标签  返回值为0表示成功完成了设置  如果返回值不是这个 那么就执行else分支
		alarm(nsecs);		/* start the timer */   // 来个闹钟实时  如果时钟信号来了  那么代码跳到25行重新来过 避免了alarm先调用 后面pause没法结束
		pause();			/* next caught signal wakes us up */
	}
	return(alarm(0));		/* turn off timer, return unslept time */
}



// 用例告诉我们不能轻易决定实际运行到底是谁先结束
int test9(void){
	unsigned int	unslept;
	if (signal(SIGINT, sig_int) == SIG_ERR)
		printf("signal(SIGINT) error\n");
	unslept = sleep3(1);
	printf("sleep2 returned: %u\n", unslept);
	exit(0);
}

static void
sig_int(int signo)
{
	int				i, j;
	volatile int	k;
	/*
	 * Tune these loops to run for more than 5 seconds
	 * on whatever system this test program is run.
	 */
	long long L1,L2,L3;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	L1 = tv.tv_sec;
	printf("\nsig_int starting\n");
	for (i = 0; i < 300000; i++)
		for (j = 0; j < 4000; j++)
			k += i * j;
	printf("sig_int finished\n");
	gettimeofday(&tv,NULL);
	L2 = tv.tv_sec;
	printf("%lld    /n",L2-L1 );
}


void  jishi(){
	int i=0,j=0,k=0;
	long long L1,L2,L3;
	struct timeval tv;
	
	//取一个时间
	gettimeofday(&tv,NULL);
	//L1 = tv.tv_sec*1000*1000 + tv.tv_usec;
	L1 = tv.tv_sec;
	//执行一些代码,如算法1...
		printf("\nsig_int starting\n");
	for (i = 0; i < 300000; i++)
		for (j = 0; j < 4000; j++)
			k += i * j;
	printf("sig_int finished\n");
	//第二次取时间
	gettimeofday(&tv,NULL);
	//L2 = tv.tv_sec*1000*1000+tv.tv_usec;
	L2 = tv.tv_sec;
	printf("%lld    \n",L2-L1 );
}