#include "utils.h"
#include "myTime.h"
#include <ace/UUID.h>
#ifdef WIN32
#include <Windows.h>
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif  // WIN32
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <memory>

std::vector<std::string> s_split(const std::string& in, const std::string& delim)
{
    size_t nPos = 0;
    std::vector<std::string> vec;
    std::string src = in;
    nPos = src.find(delim.c_str());
    while(std::string::npos != nPos)
    {
        vec.push_back(src.substr(0, nPos));
        src = src.substr(nPos + delim.length());
        nPos = src.find(delim.c_str());
    }
    vec.push_back(src);
    return vec;
}

std::string genuuid()
{
    ACE_Utils::UUID_GENERATOR::instance()->init();
    ACE_Utils::UUID uuid;
    ACE_Utils::UUID_GENERATOR::instance()->generate_UUID(uuid);
    std::string suuid = uuid.to_string()->c_str();
    auto vec = s_split(suuid, "-");
    std::string ret;
    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it)
    {
        ret += *it;
    }
    return ret;
}

int json2str(const Json::Value &jnode, std::string &sValue)
{
    sValue = jnode.toStyledString();
    return 0;
}

int str2json(const std::string &sValue, Json::Value &jnode)
{
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> const reader(readerBuilder.newCharReader());
    std::string errorMsg;
    if (!reader->parse(sValue.c_str(), 
        sValue.c_str() + sValue.length(), 
        &jnode,
        &errorMsg))
    {
        return -1;
    }
    return 0;
}

int u8tobytes(uint8_t value, uint8_t *buf, int bufSize)
{
    if (buf == NULL || bufSize < (int)sizeof(value))
    {
        return 0;
    }
    *buf = value;
    return 1;
}

int u16tobytes(uint16_t value, uint8_t *buf, int bufSize)
{
    if (buf == NULL || bufSize < (int)sizeof(value))
    {
        return 0;
    }
    *buf++ = (uint8_t)(value >> 8);
    *buf = (uint8_t)(value);
    return 2;
}

int u32tobytes(uint32_t value, uint8_t *buf, int bufSize)
{
    if (buf == NULL || bufSize < (int)sizeof(value))
    {
        return 0;
    }
    *buf++ = (uint8_t)(value >> 24);
    *buf++ = (uint8_t)(value >> 16);
    *buf++ = (uint8_t)(value >> 8);
    *buf = (uint8_t)(value);
    return 4;
}

int u64tobytes(uint64_t value, uint8_t *buf, int bufSize)
{
    if (buf == NULL || bufSize < (int)sizeof(value))
    {
        return 0;
    }
    *buf++ = (uint8_t)(value >> 56);
    *buf++ = (uint8_t)(value >> 48);
    *buf++ = (uint8_t)(value >> 40);
    *buf++ = (uint8_t)(value >> 32);
    *buf++ = (uint8_t)(value >> 24);
    *buf++ = (uint8_t)(value >> 16);
    *buf++ = (uint8_t)(value >> 8);
    *buf = (uint8_t)(value);
    return 8;
}

uint8_t bytes2u8(const uint8_t *buf)
{
    return *buf;
}

uint16_t bytes2u16(const uint8_t *buf)
{
    return ntohs(*(uint16_t*)(buf));
}

uint32_t bytes2u32(const uint8_t *buf)
{
    return ntohl(*(uint32_t*)(buf));
}

uint64_t bytes2u64(const uint8_t *buf)
{
    return (((uint64_t)ntohl(*(uint32_t*)(buf))) << 32) 
            | (ntohl(*(uint32_t*)(buf + 4)));
}

uint32_t ipstr2u32(const char* ip)
{
    return ntohl((uint32_t)inet_addr(ip));
}

const char* ipu322str(uint32_t ip)
{
    struct in_addr ina;
    ina.s_addr = htonl(ip);
    return inet_ntoa(ina);
}

const char* devid2str(uint64_t devid, char *buf, int bufSize)
{
    if (bufSize < 29)
    {
        return NULL;
    }
    unsigned int dev_no = (unsigned int)(devid & 0xFFFF);
    unsigned int days = (unsigned int)((devid >> 16) & 0xFFFF);
    unsigned int customer_id = (unsigned int)((devid >> 32) & 0xFFFF);
    unsigned int customer_type = (unsigned int)((devid >> 48) & 0xFF);
    unsigned int dev_type = (unsigned int)((devid >> 56) & 0xFF);

    MyTime refrenceDate(2000, 1, 1, 0, 0, 0);
    MyTimeSpan dateSpan(days, 0, 0, 0);
    MyTime productionDate = refrenceDate + dateSpan;
    unsigned int year = productionDate.getYear();
    unsigned int month = productionDate.getMonth();
    unsigned int day = productionDate.getDay();

    snprintf(buf, bufSize, "%03u-%03u-%05u-%04u%02u%02u-%05u",
        dev_type,
        customer_type,
        customer_id,
        year,
        month,
        day,
        dev_no);
    return buf;
}

uint64_t str2devid(const char* devid)
{
    if (strlen(devid) != 28)
    {
        return 0;
    }
    unsigned int dev_type = 0;
    unsigned int customer_type = 0;
    unsigned int customer_id = 0;
    unsigned int dev_no = 0;
    unsigned int year, month, day, days;
    if (7 != sscanf(devid, "%03u-%03u-%05u-%04u%02u%02u-%05u",
        &dev_type,
        &customer_type,
        &customer_id,
        &year,
        &month,
        &day,
        &dev_no))
    {
        return 0;
    }
    MyTime productionDate(year, month, day, 0, 0, 0);
    MyTime refrenceDate(2000, 1, 1, 0, 0, 0);
    MyTimeSpan dataSpan = productionDate - refrenceDate;
    days = dataSpan.getDays();
    
    uint64_t ret = 0;
    ret = (uint64_t)dev_type & 0xFF;
    ret = (ret << 8) | (customer_type & 0xFF);
    ret = (ret << 16) | (customer_id & 0xFFFF);
    ret = (ret << 16) | (days & 0xFFFF);
    ret = (ret << 16) | (dev_no & 0xFFFF);
    return ret;
}

const char* customerId2Str(uint32_t customerId, char *buf, int bufSize)
{
    if (bufSize < 9)
    {
        return NULL;
    }
    MyTime refrenceDate(2000, 1, 1, 0, 0, 0);
    MyTimeSpan dateSpan(customerId, 0, 0, 0);
    MyTime productionDate = refrenceDate + dateSpan;
    unsigned int year = productionDate.getYear();
    unsigned int month = productionDate.getMonth();
    unsigned int day = productionDate.getDay();
    snprintf(buf, bufSize, "%04u%02u%02u", year, month, day);
    return buf;
}


devidCache* devidCache::m_instance = NULL;

devidCache* devidCache::instance()
{
    if (m_instance == NULL)
    {
        m_instance = new devidCache();
    }
    return m_instance;
}

void devidCache::destory()
{
    if (m_instance != NULL)
    {
        delete m_instance;
        m_instance = NULL;
    }
}