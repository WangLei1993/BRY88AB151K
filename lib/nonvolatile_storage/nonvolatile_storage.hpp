#ifndef _nonvolatile_storage_hpp_
#define _nonvolatile_storage_hpp_

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace nonvolatile_storage 
{

// 非易失存储类
class NonvolatileStorage
{
    public:
        /**
         * @brief 初始化
         */
        static void Init();
        // 日志标签
        static const char *const LOG_TAG;
        static std::string ReadString(const std::string &ns, const std::string &key);
        static bool WriteString(const std::string &ns, const std::string &key, const char *const value);
        static bool WriteString(const std::string &ns, const std::string &key, const std::string &value);
    private:
        static bool init_flag;
        static SemaphoreHandle_t mutex;
};

}

}

#endif // _nonvolatile_storage_hpp_
