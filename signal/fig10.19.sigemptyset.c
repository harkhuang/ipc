#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
typedef	void	Sigfunc(int);

#define err_sys(fmt, args...)  printf(fmt, ##args)


//同18几乎没有区别
Sigfunc *
signal_intr(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
#ifdef	SA_INTERRUPT
	act.sa_flags |= SA_INTERRUPT;
#endif
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}

int test19(){
	return 0;
}