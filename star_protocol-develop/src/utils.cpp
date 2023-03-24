#include "utils.h"
#include "timeutils.h"

#if defined(_WIN32) || defined(WIN32)  || defined(_WIN64) || defined(_WINDOWS)
#include <WinSock2.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif  // WIN32
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <memory>



namespace star_protocol
{


std::vector<std::string> s_split(const std::string& in, const std::string& delim)
{
    int nPos = 0;
    std::vector<std::string> vec;
    std::string src = in;
    nPos = (int)src.find(delim.c_str());
    while(-1 != nPos)
    {
        vec.push_back(src.substr(0, nPos));
        src = src.substr(nPos + delim.length());
        nPos = (int)src.find(delim.c_str());
    }
    vec.push_back(src);
    return vec;
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
	int dev_type = 0;
	int customer_type = 0;
	int customer_id = 0;
	int production_date = 0;
	int dev_no = 0;
	int year, month, day, days;
	dev_no = (int)(devid & 0xFFFF);
	days = (int)((devid >> 16) & 0xFFFF);
	customer_id = (int)((devid >> 32) & 0xFFFF);
	customer_type = (int)((devid >> 48) & 0xFF);
	dev_type = (int)((devid >> 56) & 0xFF);

	dateAdd(2000, 1, 1, days, year, month, day);


	sprintf(buf, "%03d-%03d-%05d-%04d%02d%02d-%05d",
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
	unsigned int year, month, day;
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

	int days = 0;
	dateDec(2000, 1, 1, year, month, day, days);

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
	int year = 0, month = 0, day = 0;
	dateAdd(2000, 1, 1, customerId, year, month, day);

    snprintf(buf, bufSize, "%04d%02d%02d", year, month, day);
    return buf;
}

static uint8_t c2b(char c)
{
    if (c >= '0' && c <= '9')
    {
        return (uint8_t)(c - '0');
    }
    else if (c >= 'a' && c <= 'f')
    {
        return (uint8_t)(0x0a + c - 'a');
    }
    else if (c >= 'A' && c <= 'F')
    {
        return (uint8_t)(0x0a + c - 'A');
    }
    else
    {
        return 0xFF;
    }
}

int hexStringToBytes(const std::string &sValue, uint8_t *buff, int buffSize)
{
    int length = (int)sValue.length() / 2;
    if (buffSize < length)
    {
        return -1;
    }
    const char* p = sValue.c_str();
    for (int i = 0; i < length; i++)
    {
        uint8_t high = c2b(*(p + 2 * i));
        uint8_t low = c2b(*(p + 2 * i + 1));
        if (high > 0x0F || low > 0x0F)
        {
            return -1;
        }
        buff[i] = (high << 4) | low;
    }
    return length;
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

}