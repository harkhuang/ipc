 /**********************************************************************
 # File Name:   fifo.c
 # Version:     1.0
 # Mail:        shiyanhk@gmail.com 
 # Created Time: 2018-04-18	
 ************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
int main(int argc,char **argv)
{
	int n , fd;
	pid_t pid;
	mode_t mode = 0777;
	char buff[5] = {0};
	if (0 > mkfifo("fifo", 777))
		printf("error mkfifo\n");
	fd = open("fifo","w");
	int i = 0;
	while(1){
		sleep(1);
		i++;
		memset(buff,0,5);  //此行代码不加  读端获取数据总是拿不到的?
		sprintf(buff,"buff  is %d\n",i);
		write(fd,buff, 10);
		printf("write %s\n ",buff);
	}

	unlink(fd);
	close(fd);
    return 0;
}


