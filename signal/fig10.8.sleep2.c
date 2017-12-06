#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
static jmp_buf	env_alrm;



// 通过setjmp 返回值不同 和 longjmp来跳转  
// 解决了sleep先结束 引起的竞争错误
static void sig_alrm(int signo){
	longjmp(env_alrm, 1);// 通常第二个参数都默认写成1
}
 
unsigned int
sleep2(unsigned int nsecs)
{
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)  
		return(nsecs);

	// 这行代码的意义就是确定了  alarm和pause的先后顺序
	// 1.执行了先完整调用alarm  然后调用pause
	// 2.alarm没有完全调用(即 产生 ->未决->递送)完整的过程 
	if (setjmp(env_alrm) == 0) { // 设置一个longjmp的标签  返回值为0表示成功完成了设置  如果返回值不是这个 那么就执行else分支
		alarm(nsecs);		/* start the timer */   // 来个闹钟实时  如果时钟信号来了  那么代码跳到25行重新来过 避免了alarm先调用 后面pause没法结束
		sleep(5);          // 这里不怕竞争条件了  跳转后 完成了对竞争条件的处理
		pause();			/* next caught signal wakes us up */
	}
	return(alarm(0));		/* turn off timer, return unslept time */
}





// 利用函数跳转的方式  来屏蔽上下文中信号属性???
// 防止内核态(繁忙)到用户态状态切换的错误



//不懂为什么避免了竞争?
int main()
{
	// 预期执行五秒  但是sleep1实现代码中  却中断了 alarm 时钟  
	// 多次调用时钟信号  后面的信号会覆盖前面的信号
	//alarm(5);  // 这里如果定义了 5秒 实际睡眠只有两秒 
	int ret = sleep2(1);
	printf("ret= %d\n",ret);
	return 0;
}
