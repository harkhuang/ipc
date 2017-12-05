#include	<setjmp.h>
#include	<signal.h>
#include	<unistd.h>
#include <stdio.h>
static jmp_buf	env_alrm;

static void
sig_alrm(int signo)
{
	printf("我进入了信号回调函数 sig_alrm\n");

	printf("我尝试了再次跳转到主调函数 \n");
	longjmp(env_alrm, 1);// 通常第二个参数都默认写成1
	printf("跳出了 sig_alrm\n");
}


//模拟sleep2
unsigned int
sleep2(unsigned int nsecs)
{
	if (signal(SIGALRM, sig_alrm) == SIG_ERR) // 定义用户级别信号动作
		return(nsecs);
	if (setjmp(env_alrm) == 0) { // 设置一个longjmp的标签  返回值为0认为是成功完成了设置
	   // 我设置了一个 
	    printf("我设置了一个 %d 秒的闹钟信号\n",nsecs);
		alarm(nsecs);		/* start the timer */   // 来个闹钟实时 
		pause();			/* next caught signal wakes us up */
	}
	return(alarm(0));		/* turn off timer, return unslept time */
}





// 利用函数跳转的方式  来屏蔽上下文中信号属性???
// 防止内核态(繁忙)到用户态状态切换的错误

int main()
{
	// 预期执行五秒  但是sleep1实现代码中  却中断了 alarm 时钟  
	// 多次调用时钟信号  后面的信号会覆盖前面的信号
	//alarm(5);  // 这里如果定义了 5秒 实际睡眠只有两秒 
	sleep2(2);
	return 0;
}
