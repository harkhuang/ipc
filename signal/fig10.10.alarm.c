#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define  MAXLINE  20
static void	sig_alrm(int);


/*
alarm  和 read 在内核中处于竞争关系


问题:  程序在执行后没有输入一直处于挂起状态? 为什么
*/
int
test10(void)
{
	int		n;
	char	line[MAXLINE];

	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		printf("signal(SIGALRM) error");

	alarm(10);// 10秒后我的默认动作啥也不做哦
	if ((n = read(STDIN_FILENO, line, MAXLINE)) < 0)//我可能产生的系统延迟1~100秒都有可能哦    
		printf("read error");
	alarm(0);// 读完了重置闹钟哦

	write(STDOUT_FILENO, line, n);
	exit(0);
}

static void
sig_alrm(int signo)
{
	/* nothing to do, just return to interrupt the read */
}
