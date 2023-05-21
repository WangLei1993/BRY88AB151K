#ifndef _utils_h_
#define _utils_h_

#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace utils 
{

#define TO_STRING(s) #s

// 工具类
class Utils 
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        // MAC地址从字符串转到字节数组
        static uint8_t *MACAddressStringToBytes(std::string mac_address_in_string, bool reverse=false);
        // MAC地址从字节数组转到字符串
        static std::string MACAddressBytesToString(uint8_t *mac_address_in_bytes, bool reverse=false);
        // 获取指定长度的随机串
        static std::string GetRandomString(const uint16_t length=16, 
                                           const bool include_number=true, 
                                           const bool include_punctuation_mark=true);
        // 分割字符串
        static void StringSplit(const std::string &str, std::vector<std::string> &tokens, const std::string &delimiters=" ");
};

}

}

#endif // _utils_hpp_
