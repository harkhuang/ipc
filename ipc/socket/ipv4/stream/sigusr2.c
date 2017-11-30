#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void sig_usr(int signo)
{
        if(signo == SIGUSR1)
                printf("received SIGUSR1\n");
        else if(signo == SIGUSR2)
                printf("received SIGUSR2\n");
        else
                printf("received signal %d\n", signo);
}
int main(void)
{
        if(signal(SIGUSR1, sig_usr) == SIG_ERR)
                printf("can't catch SIGUSR1\n");
        if(signal(SIGUSR2, sig_usr) == SIG_ERR)
                printf("can't catch SIGUSR2\n");


        // 注册了信号 后面的行为都在信号默认处理行为之下做相应的处理和调用
        for(; ;)
                pause();
}

/*

hk@ubuntu14:/mnt/ipcframe/ipc/socket/ipv4/stream$ ./a.out &
[1] 4194
hk@ubuntu14:/mnt/ipcframe/ipc/socket/ipv4/stream$ kill -USR1 4194
received SIGUSR1
hk@ubuntu14:/mnt/ipcframe/ipc/socket/ipv4/stream$ kill -USR2 4194
received SIGUSR2
hk@ubuntu14:/mnt/ipcframe/ipc/socket/ipv4/stream$ 



*/