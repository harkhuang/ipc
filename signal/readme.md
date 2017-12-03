


[TOC]

## 10  Signal

- [ ] 10.1 
- [ ] 10.2 
- [ ] 10.3
- [ ] 10.4
-  
 
### 10.5
#### 1. 历史原因

unix系统的一个特性是如果进程在执行一个低速系统调用而阻塞期间捕捉到一个信号，则该系统调用就被终端不再继续执行，该系统调用返回出错其error被设置为EINTR 这样的处理理由是因为一个信号发生了，进程捕捉到了它，这意味着已经发生了某种事情，所以是一个应当唤醒阻塞的系统调用的好机会

#### 2. 系统调用分类
> 低速系统调用（低速系统调用是可能会使进程永远阻塞的一类系统调用）

 - 在读取某些类型的文件(管道终端设备以及网络设备)时  如果数据不存在可能会是使调用者永远阻塞
 - 在写这些类型的文件时    如果不能立即接受这些数据 则可能会永远阻塞
 - 打开文件   在某种条件发生之前也可能会使调用阻塞

> 其他系统调用
 - pause 和 wait 函数
 - 某些 ioctl
 - 某些进程间通信函数 


**区分系统调用和函数**
*当捕捉到某个信号时*  




被终端的是内核中执行的系统调用
- [ ] jksfda 
- [ ] asffdas 
- [ ] asdfasdf

Key	Command
ctrl + b	Toggle bold
ctrl + i	Toggle italic
ctrl + shift + ]	Toggle heading (uplevel)
ctrl + shift + [	Toggle heading (downlevel)
alt + c	Check/Uncheck task list item



 

为了支持这种特性  将系统调用分成两类：
1.低速系统调用
   
2其他系统调用

这是` 高亮文本 `效果。


----------
Key	Command
ctrl + b	Toggle bold
ctrl + i	Toggle italic
ctrl + shift + ]	Toggle heading (uplevel)
ctrl + shift + [	Toggle heading (downlevel)

<font color=red>内容</font>  刚学markdown.

<font color=red>内容</font>  刚学markdown.

<font face="STCAIYUN">我是华文彩云</font>

<font color=gray size=5>color=gray</font>

<font color=#0099ff size=5 face="黑体">color=#0099ff 
size=5 face="黑体"</font>