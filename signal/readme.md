
APUE读书笔记
============================


第十章
--------------------------------------------------
 
ToDoList

- [x] sleep初级实现  2017年12月6日 10:41:30
- [ ] sleep进阶实现
- [ ] sleep高阶实现
- [ ] `最佳实践`  内核sleep实现 
- [ ] 

| 项目         |  时间  | 文档版本|作者|备注|
| --------    | :----:  |:------:|:----|:---|
|  读书笔记 |      2017/12/06    |初稿|shiyanhk@gmail.com|sleep初级实现|



[TOC]

- - -
### 1 编写目的

- 之前对singnal信号理解一知半解,就这篇文章想通过我的描述来把signal这一章节,深入浅出的讲解出来.
- 不妨我们就从apue书中具体用例展开我们需要的描述,虽然片面,后续仍会有修改和添加.

### 内容计划


### 2 循序渐进的实现
> sleep函数基于信号实现,循序渐进的几个用例,这里会层层深入,发现和修正我们认知路上的错误,而且附上了代码.
 
#### 2.1 `sleep` ==初级实现==

##### 2.1.1 `code`
```
#include	<signal.h>
#include	<unistd.h>
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
```

##### 2.1.2 代码问题分析
 - 1.如果调用者已设置了闹钟，则它被`sleep1`函数中的第一次`alarm`调用擦去。
 - 2.该程序中修改了对`SIGALRM`的配置。 
 - 3.在调用 `alarm`和`pause`之间有一个竞态条件。 

#####  2.1.3 解决方案
 - 1.利用alarm返回值确定上次信号状态 
 - 2.利用signal返回值确定signal函数原始回调操作 
 - 3.利用setjmp和longjmp修改 


##### 2.1.4 `详细分析`

- 问题1 **利用alarm返回值记录 剩余时钟**
  - 办法 
  >  可用下列方法更正这一点：检查第一次调用alarm的返回值，如其小于本次调用 alarm的参数值，则只应等到该前次设置的闹钟时间超时。如果前次设置闹钟时间的超时时刻后于本次设置值，则在sleep1函数返回之前，再次设置闹钟时间，使其在预定时间再发生超时。

  - 代码 
```
//todo
```

- 问题2 **该程序中修改了对SIGALRM的配置。**
> 在一个繁忙的系统中，可能 a l a r m在调用  p a u s e之前超时，并调用了信号处理程序。如果发生了这种情况，则在调用 p a u s e后，如果没有  捕捉到其他信号，则调用者将永远被挂起.

  - 办法 
> 没有对原信号处理程序做保存   修改信号掩码  对未决信号 做特殊处理 
可用下列方法更正这一点：检查第一次调用 alarm的返回值，如其小于本次调用 alarm的参数值，则只应等到该前次设置的闹钟时间超时。如果前次设置闹钟时间的超时时刻后于本次设置值，则在sleep1函数返回之前，再次设置闹钟时间，使其在预定时间再发生超时。

  - 代码 
** 代码实现**
```
//todo
```

- 问题3 **在调用 alarm和pause之间有一个竞态条件。 **
 > 在一个繁忙的系统中，可能 a l a r m在调用  p a u s e之前超时，并调用了信号处理程序。如果发生了这种情况，则在调用 p a u s e后，如果没有  捕捉到其他信号，则调用者将永远被挂起

  - 办法
** 利用setjmp和longjmp函数切换上下文**
> SVR2中的sleep实现使用了setjmp和longjmp (见7 . 1 0节)以避免问题( 3 )中所说明的竞态条件。此函数的一个简化版本，称为 s l e e p 2，示于程序 1 0 - 5中（为了缩短实例长度，程序中没有处理上面所说的问题( 1 )和( 2 )



**问题 3代码实现**

```
代码略(这部分比较复杂,我们放到下一章节单独来讲)
```



#### 2.2 sleep进阶实现

##### 2.2.1代码


```
#include	<setjmp.h>
#include	<signal.h>
#include	<unistd.h>
static jmp_buf	env_alrm;
static void sig_alrm(int signo){
	longjmp(env_alrm, 1);
}

unsigned int sleep2(unsigned int nsecs){
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		return(nsecs);
	if (setjmp(env_alrm) == 0) {
		alarm(nsecs);		/* start the timer */
		pause();			/* next caught signal wakes us up */
	}
	return(alarm(0));		/* turn off timer, return unslept time */
}

```

##### 2.2.2问题
> 1.**xxxxxxxx**
> 2.**xxxxxxxx**

#####  2.2.3解决办法
> 1.**xxxxxxxx**
> 2.**xxxxxxxx**


##### 2.2.4详细分析

>  **问题 1**  ++利用alarm返回值记录 剩余时钟 ++
   
  > 可用下列方法更正这一点：检查第一次调用alarm的返回值，如其小于本次调用 alarm的参数值，则只应等到该前次设置的闹钟时间超时。如果前次设置闹钟时间的超时时刻后于本次设置值，则在sleep1函数返回之前，再次设置闹钟时间，使其在预定时间再发生超时。



 
