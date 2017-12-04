#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>



// 子进程 先调用了exit 退出  父进程没有退出 成为了僵尸 没人给收尸

// 除非主调进程异常结束  会终止全部操作 然而主调程序终止可能不是我们想要的结果  也许他是个日常服务的任务
int main()
{
    pid_t pid;
    pid = fork();
    if (pid < 0) // 容错
    {
        perror("fork error:");
        exit(1);
    }
    else if (pid == 0) // 子进程 正常退出
    {
        printf("I am child process.I am exiting.\n");
        exit(0);
    }
    printf("I am father process.I will sleep two seconds\n");
    //等待子进程先退出
    sleep(2);
    //输出进程信息
    system("ps -o pid,ppid,state,tty,command");
    printf("father process is exiting.\n");
    return 0;
}