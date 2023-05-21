BRY88AB151K项目说明

## 更新说明

### 0.0.1 ---- 2023.04.21

1. 初始化项目

### 0.0.2 ---- 2023.04.28

1. 调整代码结构
2. 完成各传感器代码

### 0.0.3 ---- 2023.05.19

1. 调整代码
2. 加入u8g2

## 备注说明

### 编译配置

1. 默认关闭64位time_t，工具链不支持
2. 选择选择自定义分区表 √
3. 开启c++异常支持及RTTI √
4. 堆栈崩溃保护模式开到normal √
5. 组件库中开启蓝牙 x
6. 设置bootloader的日志级别为warning √
7. 日志默认级别开到warning,最大日志级别开到debug √
8. 调试时，FreeRTOS开启USE_TRACE_FACILITY，USE_STATS_FORMATTING_FUNCTIONS, VTASKLIST_INCLUDE_COREID，GENERATE_RUN_TIME_STATS  √
9. 蓝牙仅开启NimBLE x
10. 取消CONFIG_ESP32_WIFI_NVS_ENABLED √
11. CONFIG_LWIP_SNTP_MAX_SERVERS设置为3 √
12. 启用CONFIG_ESP_TLS_SERVER，即启用https serve，启用CONFIG_ESP_TLS_CLIENT_SESSION_TICKETS，启用CONFIG_ESP_TLS_SERVER_SESSION_TICKETS √
13. CONFIG_ESP_MAIN_TASK_STACK_SIZE=5120，CONFIG_MAIN_TASK_STACK_SIZE=5120 √
14. CONFIG_HTTPD_MAX_REQ_HDR_LEN=2048 √
15. CONFIG_ESP_TLS_INSECURE=y, CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY=y √
16. CONFIG_ESP_TASK_WDT_PANIC=y
17. CPU频率选择240MHz: CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y, CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ=240

### 目前仅支持IPv4

### 查看代码行数

```
.\cloc-1.66.exe E:\Code\IDF\BRY88AB151K\lib  E:\Code\IDF\BRY88AB151K\src
```

### 执行配置菜单 

`pio run -t menuconfig`

### u8g2

启用U8G2_USE_DYNAMIC_ALLOC, U8X8_WITH_USER_PTR