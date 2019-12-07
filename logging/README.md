# 异步日志

## Feature

+ 支持7级日志，等级高亮，backtrace
+ 支持自定义日志输出函数

+ 带默认参数风格的日志文件参数设置

+ 双缓冲异步输出
+ C语言 `printf` 样式调用

## 使用

1. 引入头文件 `log.h` 即可使用

2. 标准使用流程为

   ```c
   #include "log.h"
   
   int main()
   {
       // 设置日志各项参数
       set_XX();
       // 初始化日志
       log_init();
       
       // 功能逻辑
       ...;
       
       // 关闭日志
       log_exit();
   }
   ```

   

## 性能测试

CPU: i5-8265U  RAM: 8G  OS: WSL Ubuntu-18.04 LTS

```shell
# /dev/null
0.431751 seconds, 133888890 bytes, 2316149.82 msg/s, 295.74 MiB/s
# /tmp/log
0.478158 seconds, 133888890 bytes, 2091358.92 msg/s, 267.04 MiB/s
# local file
0.601992 seconds, 133888890 bytes, 1661151.64 msg/s, 212.11 MiB/s
```

