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
	int n , fd;
	pid_t pid;
	char line[100] = { 0 };
	fd = open("fifo","r");
	while(1){
		sleep(1);
		memset(line,0,100);
		read(fd,line,100);
		printf("%s\n",line);
	}
	unlink(fd);
	fclose(fd);
    return 0;
}

