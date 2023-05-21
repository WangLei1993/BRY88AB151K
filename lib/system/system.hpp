#ifndef _system_hpp_
#define _system_hpp_

#include <vector>
#include <string>

#include "lwip/netdb.h"

namespace cubestone_wang 
{

namespace system 
{

// 系统类
class System
{
    public:
        // 任务信息
        struct TaskInfo {
            uint32_t Id;
            std::string Name;
            uint32_t Priority;
            eTaskState State;
            uint32_t Runtime;
            uint32_t StackHighWaterMark;
            int32_t CoreID;
        };
        // 任务信息总结
        struct TaskInfoSummary {
            uint32_t TotalRuntime;
            std::vector<TaskInfo> TaskInfoList;
        };
        /**
         * @brief 输出硬件信息
         */
        static void LogHardwareInfo();
        /**
         * @brief 输出软件信息
         */
        static void LogSoftwareInfo();
        /**
         * @brief 获取当前任务信息总结
         */
        static TaskInfoSummary GetCurrentTaskInfoSummary();
        /**
         * @brief 获取当前空堆大小
         */
        static uint32_t GetCurrentFreeHeapSize();
        /**
         * @brief 获取当前最小的空堆大小
         */
        static uint32_t GetCurrentMinimumFreeHeapSize();
        /**
         * @brief 获取当前时间戳（秒）
         */
        static time_t GetCurrentTimestamp();
        /**
         * @brief 获取当前时间
         */
        static struct tm GetCurrentTime();
        /**
         * @brief 获取当前时间标准串
         */
        static std::string GetCurrentTimeStandardString();
        /**
         * @brief 获取启动后的持续时间戳（秒）
         * 
         * 将毫秒级的时间戳降级到秒级，以支持更长的时间
         */
        static time_t GetStartupTimestamp();
        /**
         * @brief 获取当前时间串
         */
        static std::string GetStartupTimeString();
        /**
         * @brief 休眠函数
         *
         * @param milliseconds 毫秒
         */
        static void Sleep(const uint32_t milliseconds);  
        /**
         * @brief 重启函数
         *
         * @param reason 原因
         * @param delay 延迟（秒）
         */
        static void Restart(const char *const reason=nullptr, const uint32_t delay=3);
        static bool DomainToIPAddress(const std::string &domain, ip_addr_t *ip);
        // 日志标签
        static const char *const LOG_TAG;
    private:
        static bool restart_flag;
        static SemaphoreHandle_t restart_mutex;
};

}

}

#endif // _system_hpp_
