#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "proto.h"

#define MINSPARESERVERS 5
#define MAXSPARESERVERS 10
#define MAXCLIENTS 20

#define IPSTRSIZE	40
#define LINESIZE	80

#define SIG_NOTIFY	SIGUSR2

enum {
	STATE_IDLE=0,
	STATE_BUSY
};



// 服务器状态器  
struct server_st {
	pid_t pid;
	int state;
	int reuse;
};

typedef struct server_st msg_t;//自定义服务器消息结构体

static struct server_st *serverpool;//服务器所有子线程共享的服务器状态结构体
static int idle_count=0, busy_count=0;
static int sd;


// 静态的回调函数  不懂为什么要把job定义成static   防止被外部函数调用吗
static void server_job(int sd, int pos);


// 这个函数为什么这么命名? 目的是什么  是不是因为每个资源都会用到
void
usr2_handler(int s)
{
	return;
}



int
add_1_server(void)
{
	int slot;// 卡槽
	pid_t pid;

	// 增加工作任务 逻辑  空闲+使用  > maxclient  容错
	if (idle_count+busy_count >= MAXCLIENTS) {
		return -1;
	}

	// 作用?
	// 初始化pid值  当pid = -1时  表示可以添加工作任务 并且记录该工作节点位置 fd数组中第一个可以使用的
	for (slot=0;slot<MAXCLIENTS;++slot) {
		if (serverpool[slot].pid==-1) {
			break;
		}
	}

	//  找到上  标记该进程空闲?  不是应该标记使用吗???
	serverpool[slot].state = STATE_IDLE;
	pid = fork();  //fork返回值是子进程id
	if (pid<0) {
		return -1;
	}
	if (pid==0) {
		// 在子进程中添加工作序列
		server_job(sd, slot);
		exit(0);
	} else {
		// 对共享内存中的结构体 标志为子进程id
		serverpool[slot].pid = pid;

		// 为什么对空闲的进程做递增操作?
		idle_count++;
	}

	return 0;
}

int
del_1_server(void)
{
	int i;

	// 当idle为0认为没有可以删除的了
	if (idle_count == 0) {
		return -1;
	}

	// 删除一个sever
	for (i=0;i<MAXCLIENTS;++i) {

		// 条件是有着pid而且状态是idle  idle表示在使用状态
		if (serverpool[i].pid !=-1 && serverpool[i].state == STATE_IDLE) {


			// 这里所谓的竞争是不是没有考虑到busy的情况
			// 这里表示谁和谁的竞争呢?
			kill(serverpool[i].pid, SIGTERM); /* FIXME: 可能有竞争BUG */
			serverpool[i].pid = -1;
			idle_count--;
			break;
		}
	}
	return 0;
}

int
scan_pool(void)
{
	int i, idle=0, busy=0;

	for (i=0;i<MAXCLIENTS;++i) {
		if (serverpool[i].pid == -1) {
			continue;
		}

		
		if (kill(serverpool[i].pid, 0)) {
			serverpool[i].pid = -1;
			continue;
		}
		if (serverpool[i].state == STATE_IDLE) {
			idle++;
		} else if (serverpool[i].state == STATE_BUSY) {
			busy++;
		} else {
			fprintf(stderr, "Unknown state.\n");
			abort();
		}
	}
	idle_count=idle; busy_count=busy;
	return 0;
}

static void server_job(int sd, int pos)
{
	int client_sd;
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_len;
	char ipstr[IPSTRSIZE];
	time_t stamp;
	char linebuf[LINESIZE];
	int len;
	pid_t ppid;

	ppid = getppid();

	peer_addr_len = sizeof(peer_addr);
	while (1) {
		serverpool[pos].state = STATE_IDLE;
		kill(ppid, SIG_NOTIFY);

		client_sd = accept(sd, (sockaddr *)&peer_addr, &peer_addr_len);
		if (client_sd<0) {
			if (errno==EINTR || errno==EAGAIN) {
				continue;
			}
			perror("accept()");
			exit(1);
		}

		serverpool[pos].state = STATE_BUSY;
		kill(ppid, SIG_NOTIFY);

		inet_ntop(AF_INET, &peer_addr.sin_addr, ipstr, IPSTRSIZE);
//		printf("[%d]: Client: %s:%d\n", getpid(), ipstr, ntohs(peer_addr.sin_port));
		stamp = time(NULL);

		len = snprintf(linebuf, LINESIZE, FMT_STAMP, stamp);
		/* FIXME: If LINESIZE is not big enough */

		send(client_sd, linebuf, len, 0);

		sleep(5);

		close(client_sd);
	}
	close(sd);
}

int
main()
{
	int ret, i;
	struct sockaddr_in myend;
	struct sigaction sa, osa;//sa是我们需要定义和捕捉在当前进程中生效的信号捕捉掩码
	sigset_t set, oset;
	int val;


	// 定义信号的回调函数 和信号默认掩码
	sa.sa_handler = SIG_IGN; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDWAIT;
	sigaction(SIGCHLD, &sa, &osa);

	sa.sa_handler = usr2_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIG_NOTIFY, &sa, &osa);

	sigemptyset(&set);
	sigaddset(&set, SIG_NOTIFY);

	// 吧新的信号集合替换旧的集合
	sigprocmask(SIG_BLOCK, &set, &oset);


	//  非常重要  这里申请的是一个共享内存  目的是为了 所有服务器线程共享数据
	// 1、将一个普通文件映射到内存中，通常在需要对文件进行频繁读写时使用，这样用内存读写取代I/O读写，以获得较高的性能；
	// 2、将特殊文件进行匿名内存映射，可以为关联进程提供共享内存空间；
	// 3、为无关联的进程提供共享内存空间，一般也是将一个普通文件映射到内存中。
	// -1是初始化的fd值  0表示偏移 
	// 读写权限的fd    参数flags：影响映射区域的各种特性。 
	// MAP_SHARE   对映射区域的写入数据会复制回文件内，而且允许其他映射该文件的进程共享。
	// MAP_ANONYMOUS  建立匿名映射。此时会忽略参数fd，不涉及文件，而且映射区域无法和其他进程共享。
	serverpool = (struct server_st*)mmap(NULL, (sizeof(struct server_st)*MAXCLIENTS), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	if (serverpool==MAP_FAILED) {  // 申请失败的情况处理 容错
		perror("mmap()");
		exit(1);
	}

	// 
	for (i=0;i<MAXCLIENTS;++i) {
		serverpool[i].pid=-1;
	}

	sd=socket(AF_INET, SOCK_STREAM, 0);
	if (sd==-1) {
		perror("socket()");
		exit(1);
	}

	val = 1;
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))<0) {
		perror("setsockopt(..., SO_REUSEADDR, ...)");
	}

	myend.sin_family = AF_INET;
	myend.sin_port = htons(atoi(SERVERPORT));
	inet_pton(AF_INET, "0.0.0.0", &myend.sin_addr);
	ret=bind(sd, (struct sockaddr*)&myend, sizeof(myend));
	if (ret==-1) {
		perror("bind()");
		exit(1);
	}

	ret=listen(sd, 100);
	if (ret==-1) {
		perror("listen()");
		exit(1);
	}

	for (i=0;i<MINSPARESERVERS;++i) {
		add_1_server();
	}

	while (1) {
		sigsuspend(&oset);
		scan_pool();
		if (idle_count > MAXSPARESERVERS) {
//fprintf(stderr, "[%d]: kill %d server(s).\n", getpid(), (idle_count-MAXSPARESERVERS));
			for (i=0;i<(idle_count-MAXSPARESERVERS);++i) {
				del_1_server();
			}
		} else if (idle_count < MINSPARESERVERS) {
			for (i=0;i<MINSPARESERVERS-idle_count; ++i) {
				add_1_server();
			}
		}
		for (i=0;i<MAXCLIENTS;++i) {
			if (serverpool[i].pid==-1) {
				putchar(' ');
			} else {
				if (serverpool[i].state == STATE_IDLE) {
					putchar('.');
				} else {
					putchar('x');
				}
			}
		}
		putchar('\n');
	}

	sigprocmask(SIG_SETMASK, &oset, NULL);

	exit(0);
}

