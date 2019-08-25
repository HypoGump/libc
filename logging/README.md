# 异步日志

## Feature

+ 支持多种级别日志
+ 支持自定义日志输出
+ 双缓冲异步输出
+ C语言 `printf` 样式调用

## 使用

1. 引入头文件 `log.h` 即可使用
2. 开启异步日志需要先调用 `set_log_async_enabled()` ，再调用 `log_init()` 开启日志后端线程

## 性能测试

```shell
# /dev/null
0.431751 seconds, 133888890 bytes, 2316149.82 msg/s, 295.74 MiB/s
# /tmp/log
0.478158 seconds, 133888890 bytes, 2091358.92 msg/s, 267.04 MiB/s
# local file
0.601992 seconds, 133888890 bytes, 1661151.64 msg/s, 212.11 MiB/s
```

