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