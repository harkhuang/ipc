

### 10.7 信号函数定义中应该注意的问题

#### 问题1 

  - 如果调用者已设置了闹钟，则它被sleep1函数中的第一次alarm调用擦去。
  

#### 解决办法
  - 利用返回值记录 剩余时钟
  可用下列方法更正这一点：检查第一次调用 alarm的返回值，如其小于本次调用 alarm的参数值，则只应等到该前次设置的闹钟时间超时。如果前次设置闹钟时间的超时时刻后于本次设置值，则在sleep1函数返回之前，再次设置闹钟时间，使其在预定时间再发生超时。

#### 问题2.该程序中修改了对SIGALRM的配置。  
       在一个繁忙的系统中，可能 a l a r m在调用  p a u s e之前超时，并调用了信号处理程序。如果发生了这种情况，则在调用 p a u s e后，如果没有  捕捉到其他信号，则调用者将永远被挂起

#### 解决办法  
   - 没有对原信号处理程序做保存   修改信号掩码  对未决信号 做特殊处理=
    可用下列方法更正这一点：检查第一次调用 alarm的返回值，如其小于本次调用 alarm的参数值，则只应等到该前次设置的闹钟时间超时。如果前次设置闹钟时间的超时时刻后于本次设置值，则在sleep1函数返回之前，再次设置闹钟时间，使其在预定时间再发生超时。

#### 问题 3 在调用 alarm和pause之间有一个竞态条件。 

    在一个繁忙的系统中，可能 a l a r m在调用  p a u s e之前超时，并调用了信号处理程序。如果发生了这种情况，则在调用 p a u s e后，如果没有  捕捉到其他信号，则调用者将永远被挂起

#### 解决办法  利用setjmp和longjmp函数切换上下文
    S V R 2中的s l e e p实现使用了s e t j m p和l o n g j m p (见7 . 1 0节)以避免问题( 3 )中所说明的竞态条件。
    此函数的一个简化版本，称为 s l e e p 2，示于程序 1 0 - 5中（为了缩短实例长度，程序中没有处理
    上面所说的问题( 1 )和( 2 )



代码1sleep1

```
static void sig_alrm(int signo){
	/* nothing to do, just return to wake up the pause */
}

unsigned int sleep1(unsigned int nsecs){
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		return(nsecs);
	alarm(nsecs);		/* start the timer */
	pause();			/* next caught signal wakes us up */
	return(alarm(0));	/* turn off timer, return unslept time */
}
 
// 实际执行了1秒钟  预期sleep  共6秒钟
int main()
{
	alarm(5);  // 问题1	:多次调用时钟信号  后面的信号会覆盖前面的信号
	sleep1(1);
	return 0;
}
```


代码1sleep2


```
static jmp_buf	env_alrm;

static void
sig_alrm(int signo)
{
	printf("时钟信号来了,我进入了信号回调函数 sig_alrm\n");
	printf("我尝试了longjmp(env_alrm, 1);再次跳转到主调函数 \n");
	longjmp(env_alrm, 1);// 通常第二个参数都默认写成1
	printf("跳出了 sig_alrm\n");//此行代码无效
}




// 避免了pause函数和alarm韩寒苏竞争
unsigned int
sleep2(unsigned int nsecs)
{
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)  
		return(nsecs);
	if (setjmp(env_alrm) == 0) { // 设置一个longjmp的标签  返回值为0表示成功完成了设置
	   // 我设置了一个 
	    printf("我成功的设置了 setjmp(env_alrm) \n");
	    printf("我设置了一个 %d 秒的闹钟信号\n",nsecs);
		alarm(nsecs);		/* start the timer */   // 来个闹钟实时  如果时钟信号来了  那么代码跳到25行重新来过 避免了alarm先调用 后面pause没法结束
	    printf("成功定时向本进程发送一个时钟信号\n");
		printf("在此pause等待时钟信号.......\n\n\n");
		pause();			/* next caught signal wakes us up */
	}
	return(alarm(0));		/* turn off timer, return unslept time */
}


```