 /**********************************************************************
 # File Name:   1epoll.c
 # Version:     1.0
 # Mail:        shiyanhk@gmail.com 
 # Created Time: 2016-02-02	
 ************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

struct epoll_event event1 = {
    .events = EPOLLIN,
    .data = { .fd = 0}
};

struct str_my {
	int a;
	int b;
	int c;

};


struct str_my  getnew={
	.a=10,
	.b= {.c=0}
};

int main(int argc, char **argv)
{
    int epfd = epoll_create(1);
    int rc;
    epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event1);
    sleep(2); //allow meself time to type false\n
    printf("closing stdin ...\n");
    close(0);
    //even if i remove it explicitly the event will be found in the que after
    epoll_ctl(epfd, EPOLL_CTL_DEL, 0, &event1);
    printf("gathering events ...\n");
    event1.events = 0;
    event1.data.fd = -1;
    rc = epoll_wait(epfd, &event1, 1, 0);
    switch(rc) {
        case 1:
            printf("event received: event=%d on fd %d\n", event1.events, event1.data.fd);
            break;
        case 0:
            printf("no events received");
            break;
        case -1:
            printf("epoll_wait error\n");
            break;
        default:
            printf("weird event count %d\n", rc);
    }

    return 0;
}
