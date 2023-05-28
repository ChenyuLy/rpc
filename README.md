### 日志模块开发
首先需要创建项目：

日志模块：
```
1.日志级别
2.打印到文件，支持日期命名，以及日志的滚动
3.c格式化风格
4.线程安全
```

LogLevel：
```
Debug
Info
Error
```

LogEvent:
```
行号，文件名
MsgNo
Thread id 
进程号

日期 时间，精确到毫秒
自定义消息
```

日志格式
```
[level][%y-%m-%d %H:%M:%ms]\t[pid:thread_id]\t[file_name:line][%msg]s
```

Logger 日志器
1.提供打印日志的方法
2.设置日志的输出路径


### 2.4 Reactors
事件循环器
```c++{.line-numbers}
void loop{
    while (!stop){
        foreach(task in tasks){
            task;
        }
        
        //1.取得下次定时任务的时间，与设定timeout取最大值，若下次定时任务时间超过1s就取下次定时任务时间为超时，否者为1s
        int time_out = Max(1000,getNextTimerCallback());
        //2.调用Epoll等待时间的发生，超时时间为上述的timeOut
        int rt = epoll_wait(epfd,fds,....,time_out);
        if(rt < 0>){
            //epoll 调用失效。。。
        } else {
            if(rt >0 ){
                foreach(fd in fds){
                    tasks.push(fd);
                }
            }
        }

        
    }
}
```


在rocket中，使用的是主从Reactor模型

服务器有一个mainReactor 和多个subReactor。
mainReactor由主线程运行 他的作用如下：通过epoll监听listenfd的可读事件，当可读时间发生以后，调用accept函数获取clientfd,然后随即取出一个subReactor,将clientfd的读写时间注册
到这个subReactor的epoll上即可。也就是说mainReactor只负责建立链接事件,不进行业务处理，也不关心已经链接套接字的IO事件。

subReactor通常有多个，每个subReactor由一个线程来运行，subReactor的epoll中注册了clientfd的读写时间，当发了IO事件后，需要进行业务处理



#### 2.4.1 TimerEvent 定时任务


```
1.指定时间点 arrive_time
2.interval,ms; 
3.isrepeated 是否是周期性的
4.is_cancled
5.task

canle()
cancleRepeated()
```


#### 2.4.2 Timer
定时器 他是一个TimerEvent的集合
Timer 继承FdEvent


‘’‘
addTimeEvent();
deleteTimerEvent();
onTimer();//发生IO以后需要执行的方法
reserArriveTime(); 

mutimap 存储 TimerEvent <key(arrivetime),TimerEvent>



’‘’