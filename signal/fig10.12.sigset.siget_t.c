#include	<signal.h>
#include	<errno.h>

/* <signal.h> usually defines NSIG to include signal number 0 */
#define	SIGBAD(signo)	((signo) <= 0 || (signo) >= NSIG)

// 用sigset_t 类型保存信号集
int
sigaddset1(unsigned long *set, int signo)
{
	if (SIGBAD(signo)) { errno = EINVAL; return(-1); }
//把1左移signo-1位(先找位->位或)
	*set |= (1 << (signo - 1));		/* turn bit on */  
	
	return(0);
}

int
sigdelset1(unsigned long *set, int signo)
{
	if (SIGBAD(signo)) { errno = EINVAL; return(-1); }
 //把1左移signo-1位 二者都有1 即可(先找位->取反->位与)
	*set &= ~(1 << (signo - 1));	/* turn bit off */ 
	return(0);
}

int
sigismember1(const unsigned long *set, int signo)
{
	if (SIGBAD(signo)) { errno = EINVAL; return(-1); }

	return((*set & (1 << (signo - 1))) != 0);
}

int test12(){

	int data = 100;
	//turn on isUsed
	data |= 1;
	//turn off isUsed
	data &= ~1;
	//turn on notLast
	data &= ~2;
	//turn off notLast
	data |= 2;
	return 0;
}