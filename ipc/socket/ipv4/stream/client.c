#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "proto.h"
#include <unistd.h>

#define MAXLINE 1024

int
main(int argc, char **argv)
{
	int 				sd,n;
	struct sockaddr_in 	raddr;
	long long 			stamp;
	FILE 				*fp;
	char 				recvline[MAXLINE + 1];
	
	
	if (argc<2) {
		// 参数 不能小鱼1
		fprintf(stderr, "Usage...\n");
		exit(1);
	}


	// 创建一个IPV4 的TCP描述符
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd<0) {
		perror("socket()");
		exit(1);
	}

	// 初始化描述符属性
	raddr.sin_family = AF_INET;
	raddr.sin_port = htons(atoi(SERVERPORT));

	//转换套接字字节序
	inet_pton(AF_INET, argv[1], &raddr.sin_addr);
	
	//创建了一堆可用链接   这是一个异步的io处理   在客户端并没有对socket  io 做任何处理 但是  
	//在


	if (connect(sd, ( sockaddr *)&raddr, sizeof(raddr))<0) {
		perror("connect()");
		exit(1);
	}

	printf("  第一次读取 \n" );
	while ( (n = read(sd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF)
			printf("fputs error");
	}
	if (n < 0)
		printf("read error");


    //流的读取 只能单次读取  第二次去读取就没有了 
	//  这里有两种读取的方式  一种是通过read  io方式直接读取  第二种是通过打开文件指针的方式去读取
  
	printf("  第二次读取 \n" );
	while ( (n = read(sd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF)
			printf("fputs error");
	}
	if (n < 0)
		printf("read error");

	//  打开文件描述符
	fp = fdopen(sd, "r+");


#if 0
	if (fp==NULL) {
		perror("fdopen()");
		exit(1);
	}
#endif	

	int ret = 0;
	char bux[1024] = {0};
 
	// 从套接字中读取相关内容
	fscanf(fp, FMT_STAMP, &stamp);
	//fscanf(fp, "%s", &stamp);  这条语句是我对socket理解  报错error
	
	// 关闭套接字
	fclose(fp);

	printf("socket is  [%s]\n", stamp);

	return 0;
}

