#include	<signal.h>
#include	<errno.h>

/* <signal.h> usually defines NSIG to include signal number 0 */
#define	SIGBAD(signo)	((signo) <= 0 || (signo) >= NSIG)



// 用sigset_t 类型保存信号集
int
sigaddset(sigset_t *set, int signo)
{
	if (SIGBAD(signo)) { errno = EINVAL; return(-1); }

	*set |= 1 << (signo - 1);		/* turn bit on */  //把1左移signo-1位(先找位->位或)
	return(0);
}

int
sigdelset(sigset_t *set, int signo)
{
	if (SIGBAD(signo)) { errno = EINVAL; return(-1); }

	*set &= ~(1 << (signo - 1));	/* turn bit off */  //把1左移signo-1位 二者都有1 即可(先找位->取反->位与)
	return(0);
}

int
sigismember(const sigset_t *set, int signo)
{
	if (SIGBAD(signo)) { errno = EINVAL; return(-1); }

	return((*set & (1 << (signo - 1))) != 0);
}