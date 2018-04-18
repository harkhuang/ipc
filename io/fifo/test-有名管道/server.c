#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define FIFO_WRITE "readfifo"
#define FIFO_READ "writefifo"
#define LEN       300

int main(int argc, char* argv[])
{
    char buf[LEN];
    char readbuf[LEN];
    int wfd;
    int rfd;
    int len;

    mkfifo(FIFO_WRITE, S_IFIFO|0666);

    rfd = open(FIFO_READ, O_RDONLY);
    wfd = open(FIFO_WRITE, O_WRONLY);

    
    
    while(1)
    {
        bzero(readbuf, LEN);
        len = read(rfd, readbuf, LEN);
        printf("len=%d\n", len);
        if (len > 0)
        {
            
            readbuf[len] = '\0';
            printf("client: %s\n", readbuf);

            if (strncmp("quit", readbuf, strlen(readbuf)) == 0)
            {
                break;
            }

            
        }

        bzero(buf, LEN);
        printf("server: ");
        fgets(buf, LEN, stdin);
        buf[strlen(buf)-1] = '\0';

        write(wfd, buf, strlen(buf));
        
    


        
        
    }
    
    

    close(wfd);
    

    unlink(FIFO_WRITE);

    close(rfd);



    return 0;    
}