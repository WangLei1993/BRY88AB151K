#include <stdlib.h>
#include <string.h>

#include "esp_log.h"

#include "system.hpp"

#include "utils.hpp"

namespace cubestone_wang 
{

namespace utils 
{

using namespace system;

const char *const Utils::LOG_TAG = "UTILS";

uint8_t *Utils::MACAddressStringToBytes(std::string mac_address_in_string, bool reverse)
{
    uint8_t *mac_address_in_bytes = new uint8_t[6];
    if (reverse) {
        mac_address_in_bytes[0] = (uint8_t)strtol(mac_address_in_string.substr(0,2).c_str(), nullptr, 16);
        mac_address_in_bytes[1] = (uint8_t)strtol(mac_address_in_string.substr(3,2).c_str(), nullptr, 16);
        mac_address_in_bytes[2] = (uint8_t)strtol(mac_address_in_string.substr(6,2).c_str(), nullptr, 16);
        mac_address_in_bytes[3] = (uint8_t)strtol(mac_address_in_string.substr(9,2).c_str(), nullptr, 16);
        mac_address_in_bytes[4] = (uint8_t)strtol(mac_address_in_string.substr(12,2).c_str(), nullptr, 16);
        mac_address_in_bytes[5] = (uint8_t)strtol(mac_address_in_string.substr(15,2).c_str(), nullptr, 16);
    } else {
        mac_address_in_bytes[5] = (uint8_t)strtol(mac_address_in_string.substr(0,2).c_str(), nullptr, 16);
        mac_address_in_bytes[4] = (uint8_t)strtol(mac_address_in_string.substr(3,2).c_str(), nullptr, 16);
        mac_address_in_bytes[3] = (uint8_t)strtol(mac_address_in_string.substr(6,2).c_str(), nullptr, 16);
        mac_address_in_bytes[2] = (uint8_t)strtol(mac_address_in_string.substr(9,2).c_str(), nullptr, 16);
        mac_address_in_bytes[1] = (uint8_t)strtol(mac_address_in_string.substr(12,2).c_str(), nullptr, 16);
        mac_address_in_bytes[0] = (uint8_t)strtol(mac_address_in_string.substr(15,2).c_str(), nullptr, 16);
    }
    return mac_address_in_bytes;
}
        
std::string Utils::MACAddressBytesToString(uint8_t *mac_address_in_bytes, bool reverse)
{
    char mac_address_in_char_array[18];
    memset(mac_address_in_char_array, 0, sizeof(mac_address_in_char_array));
    if (reverse) {
        sprintf(mac_address_in_char_array, 
                "%02x:%02x:%02x:%02x:%02x:%02x",
                mac_address_in_bytes[0], 
                mac_address_in_bytes[1],
                mac_address_in_bytes[2],
                mac_address_in_bytes[3],
                mac_address_in_bytes[4],
                mac_address_in_bytes[5]);
    } else {
        sprintf(mac_address_in_char_array, 
                "%02x:%02x:%02x:%02x:%02x:%02x",
                mac_address_in_bytes[5], 
                mac_address_in_bytes[4],
                mac_address_in_bytes[3],
                mac_address_in_bytes[2],
                mac_address_in_bytes[1],
                mac_address_in_bytes[0]);
    }
    return std::string(mac_address_in_char_array);
}

std::string Utils::GetRandomString(const uint16_t length, 
                                   const bool include_number, 
                                   const bool include_punctuation_mark)
{
    static char *letter_list = (char *)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static char *number_list = (char *)"0123456789";
    static char *punctuation_mark_list = (char *)"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    static uint8_t letter_length = strlen(letter_list);
    static uint8_t number_length = strlen(number_list);
    static uint8_t punctuation_mark_length = strlen(punctuation_mark_list);
    uint16_t random_length = letter_length;
    uint16_t random_index;
    if (true == include_number) {
        random_length += number_length;
    }
    if(true == include_punctuation_mark) {
        random_length += punctuation_mark_length;
    }
    srand(System::GetCurrentTimestamp());
    std::string result;
    for (uint16_t index=0; index<length; index++) {
        random_index = (uint16_t)(rand() % random_length);
        if (random_index < letter_length) {
            result.append(1, letter_list[random_index]);
            continue;
        }
        random_index -= letter_length;
        if (true == include_number) {
            if (random_index < number_length) {
                result.append(1, number_list[random_index]);
                continue;
            } else {
                random_index -= number_length;
            }
        }
        if(true == include_punctuation_mark) {
            if (random_index < punctuation_mark_length) {
                result.append(1, punctuation_mark_list[random_index]);
                continue;
            } else {
                // 逻辑上此行永远执行不到
                ESP_LOGE(LOG_TAG, "GetRandomString failed");
            }
        }
    }
    return result;
}

void Utils::StringSplit(const std::string &str, std::vector<std::string> &tokens, const std::string &delimiters)
{
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);
    while (std::string::npos != pos || std::string::npos != lastPos) {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

}

}
