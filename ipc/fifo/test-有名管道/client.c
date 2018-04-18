#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define FIFO_READ "readfifo"
#define FIFO_WRITE "writefifo"
#define LEN       300

int main(int argc, char* argv[])
{
    char buf[LEN];
    char readbuf[LEN];
    int wfd;
    int rfd;
    int len;

    mkfifo(FIFO_WRITE, S_IFIFO|0666);

    wfd = open(FIFO_WRITE, O_WRONLY);

    rfd = open(FIFO_READ, O_RDONLY);
    
    while(1)
    {
        bzero(buf, LEN);
        printf("client: ");
        fgets(buf, LEN, stdin);
        buf[strlen(buf)-1] = '\0';

        write(wfd, buf, strlen(buf));

        printf("buf:%s\n", buf);
        
        if (strncmp("quit", buf, strlen(buf)) == 0)
        {
            break;
        }

        bzero(readbuf, LEN);
        len = read(rfd, readbuf, LEN);
        if (len > 0)
        {
            
            readbuf[len] = '\0';
            printf("server: %s\n", readbuf);
        }
        
        
    }
    

    close(wfd);
    

    unlink(FIFO_WRITE);

    close(rfd);



    return 0;    
}