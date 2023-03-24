/**
 * @file protocolUtils.h
 * @brief  星广播平台工具函数
 * @author Bill
 * @version 0.0.1
 * @date 2021-05-28
 */

#pragma once

#include <stdint.h>
#include <stdint.h>
#include <map>
#include <string>
#include <mutex>
#include <iostream>
#include <json/json.h>


namespace star_protocol
{

int json2str(const Json::Value &jnode, std::string &sValue);
int str2json(const std::string &sValue, Json::Value &jnode);

int u8tobytes(uint8_t value, uint8_t *buf, int bufSize);
int u16tobytes(uint16_t value, uint8_t *buf, int bufSize);
int u32tobytes(uint32_t value, uint8_t *buf, int bufSize);
int u64tobytes(uint64_t value, uint8_t *buf, int bufSize);
uint8_t bytes2u8(const uint8_t *buf);
uint16_t bytes2u16(const uint8_t *buf);
uint32_t bytes2u32(const uint8_t *buf);
uint64_t bytes2u64(const uint8_t *buf);

int hexStringToBytes(const std::string &sValue, uint8_t *buff, int buffSize);

/* IP字符串转uint32 (大端序) */
uint32_t ipstr2u32(const char* ip);

/* IP uint32 (大端序)转字符串 */
const char* ipu322str(uint32_t ip);

/* 终端序列号uint64 to string, bufSize >= 29 */
const char* devid2str(uint64_t devid, char *buf, int bufSize);
/* 终端序列号string to uint64 */
uint64_t str2devid(const char* devid);
/* 从序列号中取客户号, bufSize >= 9 */
const char* customerId2Str(uint32_t customerId, char *buf, int bufSize);
/* 从序列号中取客户ID */
#define CUSTOM_ID_FROM_DEVID(devid)     ((uint32_t)(((devid) >> 32) & 0xFFFF))


/*
 *  终端序列号 字符串与64位无符号整数转换类
 *  保存转换结果，使得转换更高效。
 */
class devidCache
{
public:
    static devidCache* instance();
    static void destory();
    ~devidCache() { std::cout << m_devIdB2SMap.size() << " " << m_devIdS2BMap.size() << std::endl; };

    inline const char* devid2str(uint64_t devid) {
        std::lock_guard<std::mutex> guard(m_mutex);
        std::map<uint64_t, std::string>::iterator it = m_devIdB2SMap.find(devid);
        if (it != m_devIdB2SMap.end())
        {
            return it->second.c_str();
        }
        else
        {
            char buf[32];
            m_devIdB2SMap[devid] = star_protocol::devid2str(devid, buf, sizeof(buf));
            m_devIdS2BMap[buf] = devid;
            return m_devIdB2SMap[devid].c_str();
        }
    }
    inline uint64_t str2devid(const std::string &devid) {
        std::lock_guard<std::mutex> guard(m_mutex);
        std::map<std::string, uint64_t>::iterator it = m_devIdS2BMap.find(devid);
        if (it != m_devIdS2BMap.end())
        {
            return it->second;
        }
        else
        {
            uint64_t bDevId = star_protocol::str2devid(devid.c_str());
            m_devIdS2BMap[devid] = bDevId;
            m_devIdB2SMap[bDevId] = devid;
            return bDevId;
        }
    }
    inline const char* customerId2Str(uint32_t customerId) {
        std::lock_guard<std::mutex> guard(m_mutex);
        std::map<uint32_t, std::string>::iterator it = m_customerIdMap.find(customerId);
        if (it != m_customerIdMap.end())
        {
            return it->second.c_str();
        }
        else
        {
            char buf[32];
            m_customerIdMap[customerId] = star_protocol::customerId2Str(customerId, buf, sizeof(buf));
            return m_customerIdMap[customerId].c_str();
        }
    }

protected:
    devidCache(){};
private:
    std::map<uint64_t, std::string> m_devIdB2SMap;
    std::map<std::string, uint64_t> m_devIdS2BMap;
    std::map<uint32_t, std::string> m_customerIdMap;
    std::mutex m_mutex;
    static devidCache* m_instance;
};

}