#include <stdio.h>  
#include <netinet/in.h>//for struct sockaddr_in  
#include <string.h>//for memset  
#include <signal.h>//for signal  
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

typedef void (*sighandler_t)(int);  
  
static void  
read_alarm(int signo)  
{  
    return;  
}  
  
int test_alarm()  
{  
    int conn_sock;  
    struct sockaddr_in ser_addr;  
    int ret;  
    char buf[1024];  
    sighandler_t src_sig;  
    struct sigaction sa_alarm;  
  
    conn_sock = socket(AF_INET, SOCK_STREAM, 0);  
    if (conn_sock < 0)  
    {  
        perror("socket error");  
        return -1;  
    }  
  
    memset(&ser_addr, 0, sizeof(ser_addr));  
    ser_addr.sin_family = AF_INET;  
    ser_addr.sin_port = htons(8080);  
    inet_pton(AF_INET, "127.0.0.1", &ser_addr.sin_addr);  
  
    ret = connect(conn_sock, (struct sockaddr *)&ser_addr, sizeof(ser_addr));  
    if (ret < 0)  
    {  
        perror("connect error");  
        return -1;  
    }  
  
    //src_sig = signal(SIGALRM, read_alarm);  
    sa_alarm.sa_flags = SA_RESETHAND;  
    sa_alarm.sa_handler = read_alarm;  
    sigaction(SIGALRM, &sa_alarm, NULL);  
    alarm(10);  
    ret = read(conn_sock, buf, sizeof(buf));  
    if (ret < 0)  
    {  
        perror("read error");  
    }  
    else if (ret == 0)  
    {  
        printf("close by peer\n");  
    }  
    else  
    {  
        printf("recv %d bytes\n", ret);  
    }  
    alarm(0);  
    //signal(SIGALRM, src_sig);  
    return 0;  
}  