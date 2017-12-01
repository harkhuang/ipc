/*************************************************************************
	> File Name: myserver.c
	> Author: 
	> Mail: 
	> Created Time: Mon Nov 27 15:26:25 2017


	loop的服务端程序
 ************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "proto.h"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>



#define	SERVERPORT	"1988"

#define	FMT_STAMP	"sleep  2 second  and return time Stamp: %ld\r\n"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define	BUFSIZE	1024
 
static void server_job(int newsd)
{
	char buf[BUFSIZE] = { 0 };
	ssize_t len;
	int pos, ret;
	
	// 获取时间写入buf
	//第一次写入
//	len = snprintf(buf, BUFSIZE, FMT_STAMP_SERVER2, (long long)time(NULL));
 	len = snprintf(buf, BUFSIZE, FMT_STAMP, (long long)time(NULL));
	
	pos = 0;

	//sleep(2);  //模拟一个处理程序的延迟 模拟backlog的堆积
	
	
	while (len>0) {
		// 发送时间到客户端   
		ret = send(newsd, buf+pos, len, 0);
		if (ret<0) {
			if (errno==EINTR) {
				continue;
			} else {
				perror("send()");
				exit(1);
			}
		}

		// 如果没有发送完成 指针移动
		len -= ret;
		pos += ret;
	}


	// 做了两次写
	len = snprintf(buf, BUFSIZE, FMT_STAMP, (long long)time(NULL));
	
	pos = 0;

	sleep(2);  //模拟一个处理程序的延迟 模拟backlog的堆积
	
	
	while (len>0) {
		// 发送时间到客户端   
		ret = send(newsd, buf+pos, len, 0);
		if (ret<0) {
			if (errno==EINTR) {
				continue;
			} else {
				perror("send()");
				exit(1);
			}
		}

		// 如果没有发送完成 指针移动
		len -= ret;
		pos += ret;
	}
}

#define	IPSTRSIZE	40

int
main()
{
	
	int sd, newsd;
	struct sockaddr_in laddr, raddr;
	socklen_t raddr_len;
	char ipstr[IPSTRSIZE];
	pid_t pid;
	// 绑定了套接字
	sd = socket(AF_INET, SOCK_STREAM, 0 /* IPPROTO_TCP, IPPROTO_SCTP */);
	if (sd<0) {
		perror("socket()");
		exit(1);
	}
	// 初始化套接字
	laddr.sin_family = AF_INET;
	laddr.sin_port = htons(atoi(SERVERPORT));
	// 转换初始变量为本地字节序
	inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);
	if (bind(sd, (sockaddr*)&laddr, sizeof(laddr))<0) {
		perror("bind()");
		exit(1);
	}


	// 设置最大队列为200  
	if (listen(sd, 200)<0) {
		perror("listen()");
		exit(1);
	}

	//监听socket
	raddr_len = sizeof(raddr);
	while (1) {

		// 监听socket  获取了socket完整地址 
		newsd = accept(sd, (sockaddr*)&raddr, &raddr_len);

		// 容错处理
		if (newsd<0) {
			if (errno==EINTR) {
				continue;
			} else {
				perror("accept()");
				exit(1);
			}
		} else {

			//创建了多个进程处理 客户端请求
			pid = fork();

			//子进程做的事情  这里做到的是  不必每个进程都来等待排队  而是通过子进程方式完成了一次她们的通讯
			if (pid==0) {

				// 先关闭套接字
				close(sd);
				// 转换地址网络字节序为本地字节序
				inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);

				//打印从客户端获取的信息
				printf("=== Client: %s:%d ===\n", ipstr, ntohs(raddr.sin_port));

				// 工作组仅仅处理递交的socketfd
				server_job(newsd);
				close(newsd);
				exit(0);
			} else {
				close(newsd);
			}
		}
	}

	close(sd);

	return 0;
}

