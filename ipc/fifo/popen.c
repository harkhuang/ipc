 /**********************************************************************
 # File Name:   popen.c
 # Version:     1.0
 # Mail:        shiyanhk@gmail.com 
 # Created Time: 2018-04-18	
 
  
这个函数主要是配合exec来用的 
无名管道的一种方式 
相当于exec创建进程 
二者共享了 文件描述符  
这个文件描述符就是管道
  
  ************************************************************************/
#include <stdio.h>
#include <unistd.h>

#define PAGER "${PAGER:-more}"

int main(int argc,char **argv)
{
    char line[20];
    FILE *fpin,*fpout;
    
	fpin = fopen("popen.fifo", "r");

	fpout = open(PAGER, "w");

	sleep(5);
	while(fgets(line,20,fpin) != NULL)
	{
	
		if(fputs(line,fpout) == EOF)
		{
			printf("fputs error to pipo");
		}	
	}
	
	exit(0);

	return 0;
}

