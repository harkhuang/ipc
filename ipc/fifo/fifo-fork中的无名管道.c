 /**********************************************************************
 # File Name:   fifo.c
 # Version:     1.0
 # Mail:        shiyanhk@gmail.com 
 # Created Time: 2018-04-18	
 ************************************************************************/
#include <stdio.h>
#include <unistd.h>
int main(int argc,char **argv)
{
	int n , fd[2];
	pid_t pid;
	char line[100];
	fd[1] = open("fifo.info");
	
	if(pipe(fd) < 0)
		printf("pipe error");
	if( (pid = fork())  < 0)
		printf("fork error");
	else if(pid > 0 ){   //parent
		//close(fd[0]);
		sleep(1);
		write(fd[1],"hello,world\n",12);
	} 
	else{				//child
		//close(fd[1]);
		n = read(fd[0],line,50);
		write(1,line,n);
	}
     return 0;
}

