#include <signal.h>
#include <unistd.h>
#include <stdio.h>
static void
sig_alrm(int signo)
{
	/* nothing to do, just return to wake up the pause */
}




//  模拟的第一个sleep函数

// 缺陷在哪里?
// 1.sleep1 在使用前调用了alarm(5)这样会被函数里面第二次调用alarm覆盖
// 2.alarm默认行为的修改
// 3.系统繁忙存在竞争   pause 函数 和 alarm函数存在竞争  即存在延迟的情况 即alarm被处理后 一直挂在pause()


//解决办法 (基于通信)
// 1.利用返回值记录 剩余时钟
// 2.修改信号掩码  对未决信号 做特殊处理
// 3.利用setjmp和longjmp函数切换上下文

unsigned int
sleep1(unsigned int nsecs)
{

	// sleep(1);  // 添加这行函数看alarm函数返回值变化   5- 1
	//   SIGALRM默认处理方式：终止；当由alarm或setitimer函数设置的定时器超时时，会产生该信号。
	// 这是一条用户态到内核态的转换 所有的apue函数都是涉及到内核的切换
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)  // 既然重新定义了该信号就要对该信号默认行为 所在系统的定义要有所了解
		return(nsecs);
	alarm(nsecs);		/* start the timer */
	pause();			/* next caught signal wakes us up */
	return(alarm(0));	/* turn off timer, return unslept time */
}



// 预期sleep  共6秒钟
// 实际执行了1秒钟
int main()
{
	alarm(5);  // 问题1	:多次调用时钟信号  后面的信号会覆盖前面的信号
	sleep1(1);
	return 0;
}
