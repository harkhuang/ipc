#include	<sys/wait.h>
#include	<errno.h>
#include	<signal.h>
#include	<unistd.h>


// system 函数的系统实现
int
system(const char *cmdstring)	/* with appropriate signal handling */
{
	pid_t				pid;  //定义一个进程id
	int					status; // 定一个状态值标记
	struct sigaction	ignore, saveintr, savequit;  // 定义信号集
	sigset_t			chldmask, savemask;

	if (cmdstring == NULL) // 查看传递进来的命令
		return(1);		/* always a command processor with UNIX */

	ignore.sa_handler = SIG_IGN;	/* ignore SIGINT and SIGQUIT */   // 定义一个忽略信号
	sigemptyset(&ignore.sa_mask); // 定义忽略信号掩码
	ignore.sa_flags = 0; //定义忽略信号标志信号
	if (sigaction(SIGINT, &ignore, &saveintr) < 0)  //设置进程当前忽略信号机
		return(-1);
	if (sigaction(SIGQUIT, &ignore, &savequit) < 0)
		return(-1);
	sigemptyset(&chldmask);			/* now block SIGCHLD */
	sigaddset(&chldmask, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &chldmask, &savemask) < 0)
		return(-1);

	if ((pid = fork()) < 0) {
		status = -1;	/* probably out of processes */
	} else if (pid == 0) {			/* child */
		/* restore previous signal actions & reset signal mask */
		sigaction(SIGINT, &saveintr, NULL);
		sigaction(SIGQUIT, &savequit, NULL);
		sigprocmask(SIG_SETMASK, &savemask, NULL);

		execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
		_exit(127);		/* exec error */
	} else {						/* parent */
		while (waitpid(pid, &status, 0) < 0)
			if (errno != EINTR) {
				status = -1; /* error other than EINTR from waitpid() */
				break;
			}
	}

	/* restore previous signal actions & reset signal mask */
	if (sigaction(SIGINT, &saveintr, NULL) < 0)
		return(-1);
	if (sigaction(SIGQUIT, &savequit, NULL) < 0)
		return(-1);
	if (sigprocmask(SIG_SETMASK, &savemask, NULL) < 0)
		return(-1);

	return(status);
}
