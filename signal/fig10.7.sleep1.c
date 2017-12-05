#include <signal.h>
#include <unistd.h>
#include <stdio.h>
static void
sig_alrm(int signo)
{
	/* nothing to do, just return to wake up the pause */
}

 
///*********************存在问题
// Q1.如果调用者已设置了闹钟，则它被sleep1函数中的第一次alarm调用擦去。
// ******解决办法 可用下列方法更正这一点：检查第一次调用 alarm的返回值，如其小于本次调用 alarm的参
// ******数值，则只应等到该前次设置的闹钟时间超时。如果前次设置闹钟时间的超时时刻后于本次设
// ******置值，则在sleep1函数返回之前，再次设置闹钟时间，使其在预定时间再发生超时。
// Q2.该程序中修改了对SIGALRM的配置。 
// 更改这一点的方法是：保存s i g n a l函数的返回值(回调函数返回值)，在返回前恢复设置原配置。 
//
// 
// Q3. 在调用 alarm和pause之间有一个竞态条件。 


//解决办法 (基于通信)
// 1.利用返回值记录 剩余时钟
// 2.没有对原信号处理程序做保存   修改信号掩码  对未决信号 做特殊处理
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
