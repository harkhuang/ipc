#include <signal.h>
#include <unistd.h>
#include <stdio.h>
static void
sig_alrm(int signo) // 信号不再默认中断程序
{
	/* nothing to do, just return to wake up the pause */
}
 

unsigned int
sleep1(unsigned int nsecs)
{
	// sleep(1);  // 添加这行函数看alarm函数返回值变化   5- 1
	//   SIGALRM默认处理方式：终止；当由alarm或setitimer函数设置的定时器超时时，会产生该信号。
	// 这是一条用户态到内核态的转换 所有的apue函数都是涉及到内核的切换
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)  // 既然重新定义了该信号就要对该信号默认行为 所在系统的定义要有所了解
		return(nsecs);
	
	/*
	下面两行都是对信号的重新定义,存在竞争
	竞争的理解:1.内核产生alarm有个未决期 系统繁忙我们认为42行会切到内核形态  产生信号->(未决期)->递送
	在未决期 spending 状态 如果43行(产生信号->(未决期)->递送)这个时间 非常短,系统还没有执行pause,那么 pause将永远捕获不到信号
	*/
	// 这行代码会切换到内核态  经历了(产生信号->(未决期)->递送) 我们期望的是在递送前 pause已经准备好了 
	alarm(nsecs);		/* start the timer */  // 内核调用  
	sleep(2);        // 情况3模拟
	pause();			/* next caught signal wakes us up */  // 内核调用
	return(alarm(0));	/* turn off timer, return unslept time */
}



void func(void ){
	printf("this is a alarm(1) function");
	alarm(1);
	pause();
}

int test7()
{
	//alarm(5);  // 问题1	:多次调用时钟信号  后面的信号会覆盖前面的信号
	sleep1(1);

	return 0;
}
///*********************存在问题
// Q1.如果调用者已设置了闹钟，则它被sleep1函数中的第一次alarm调用擦去。
// ******解决办法 可用下列方法更正这一点：检查第一次调用 alarm的返回值，如其小于本次调用 alarm的参
// ******数值，则只应等到该前次设置的闹钟时间超时。如果前次设置闹钟时间的超时时刻后于本次设
// ******置值，则在sleep1函数返回之前，再次设置闹钟时间，使其在预定时间再发生超时。
// Q2.该程序中修改了对SIGALRM的配置。 
// 更改这一点的方法是：保存s i g n a l函数的返回值(回调函数返回值)，在返回前恢复设置原配置。 
// Q3. 在调用 alarm和pause之间有一个竞态条件。 

//解决办法 (基于通信)
// 1.利用返回值记录 剩余时钟
// 2.没有对原信号处理程序做保存   修改信号掩码  对未决信号 做特殊处理
// 3.利用setjmp和longjmp函数切换上下文
