#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#define BUFSIZE 1024
#define PATH "/etc/service"
#define PATH_TMP "./tmp"
int main()
{
	int size = 0;
	char buf[1024] = {"0"};
	int fd = open(PATH, O_RDONLY | O_NONBLOCK);
	/* if error */

	while (1) {
	    size = read(fd, buf, BUFSIZE);
	    if (size < 0) {
	        if (EAGAIN == errno) {
	            continue;
	        }
	        perror("read()");
	        exit(1);
	    }

	    // do sth...

	}

}