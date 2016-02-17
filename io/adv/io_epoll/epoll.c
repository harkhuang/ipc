#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BUFSIZE        1024


//以下定义的两个宏相当于终端
#define TTY1        "/dev/tty11"
#define TTY2        "/dev/tty12"

/* 状态机的各种状态 */
/* Finite State Machine' state */
enum
{
    STATE_R=1,          //read        state
    STATE_W,            //write       state
    STATE_Ex,           //exe         state
    STATE_T             //termination state
};

/* 状态机，根据不同的需求设计不同的成员 */
/* Finite State Machine's struct */
struct fsm_st
{
    int state;               // 状态机当前的状态
    int sfd;                 // 读取的来源文件描述符
    int dfd;                 // 写入的目标文件描述符
    char buf[BUFSIZE];       // 缓冲
    int len;                 // 一次读取到的实际数据量
    int pos;                 // buf 的偏移量，用于记录坚持写够 n 个字节时每次循环写到了哪里
    char *errstr;            // 错误消息
};

/* 状态机驱动 */
/* Finite State Machine driver */
static void fsm_driver(struct fsm_st *fsm)
{
    int ret;

    switch(fsm->state)
    {
        //read able 
        case STATE_R: // 读态
            fsm->len = read(fsm->sfd,fsm->buf,BUFSIZE);
            if(fsm->len == 0) // 读到了文件末尾，将状态机推向 T态
            {
                fsm->state = STATE_T;
            }
            else if(fsm->len<0)// 读取出现异常
            {
                if(errno == EAGAIN) // 如果是假错就推到 读态，重新读一次
                {
                    fsm->state = STATE_R;
                }
                    
                else // 如果是真错就推到 异常态
                {
                    fsm->errstr = "read()";
                    fsm->state = STATE_Ex;
                }
            }
            else // 成功读取到了数据，将状态机推到 写态
            {
                fsm->pos = 0;
                fsm->state = STATE_W;
            }
            break;
        //write able
        case STATE_W: // 写态
            ret = write(fsm->dfd,fsm->buf+fsm->pos,fsm->len);
            if(ret < 0) // 写入出现异常
            {
                if(errno == EAGAIN) // 如果是假错就再次推到 写态，重新再写入一次
                {    
                    fsm->state = STATE_W;
                }
                else // 如果是真错就推到 异常态
                {
                    fsm->errstr = "write()";
                    fsm->state = STATE_Ex;
                }
            }
            else // 成功写入了数据
            {
                fsm->pos += ret;
                fsm->len -= ret;
                if(fsm->len == 0) // 如果将读到的数据完全写出去了就将状态机推向 读态，开始下一轮读取
                {
                    fsm->state = STATE_R;
                
                }
                    

                else // 如果没有将读到的数据完全写出去，那么状态机依然推到 写态，下次继续写入没写完的数据，实现“坚持写够 n 个字节”
                {
                    fsm->state = STATE_W;
                }
                
            }

            break;
        // 
        case STATE_Ex: // 异常态，打印异常并将状态机推到 T态
            perror(fsm->errstr);
            fsm->state = STATE_T;
            break;

        case STATE_T: // 结束态，在这个例子中结束态没有什么需要做的事情，所以空着
            /*do sth */
            break;
        default: // 程序很可能发生了溢出等不可预料的情况，为了避免异常扩大直接自杀
            abort();
    }

}

/* 推动状态机 */
static void relay(int fd1,int fd2)
{
    int fd1_save,fd2_save;    
    // 因为是读 tty1 写 tty2；读 tty2 写 tty1，所以这里的两个状态机直接取名为 fsm12 和 fsm21
    struct fsm_st fsm12,fsm21;

    fd1_save = fcntl(fd1,F_GETFL);
    // 使用状态机操作 IO 一般都采用非阻塞的形式，避免状态机被阻塞
    fcntl(fd1,F_SETFL,fd1_save|O_NONBLOCK);
    fd2_save = fcntl(fd2,F_GETFL);
    fcntl(fd2,F_SETFL,fd2_save|O_NONBLOCK);

    // 在启动状态机之前将状态机推向 读态
    fsm12.state = STATE_R;
    // 设置状态机中读写的来源和目标，这样状态机的读写接口就统一了。在状态机里面不用管到底是 读tty1 写tty2 还是 读tty2 写tty1 了，它只需要知道是 读src 写des 就可以了。
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;

    // 同上
    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;


    // 开始推状态机，只要不是 T态 就一直推
     while(fsm12.state != STATE_T || fsm21.state != STATE_T)
    {
        // 调用状态机驱动函数，状态机开始工作
        fsm_driver(&fsm12);
        fsm_driver(&fsm21);
    }

    fcntl(fd1,F_SETFL,fd1_save);
    fcntl(fd2,F_SETFL,fd2_save);

}

int main()
{
    int fd1,fd2;

    // 假设这里忘记将设备 tty1 以非阻塞的形式打开也没关系，因为推动状态机之前会重新设定文件描述符为非阻塞形式
    fd1 = open(TTY1,O_RDWR);
    if(fd1 < 0)
    {
        perror("open()");
        exit(1);
    }
    write(fd1,"TTY1\n",5);
    
    fd2 = open(TTY2,O_RDWR|O_NONBLOCK);
    if(fd2 < 0)
    {
        perror("open()");
        exit(1);
    }
    write(fd2,"TTY2\n",5);


    relay(fd1,fd2);


    close(fd1);
    close(fd2);


    exit(0);
}
