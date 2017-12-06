


[TOC]

大部分测试用例都是不能够按照预期正常运行的,也就是我们编程中可能常常遇到的错误和问题.思考每个函数内核实现,这些问题不可避免,这里拿出经典,而且非常简洁的例子告诉我们通用的道理 难能可贵


## 10  Signal

- [ ] 10.1 
- [ ] 10.2 
- [ ] 10.3
- [ ] 10.4  signal function
- [ ] 10.5  Signal System interrupt 
- [ ] 10.5
 

### 10.4 signal function

```
Linux 2.0之前版本
void (*signal (int signo, void (*func)(int))) (int);
Linux 2.6 版本
typedef void (*__sighandler_t) (int);
extern __sighandler_t signal (int __sig, __sighandler_t __handler)
```

### 10.5 中断当系统调用
#### 1. 历史原因

unix系统的一个特性是如果进程在执行一个低速系统调用而阻塞期间捕捉到一个信号，则该系统调用就被终端不再继续执行，该系统调用返回出错其error被设置为EINTR 这样的处理理由是因为一个信号发生了，进程捕捉到了它，这意味着已经发生了某种事情，所以是一个应当唤醒阻塞的系统调用的好机会

#### 2. 系统调用分类
> 低速系统调用（低速系统调用是可能会使进程永远阻塞的一类系统调用）

 - 在读取某些类型的文件(管道终端设备以及网络设备)时  如果数据不存在可能会是使调用者永远阻塞
 - 在写这些类型的文件时    如果不能立即接受这些数据 则可能会永远阻塞
 - 打开文件 在某种条件发生之前也可能会使调用阻塞

> 其他系统调用
 - pause 和 wait 函数
 - 某些 ioctl
 - 某些进程间通信函数 

> 例外 磁盘io可能是一种非系统调用，写一个磁盘可能暂时阻塞调用者（在磁盘驱动器将请求排入队列，然后在适当的时间执行请求） 但是除非发生硬件错误， io 操作总会很快返回


**区分系统调用和函数**
*当捕捉到某个信号时*  




被终端的是内核中执行的系统调用
- [ ] 
- [ ]  
- [ ]  

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






10.5. Interrupted System Calls
系统调用

A characteristic of earlier UNIX systems is that 
早期unix特性是这样的

if a process caught a signal while the process was blocked in a "slow" system call,
如果阻塞系统调用的进程捕获到一个信号

the system call was interrupted. 
那么系统调用被 interrupted。

The system call returned an error and errno was set to EINTR. 
系统调用会出错  出错后返回值是中断信号的值。


This was done under the assumption that 
当信号发生并且被进程捕获（意味着发生了什么事情）
是唤醒一个blocked（阻塞）当系统调用当绝佳时机。
since a signal occurred and the process caught it, there is a good chance that something has happened that should wake up the blocked system call.


Here, we have to differentiate between a system call and a function. 
请区分systemcall 和 function


It is a system call within the kernel that is interrupted when a signal is caught.
系统调用当意思是，在内核中，当一个信号被捕获系统调用被中断


To support this feature, the system calls are divided into two categories: the "slow" system calls and all the others. 
为了支持上面两个特性，系统调用被氛围2种，   

1慢速系统调用
2其他


The slow system calls are those that can block forever.
慢速系统调用时可以被永远block（阻塞）当

 Included in this category are
Reads that can block the caller forever if data isn't present with certain file types (pipes, terminal devices, and network devices)Writes that can block the caller forever if the data can't be accepted immediately by these same file types

Opens that block until some condition occurs on certain file types (such as an open of a terminal device that waits until an attached modem answers the phone)

The pause function (which by definition puts the calling process to sleep until a signal is caught) and the wait function

Certain ioctl operations

Some of the interprocess communication functions (Chapter 15)

The notable exception to these slow system calls is anything related to disk I/O. Although a read or a write of a disk file can block the caller temporarily (while the disk driver queues the request and then the request is executed), unless a hardware error occurs, the I/O operation always returns and unblocks the caller quickly.

One condition that is handled by interrupted system calls, for example, is when a process initiates a read from a terminal device and the user at the terminal walks away from the terminal for an extended period. In this example, the process could be blocked for hours or days and would remain so unless the system was taken down.

POSIX.1 semantics for interrupted reads and writes changed with the 2001 version of the standard. Earlier versions gave implementations a choice for how to deal with reads and writes that have processed partial amounts of data. If read has received and transferred data to an application's buffer, but has not yet received all that the application requested and is then interrupted, the operating system could either fail the system call with errno set to EINTR or allow the system call to succeed, returning the partial amount of data received. Similarly, if write is interrupted after transferring some of the data in an application's buffer, the operation system could either fail the system call with errno set to EINTR or allow the system call to succeed, returning the partial amount of data written. Historically, implementations derived from System V fail the system call, whereas BSD-derived implementations return partial success. With the 2001 version of the POSIX.1 standard, the BSD-style semantics are required.

The problem with interrupted system calls is that we now have to handle the error return explicitly. The typical code sequence (assuming a read operation and assuming that we want to restart the read even if it's interrupted) would be

    again:
        if ((n = read(fd, buf, BUFFSIZE)) < 0) {
            if (errno == EINTR)
                goto again;     /* just an interrupted system call */
            /* handle other errors */
        }

To prevent applications from having to handle interrupted system calls, 4.2BSD introduced the automatic restarting of certain interrupted system calls. The system calls that were automatically restarted are ioctl, read, readv, write, writev, wait, and waitpid. As we've mentioned, the first five of these functions are interrupted by a signal only if they are operating on a slow device; wait and waitpid are always interrupted when a signal is caught. Since this caused a problem for some applications that didn't want the operation restarted if it was interrupted, 4.3BSD allowed the process to disable this feature on a per signal basis.

POSIX.1 allows an implementation to restart system calls, but it is not required. The Single UNIX Specification defines the SA_RESTART flag as an XSI extension to sigaction to allow applications to request that interrupted system calls be restarted.

System V has never restarted system calls by default. BSD, on the other hand, restarts them if interrupted by signals. By default, FreeBSD 5.2.1, Linux 2.4.22, and Mac OS X 10.3 restart system calls interrupted by signals. The default on Solaris 9, however, is to return an error (EINTR) instead.

One of the reasons 4.2BSD introduced the automatic restart feature is that sometimes we don't know that the input or output device is a slow device. If the program we write can be used interactively, then it might be reading or writing a slow device, since terminals fall into this category. If we catch signals in this program, and if the system doesn't provide the restart capability, then we have to test every read or write for the interrupted error return and reissue the read or write.

Figure 10.3 summarizes the signal functions and their semantics provided by the various implementations.

Figure 10.3. Features provided by various signal implementations

Functions

System

Signal handler remains installed

Ability to block signals

Automatic restart of interrupted system calls?

signal

ISO C, POSIX.1

unspecified

unspecified

unspecified

V7, SVR2, SVR3, SVR4, Solaris

 	 	
never

4.2BSD

•

•

always

4.3BSD, 4.4BSD, FreeBSD, Linux, Mac OS X

•

•

default

sigset

XSI

•

•

unspecified

SVR3, SVR4, Linux, Solaris

•

•

never

sigvec

4.2BSD

•

•

always

4.3BSD, 4.4BSD, FreeBSD, Mac OS X

•

•

default

sigaction

POSIX.1

•

•

unspecified

XSI, 4.4BSD, SVR4, FreeBSD, Mac OS X, Linux, Solaris

•

•

optional



We don't discuss the older sigset and sigvec functions. Their use has been superceded by the sigaction function; they are included only for completeness. In contrast, some implementations promote the signal function as a simplified interface to sigaction.

Be aware that UNIX systems from other vendors can have values different from those shown in this figure. For example, sigaction under SunOS 4.1.2 restarts an interrupted system call by default, different from the platforms listed in Figure 10.3.

In Figure 10.18, we provide our own version of the signal function that automatically tries to restart interrupted system calls (other than for the SIGALRM signal). In Figure 10.19, we provide another function, signal_intr, that tries to never do the restart.

We talk more about interrupted system calls in Section 14.5 with regard to the select and poll functions.
















----------
[『Cmd 技术渲染的沙箱页面，点击此处编写自己的文档』](https://www.zybuluo.com/mdeditor "作业部落旗下 Cmd 在线 Markdown 编辑阅读器")

# Cmd Markdown 简明语法手册

标签： Cmd-Markdown

---

### 1. 斜体和粗体

使用 * 和 ** 表示斜体和粗体。

示例：

这是 *斜体*，这是 **粗体**。

### 2. 分级标题

使用 === 表示一级标题，使用 --- 表示二级标题。

示例：

```
这是一个一级标题
============================

这是一个二级标题
--------------------------------------------------

### 这是一个三级标题
```

你也可以选择在行首加井号表示不同级别的标题 (H1-H6)，例如：# H1, ## H2, ### H3，#### H4。

### 3. 外链接

使用 \[描述](链接地址) 为文字增加外链接。

示例：

这是去往 [本人博客](http://ghosertblog.github.com) 的链接。

### 4. 无序列表

使用 *，+，- 表示无序列表。

示例：

- 无序列表项 一
- 无序列表项 二
- 无序列表项 三

### 5. 有序列表

使用数字和点表示有序列表。

示例：

1. 有序列表项 一
2. 有序列表项 二
3. 有序列表项 三

### 6. 文字引用

使用 > 表示文字引用。

示例：

> 野火烧不尽，春风吹又生。

### 7. 行内代码块

使用 \`代码` 表示行内代码块。

示例：

让我们聊聊 `html`。

### 8.  代码块

使用 四个缩进空格 表示代码块。

示例：

    这是一个代码块，此行左侧有四个不可见的空格。

### 9.  插入图像

使用 \!\[描述](图片链接地址) 插入图像。

示例：

![我的头像](https://www.zybuluo.com/static/img/my_head.jpg)

# Cmd Markdown 高阶语法手册

### 1. 内容目录

在段落中填写 `[TOC]` 以显示全文内容的目录结构。

[TOC]

### 2. 标签分类

在编辑区任意行的列首位置输入以下代码给文稿标签：

标签： 数学 英语 Markdown

或者

Tags： 数学 英语 Markdown

### 3. 删除线

使用 ~~ 表示删除线。

~~这是一段错误的文本。~~

### 4. 注脚

使用 [^keyword] 表示注脚。

这是一个注脚[^footnote]的样例。

这是第二个注脚[^footnote2]的样例。

### 5. LaTeX 公式

$ 表示行内公式： 

质能守恒方程可以用一个很简洁的方程式 $E=mc^2$ 来表达。

$$ 表示整行公式：

$$\sum_{i=1}^n a_i=0$$

$$f(x_1,x_x,\ldots,x_n) = x_1^2 + x_2^2 + \cdots + x_n^2 $$

$$\sum^{j-1}_{k=0}{\widehat{\gamma}_{kj} z_k}$$

访问 [MathJax](http://meta.math.stackexchange.com/questions/5020/mathjax-basic-tutorial-and-quick-reference) 参考更多使用方法。

### 6. 加强的代码块

支持四十一种编程语言的语法高亮的显示，行号显示。

非代码示例：

```
$ sudo apt-get install vim-gnome
```

Python 示例：

```python
@requires_authorization
def somefunc(param1='', param2=0):
    '''A docstring'''
    if param1 > param2: # interesting
        print 'Greater'
    return (param2 - param1 + 1) or None

class SomeClass:
    pass

>>> message = '''interpreter
... prompt'''
```

JavaScript 示例：

``` javascript
/**
* nth element in the fibonacci series.
* @param n >= 0
* @return the nth element, >= 0.
*/
function fib(n) {
  var a = 1, b = 1;
  var tmp;
  while (--n >= 0) {
    tmp = a;
    a += b;
    b = tmp;
  }
  return a;
}

document.write(fib(10));
```

### 7. 流程图

#### 示例

```flow
st=>start: Start:>https://www.zybuluo.com
io=>inputoutput: verification
op=>operation: Your Operation
cond=>condition: Yes or No?
sub=>subroutine: Your Subroutine
e=>end

st->io->op->cond
cond(yes)->e
cond(no)->sub->io
```

#### 更多语法参考：[流程图语法参考](http://adrai.github.io/flowchart.js/)

### 8. 序列图

#### 示例 1

```seq
Alice->Bob: Hello Bob, how are you?
Note right of Bob: Bob thinks
Bob-->Alice: I am good thanks!
```

#### 示例 2

```seq
Title: Here is a title
A->B: Normal line
B-->C: Dashed line
C->>D: Open arrow
D-->>A: Dashed open arrow
```

#### 更多语法参考：[序列图语法参考](http://bramp.github.io/js-sequence-diagrams/)

### 9. 甘特图

甘特图内在思想简单。基本是一条线条图，横轴表示时间，纵轴表示活动（项目），线条表示在整个期间上计划和实际的活动完成情况。它直观地表明任务计划在什么时候进行，及实际进展与计划要求的对比。

```gantt
    title 项目开发流程
    section 项目确定
        需求分析       :a1, 2016-06-22, 3d
        可行性报告     :after a1, 5d
        概念验证       : 5d
    section 项目实施
        概要设计      :2016-07-05  , 5d
        详细设计      :2016-07-08, 10d
        编码          :2016-07-15, 10d
        测试          :2016-07-22, 5d
    section 发布验收
        发布: 2d
        验收: 3d
```

#### 更多语法参考：[甘特图语法参考](https://knsv.github.io/mermaid/#gant-diagrams)

### 10. Mermaid 流程图

```graphLR
    A[Hard edge] -->|Link text| B(Round edge)
    B --> C{Decision}
    C -->|One| D[Result one]
    C -->|Two| E[Result two]
```

#### 更多语法参考：[Mermaid 流程图语法参考](https://knsv.github.io/mermaid/#flowcharts-basic-syntax)

### 11. Mermaid 序列图

```sequence
    Alice->John: Hello John, how are you?
    loop every minute
        John-->Alice: Great!
    end
```

#### 更多语法参考：[Mermaid 序列图语法参考](https://knsv.github.io/mermaid/#sequence-diagrams)

### 12. 表格支持

| 项目        | 价格   |  数量  |
| --------   | -----:  | :----:  |
| 计算机     | \$1600 |   5     |
| 手机        |   \$12   |   12   |
| 管线        |    \$1    |  234  |


### 13. 定义型列表

名词 1
:   定义 1（左侧有一个可见的冒号和四个不可见的空格）

代码块 2
:   这是代码块的定义（左侧有一个可见的冒号和四个不可见的空格）

        代码块（左侧有八个不可见的空格）

### 14. Html 标签

本站支持在 Markdown 语法中嵌套 Html 标签，譬如，你可以用 Html 写一个纵跨两行的表格：

    <table>
        <tr>
            <th rowspan="2">值班人员</th>
            <th>星期一</th>
            <th>星期二</th>
            <th>星期三</th>
        </tr>
        <tr>
            <td>李强</td>
            <td>张明</td>
            <td>王平</td>
        </tr>
    </table>


<table>
    <tr>
        <th rowspan="2">值班人员</th>
        <th>星期一</th>
        <th>星期二</th>
        <th>星期三</th>
    </tr>
    <tr>
        <td>李强</td>
        <td>张明</td>
        <td>王平</td>
    </tr>
</table>

### 15. 内嵌图标

本站的图标系统对外开放，在文档中输入

    <i class="icon-weibo"></i>

即显示微博的图标： <i class="icon-weibo icon-2x"></i>

替换 上述 `i 标签` 内的 `icon-weibo` 以显示不同的图标，例如：

    <i class="icon-renren"></i>

即显示人人的图标： <i class="icon-renren icon-2x"></i>

更多的图标和玩法可以参看 [font-awesome](http://fortawesome.github.io/Font-Awesome/3.2.1/icons/) 官方网站。

### 16. 待办事宜 Todo 列表

使用带有 [ ] 或 [x] （未完成或已完成）项的列表语法撰写一个待办事宜列表，并且支持子列表嵌套以及混用Markdown语法，例如：

    - [ ] **Cmd Markdown 开发**
        - [ ] 改进 Cmd 渲染算法，使用局部渲染技术提高渲染效率
        - [ ] 支持以 PDF 格式导出文稿
        - [x] 新增Todo列表功能 [语法参考](https://github.com/blog/1375-task-lists-in-gfm-issues-pulls-comments)
        - [x] 改进 LaTex 功能
            - [x] 修复 LaTex 公式渲染问题
            - [x] 新增 LaTex 公式编号功能 [语法参考](http://docs.mathjax.org/en/latest/tex.html#tex-eq-numbers)
    - [ ] **七月旅行准备**
        - [ ] 准备邮轮上需要携带的物品
        - [ ] 浏览日本免税店的物品
        - [x] 购买蓝宝石公主号七月一日的船票
        
对应显示如下待办事宜 Todo 列表：
        
- [ ] **Cmd Markdown 开发**
    - [ ] 改进 Cmd 渲染算法，使用局部渲染技术提高渲染效率
    - [ ] 支持以 PDF 格式导出文稿
    - [x] 新增Todo列表功能 [语法参考](https://github.com/blog/1375-task-lists-in-gfm-issues-pulls-comments)
    - [x] 改进 LaTex 功能
        - [x] 修复 LaTex 公式渲染问题
        - [x] 新增 LaTex 公式编号功能 [语法参考](http://docs.mathjax.org/en/latest/tex.html#tex-eq-numbers)
- [ ] **七月旅行准备**
    - [ ] 准备邮轮上需要携带的物品
    - [ ] 浏览日本免税店的物品
    - [x] 购买蓝宝石公主号七月一日的船票
        
        
[^footnote]: 这是一个 *注脚* 的 **文本**。

[^footnote2]: 这是另一个 *注脚* 的 **文本**。

