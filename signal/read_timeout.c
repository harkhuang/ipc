#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>

#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#include  <stdio.h>
#include  <signal.h>


void  ALARMhandler(int sig)
{
  signal(SIGALRM, SIG_IGN);          /* ignore this signal       */
  printf("Hello");
  signal(SIGALRM, ALARMhandler);     /* reinstall the handler    */
}



void test_read_time_out( )
{

  signal(SIGALRM, ALARMhandler);   
  alarm(2);                     /* set alarm clock          */
  while (1)
    ;
  printf("All done");
}


void  maketimeout1(){
    // struct termios termios;
    // tcgetattr(filedesc, &termios);
    // termios.c_lflag &= ~ICANON; /* Set non-canonical mode */
    // termios.c_cc[VTIME] = 100; /* Set timeout of 10.0 seconds */
    // tcsetattr(filedesc, TCSANOW, &termios);
}


void maketimeout2(){
    fd_set set;
    struct timeval timeout;
    int rv;
    char buff[100];
    int len = 100;
    int filedesc = open( "dev/ttyS0", O_RDWR );

    FD_ZERO(&set); /* clear the set */
    FD_SET(filedesc, &set); /* add our file descriptor to the set */

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    rv = select(filedesc + 1, &set, NULL, NULL, &timeout);
    if(rv == -1)
        perror("select"); /* an error accured */
    else if(rv == 0)
        printf("timeout"); /* a timeout occured */
    else
        read( filedesc, buff, len ); /* there was data to read */
}


int test_readtimeout(void)
{
    test_read_time_out();
    maketimeout1();
}