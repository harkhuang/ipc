 
#include <errno.h>
#include	<signal.h>
#include <stdio.h>
void
pr_mask(const char *str)
{
	sigset_t	sigset;//  保存信号掩码的结构体
	int			errno_save; // 

	errno_save = errno;		/* we can be called by signal handlers */
	if (sigprocmask(0, NULL, &sigset) < 0)  // 掩码的容错处理
		printf("sigprocmask error");

	printf("%s", str);
	if (sigismember(&sigset, SIGINT))   printf("SIGINT ");
	if (sigismember(&sigset, SIGQUIT))  printf("SIGQUIT ");
	if (sigismember(&sigset, SIGUSR1))  printf("SIGUSR1 ");
	if (sigismember(&sigset, SIGALRM))  printf("SIGALRM ");

	/* remaining signals can go here  */

	printf("\n");
	errno = errno_save;
}



int main(){
	return 0;
}