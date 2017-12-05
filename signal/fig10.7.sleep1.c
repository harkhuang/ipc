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
// 3.系统繁忙存在竞争  即存在延迟的情况 即alarm被处理后 一直挂在pause()
unsigned int
sleep1(unsigned int nsecs)
{

	// sleep(1);  // 添加这行函数看alarm函数返回值变化   5- 1
	
	//   SIGALRM默认处理方式：终止；当由alarm或setitimer函数设置的定时器超时时，会产生该信号。
	// 这是一条用户态到内核态的转换 所有的apue函数都是涉及到内核的切换
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)  // 既然重新定义了该信号就要对该信号默认行为 所在系统的定义要有所了解
		return(nsecs);
	int ret  =  alarm(nsecs);		/* start the timer */

	alarm(ret+nsecs);    //再次覆盖闹钟函数  
	printf("alarm函数返回值 = %d\n",ret);
	pause();			/* next caught signal wakes us up */
	return(alarm(0));	/* turn off timer, return unslept time */
}



// 预期sleep  共6秒钟
// 实际执行了1秒钟
int main()
{
	// 预期执行五秒  但是sleep1实现代码中  却中断了 alarm 时钟  
	// 多次调用时钟信号  后面的信号会覆盖前面的信号
	alarm(5);  // 这里如果定义了 5秒 实际睡眠只有两秒 
	sleep1(1);
	return 0;
}
