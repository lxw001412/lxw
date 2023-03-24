#include "cmdParser.h"
#include "productModelImp.h"
#include "commandModelImp.h"
#include "protocolUtils.h"
#include "spdlogging.h"
#include "Exception.h"
#if defined(_WIN32) || defined(WIN32)  || defined(_WIN64) || defined(_WINDOWS)
#include <Windows.h>
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif  // WIN32
#include <sstream>
#include <iostream>
#include <iomanip>
#include <exception>

namespace star_protocol
{

static commandPenetrate s_cmdPenetrate;
cmdParserRepo* cmdParserRepo::m_instance = NULL;

cmdParserRepo* cmdParserRepo::instance()
{
    if (NULL == m_instance)
    {
        m_instance = new cmdParserRepo();
    }
    return m_instance;
}

void cmdParserRepo::destory()
{
    delete m_instance;
    m_instance = NULL;
}

cmdParserRepo::cmdParserRepo()
{
    m_cmdPenetrate   = new commandPenetrate();
    m_cmdMap[0x0101] = new syncHeartbeatReq();
    m_cmdMap[0x0102] = new syncHeartbeatResp();
    m_cmdMap[0x0103] = new syncSetData();
    m_cmdMap[0x0104] = new syncGetData();
    m_cmdMap[0x0201] = new multicastTestReq();
    m_cmdMap[0x0202] = new multicastTestResp();
    m_cmdMap[0x0203] = new startStream();
    m_cmdMap[0x0204] = new stopStream();
    m_cmdMap[0x0205] = new streamError();
    m_cmdMap[0x0206] = new resendData();
    m_cmdMap[0x0207] = new setStreamData();
    m_cmdMap[0x0208] = new getStreamData();
    m_cmdMap[0x0209] = new examHeartbeatReq();
    m_cmdMap[0x020A] = new examHeartbeatResp();
    m_cmdMap[0x020B] = new pullStreamHeartReq();
    m_cmdMap[0x0301] = new serverDiscoverReq();
    m_cmdMap[0x0302] = new serverDiscoverResp();
    m_cmdMap[0x0401] = new getTermConfigReq();
    m_cmdMap[0x0402] = new getTermConfigResp();
    m_cmdMap[0x0403] = new setTermConfigReq();
    m_cmdMap[0x0404] = new setTermConfigResp();
    m_cmdMap[0x0501] = new readFirmwareRequest();
    m_cmdMap[0x0502] = new readFirmwareResponse();
    m_cmdMap[0x0503] = new startUpgradeRequest();
    m_cmdMap[0x0504] = new startUpgradeResponse();
    m_cmdMap[0x0505] = new stopUpgradeRequest();
    m_cmdMap[0x0506] = new stopUpgradeResponse();
    m_cmdMap[0x0507] = new queryUpgradeRequest();
    m_cmdMap[0x0508] = new queryUpgradeResponse();
    m_cmdMap[0x0701] = new encoderHeartbeat();
}

cmdParserRepo::~cmdParserRepo()
{
    for (std::map<uint16_t, cmdParser*>::iterator it = m_cmdMap.begin();
        it != m_cmdMap.end(); ++it)
    {
        delete it->second;
    }
    m_cmdMap.clear();

    if (m_cmdPenetrate != NULL)
    {
        delete m_cmdPenetrate;
        m_cmdPenetrate = NULL;
    }
}

const cmdParser* cmdParserRepo::getCmdParser(uint8_t topic, uint8_t cmd)
{
    uint16_t key = topic;
    // 为透传指令时直接使用cmdPenetrate
    if (key < 50)
    {
        key = (key << 8) | cmd;
        std::map<uint16_t, cmdParser*>::iterator it = m_cmdMap.find(key);
        if (it == m_cmdMap.end())
        {
            return NULL;
        }
        return it->second;
    }
    else
    {
        return m_cmdPenetrate;
    }
}


/************************************************************
同步心跳请求
请求：
"msg":{

		"timestamp":nnn,
		"delay":nnn
        "hbinterval":n
        "devtype":n
	}
应答
   无
************************************************************/

int syncHeartbeatReq::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_timestamp = 1;
    static const uint8_t t_delay = 2;
    static const uint8_t t_hbinterval = 3;
    static const uint8_t t_devtype = 4;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    try
    {
        offset += paramJ2B_u32(jsonRoot, "timestamp", t_timestamp, buff + offset, buffSize - offset);
        offset += paramJ2B_u16(jsonRoot, "delay", t_delay, buff + offset, buffSize - offset);
        offset += paramJ2B_u8(jsonRoot, "hbinterval", t_hbinterval, buff + offset, buffSize - offset);
        offset += paramJ2B_u8(jsonRoot, "devtype", t_devtype, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR("syncHeartbeatReq::j2b exception: {}", e.what());
        return -1;
    }

    return offset;
}

int syncHeartbeatReq::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    static const uint8_t t_timestamp = 1;
    static const uint8_t t_delay = 2;
    static const uint8_t t_hbinterval = 3;
    static const uint8_t t_devtype = 4;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    int bytes = 0;
    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -1;
        }
        switch (T)
        {
            case t_timestamp:
                if (L != 4)
                {
                    SPDERROR("Wrong timestamp length: {}", L);
                    return -2;
                }
                jsonRoot["timestamp"] = bytes2u32(data + offset);
                break;
            case t_delay:
                if (L != 2)
                {
                    SPDERROR("Wrong delay length: {}", L);
                    return -2;
                }
                jsonRoot["delay"] = bytes2u16(data + offset);
                break;
            case t_hbinterval:
                if (L != 1)
                {
                    SPDERROR("Wrong hbinterval length: {}", L);
                    return -3;
                }
                jsonRoot["hbinterval"] = *(data + offset);
                break;
            case t_devtype:
                if (L != 1)
                {
                    SPDERROR("Wrong devtype length: {}", L);
                    return -3;
                }
                jsonRoot["devtype"] = *(data + offset);
                break;
            default:
                SPDERROR("Invalid parameter type: {} in heartbeat request", T);
                break;
        }
        offset += L;
    }
    return 0;
}

/************************************************************
同步心跳响应
请求：
"msg":{
		"timestamp":nnn,
		"utc-ms":nnn,
		"modulever":[{
				"module":"xxx",
				"v":nnn,
		}]
	}
应答
   无
************************************************************/

int syncHeartbeatResp::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_timestamp = 1;
    static const uint8_t t_utc_ms = 2;
    static const uint8_t t_modulever = 3;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    int bytes = 0;
    try
    {
        offset += paramJ2B_u32(jsonRoot, "timestamp", t_timestamp, buff + offset, buffSize - offset);
        offset += paramJ2B_u64UTCMS(jsonRoot, "utc-ms", t_utc_ms, buff + offset, buffSize - offset);
        if (jsonRoot.isMember("modulever") && jsonRoot["modulever"].isArray())
        {
            if (buffSize - offset < (int)jsonRoot["modulever"].size() * 6)
            {
                return -1;
            }
            model* M = productModelRepo::instance()->getProductModel(productId);
            if (M == NULL)
            {
                SPDERROR("product model is not exist, product Id: {}", productId);
                return -2;
            }
            for (Json::Value::const_iterator it = jsonRoot["modulever"].begin(); it != jsonRoot["modulever"].end(); ++it)
            {
                if ((*it).isMember("module") && (*it)["module"].isString()
                    && (*it).isMember("v") && (*it)["v"].isUInt())
                {
                    module *md = M->getModule((*it)["module"].asString().c_str());
                    if (md == NULL)
                    {
                        SPDERROR("module is not exist, product Id: {}, module: {}", productId, (*it)["module"].asString());
                        return -3;
                    }
                    offset += u8tobytes(t_modulever, buff + offset, buffSize - offset);
                    setTlvLength(4, buff + offset, buffSize - offset, bytes);
                    offset += bytes;
                    offset += u16tobytes((uint16_t)md->index(), buff + offset, buffSize - offset);
                    offset += u16tobytes((uint16_t)((*it)["v"].asUInt()), buff + offset, buffSize - offset);
                }
            }
        }
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int syncHeartbeatResp::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    static const uint8_t t_timestamp = 1;
    static const uint8_t t_utc_ms = 2;
    static const uint8_t t_modulever = 3;

    int offset = 0;
    int bytes = 0;

    if (isAck)
    {
        return 0;
    }


    model* M = productModelRepo::instance()->getProductModel(productId);
    if (M == NULL)
    {
        SPDERROR("product model is not exist, product Id: {}", productId);
        return -1;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -1;
        }
        switch (T)
        {
            case t_timestamp:
            {
                if (L != 4)
                {
                    SPDERROR("Wrong timestamp length: {}", L);
                    return -2;
                }
                jsonRoot["timestamp"] = bytes2u32(data + offset);
            }
            break;
            case t_utc_ms:
            {
                if (L != 8)
                {
                    SPDERROR("Wrong utc-ms length: {}", L);
                    return -2;
                }
                jsonRoot["utc-ms"] = (uint64_t)bytes2u32(data + offset) * 1000 + (uint64_t)bytes2u32(data + offset + 4);
            }
            break;
            case t_modulever:
            {
                if (L != 4)
                {
                    SPDERROR("Wrong module version length: {}", L);
                    return -2;
                }
                Json::Value jTemp;
                module *md = M->getModule((int)bytes2u16(data + offset));
                if (md == NULL)
                {
                    SPDERROR("module is not exist, product Id: {}, module index: {}", productId, (int)bytes2u16(data + offset));
                    return -3;
                }
                jTemp["module"] = md->name();
                jTemp["v"] = (int)bytes2u16(data + offset + 2);
                jsonRoot["modulever"].append(jTemp);
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in heartbeat response", T);
            break;
        }
        offset += L;
    }
    return 0;
}

/************************************************************
设置数据
请求：
"msg":{
		"moduledata":[{
			"module":"xxx",
			"v":nnn,
			"identifier":value,
			...
		}]
	}
应答
"msg":{}
************************************************************/
int syncSetData::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_moduledata = 1;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    int bytes = 0;
    model* M = productModelRepo::instance()->getProductModel(productId);
    if (M == NULL)
    {
        SPDERROR("product model is not exist, product Id: {}", productId);
        return -1;
    }

    if (!jsonRoot.isMember("moduledata") || !jsonRoot["moduledata"].isArray())
    {
        SPDERROR("Invalid syncSetData json: {}", jsonRoot.toStyledString());
        return -2;
    }
    for (Json::Value::const_iterator it = jsonRoot["moduledata"].begin(); it != jsonRoot["moduledata"].end(); ++it)
    {
        const Json::Value &item = *it;
        if (!item.isMember("module") || !item["module"].isString()
            || !item.isMember("v") || !item["v"].isUInt())
        {
            SPDERROR("Invalid syncSetData json: {}", jsonRoot.toStyledString());
            return -3;
        }
        module *md = M->getModule(item["module"].asString().c_str());
        if (md == NULL)
        {
            SPDERROR("module is not exist in product modle, productId: {}, module: {}", productId, item["module"].asString());
            continue;
        }
        // TLV -> Type
        offset += u8tobytes(t_moduledata, buff + offset, buffSize - offset);
        // 稍后设置 TLV -> Length

        // module index 2 bytes
        u16tobytes((uint16_t)md->index(), buff + offset + 1, buffSize - offset - 1);
        // module version 2 bytes
        u16tobytes((uint16_t)(item["v"].asUInt()), buff + offset + 3, buffSize - offset - 3);
        int moduleDataSize = 0;
        // module data n bytes
        if (md->dataJ2B(item, buff + offset + 5, buffSize - offset - 5, moduleDataSize) != 0)
        {
            SPDERROR("module data to binary failed, productId: {}, module: {}", productId, item["module"].asString());
            offset--;
            continue;
        }
        // module data + module index + module version
        moduleDataSize += 4;
        // TLV -> Length
        if (moduleDataSize > 127)
        {
            memmove(buff + offset + 2, buff + offset + 1, moduleDataSize);
            setTlvLength(moduleDataSize, buff + offset, 2, bytes);
            offset += bytes;
        }
        else
        {
            setTlvLength(moduleDataSize, buff + offset, 1, bytes);
            offset += bytes;
        }
        // TLV -> Value length
        offset += moduleDataSize;
    }
    return offset;
}

int syncSetData::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    static const uint8_t t_moduledata = 1;
    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    int bytes = 0;

    model* M = productModelRepo::instance()->getProductModel(productId);
    if (M == NULL)
    {
        SPDERROR("product model is not exist, product Id: {}", productId);
        return -1;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -2;
        }
        switch (T)
        {
            case t_moduledata:
            {
                if (L < 4)
                {
                    SPDERROR("Wrong syncSetData pacakge");
                    return -3;
                }
                uint16_t index = bytes2u16(data + offset);
                uint16_t v = bytes2u16(data + offset + 2);
                module *md = M->getModule((int)index);
                if (md == NULL)
                {
                    SPDWARN("module is not exist, product Id: {}, module index: {}", productId, (int)index);
                    break;
                }
                Json::Value jTemp;
                md->dataB2J(data + offset + 4, L, jTemp);
                jTemp["module"] = md->name();
                jTemp["v"] = v;
                jsonRoot["moduledata"].append(jTemp);
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in set data request", T);
            break;
        }
        offset += L;
    }
    return 0;
}

/************************************************************
获取数据
请求：
"msg":{
		"moduleid":["xxx"]
	}
应答
"msg":{
		"moduledata":[{
			"module":"xxx",
			"v":nnn,
			"identifier":value,
			...
		}]
	}
************************************************************/
int syncGetData::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    if (isAck)
    {
        return j2bResp(productId, jsonRoot, buff, buffSize);
    }
    else
    {
        return j2bReq(productId, jsonRoot, buff, buffSize);
    }
}

int syncGetData::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    if (isAck)
    {
        return b2jResp(productId, data, dataLength, jsonRoot);
    }
    else
    {
        return b2jReq(productId, data, dataLength, jsonRoot);
    }
}

int syncGetData::j2bReq(const char* productId, const Json::Value &jsonRoot, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_moduleids = 1;

    int offset = 0;
    int bytes = 0;
    model* M = productModelRepo::instance()->getProductModel(productId);
    if (M == NULL)
    {
        SPDERROR("product model is not exist, product Id: {}", productId);
        return -1;
    }

    if (!jsonRoot.isMember("moduleid") || !jsonRoot["moduleid"].isArray())
    {
        SPDERROR("Invalid syncSetData json: {}", jsonRoot.toStyledString());
        return -2;
    }
    for (Json::Value::const_iterator it = jsonRoot["moduleid"].begin(); it != jsonRoot["moduleid"].end(); ++it)
    {
        module *md = M->getModule(it->asString().c_str());
        if (md == NULL)
        {
            SPDERROR("module is not exist in product modle, productId: {}, module: {}", productId, it->asString());
            continue;
        }
        offset += u8tobytes(t_moduleids, buff + offset, buffSize - offset);
        setTlvLength(2, buff + offset, buffSize - offset, bytes);
        offset += bytes;
        offset += u16tobytes((uint16_t)md->index(), buff + offset, buffSize - offset);
    }
    return offset;
}

int syncGetData::j2bResp(const char* productId, const Json::Value &jsonRoot, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_moduledata = 1;

    int offset = 0;
    int bytes = 0;
    model* M = productModelRepo::instance()->getProductModel(productId);
    if (M == NULL)
    {
        SPDERROR("product model is not exist, product Id: {}", productId);
        return -1;
    }
    if (!jsonRoot.isMember("moduledata") || !jsonRoot["moduledata"].isArray())
    {
        SPDERROR("Invalid syncGetData json: {}", jsonRoot.toStyledString());
        return -2;
    }
    for (Json::Value::const_iterator it = jsonRoot["moduledata"].begin(); it != jsonRoot["moduledata"].end(); ++it)
    {
        const Json::Value &item = *it;
        if (!item.isMember("module") || !item["module"].isString()
            || !item.isMember("v") || !item["v"].isUInt())
        {
            SPDERROR("Invalid syncGetData json: {}", jsonRoot.toStyledString());
            return -3;
        }
        module *md = M->getModule(item["module"].asString().c_str());
        if (md == NULL)
        {
            SPDERROR("module is not exist in product modle, productId: {}, module: {}", productId, item["module"].asString());
            continue;
        }
        // TLV -> Type
        offset += u8tobytes(t_moduledata, buff + offset, buffSize - offset);
        // 稍后设置 TLV -> Length

        // module index 2 bytes
        u16tobytes((uint16_t)md->index(), buff + offset + 1, buffSize - offset - 1);
        // module version 2 bytes
        u16tobytes((uint16_t)(item["v"].asUInt()), buff + offset + 3, buffSize - offset - 3);
        int moduleDataSize = 0;
        // module data n bytes
        if (md->dataJ2B(item, buff + offset + 5, buffSize - offset - 5, moduleDataSize) != 0)
        {
            SPDERROR("module data to binary failed, productId: {}, module: {}", productId, item["module"].asString());
            offset--;
            continue;
        }
        // module data + module index + module version
        moduleDataSize += 4;
        // TLV -> Length
        if (moduleDataSize > 127)
        {
            memmove(buff + offset + 2, buff + offset + 1, moduleDataSize);
            setTlvLength(moduleDataSize, buff + offset, 2, bytes);
            offset += bytes;
        }
        else
        {
            setTlvLength(moduleDataSize, buff + offset, 1, bytes);
            offset += bytes;
        }
        // TLV -> Value length
        offset += moduleDataSize;
    }
    return offset;
}

int syncGetData::b2jReq(const char* productId, const uint8_t *data, int dataLength, Json::Value &jsonRoot) const
{
    static const uint8_t t_moduleids = 1;

    int offset = 0;
    int bytes = 0;

    model* M = productModelRepo::instance()->getProductModel(productId);
    if (M == NULL)
    {
        SPDERROR("product model is not exist, product Id: {}", productId);
        return -1;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -2;
        }
        if (t_moduleids == T)
        {
            if (L != 2)
            {
                SPDERROR("Wrong module id length: {}", L);
                return -3;
            }
            module *md = M->getModule((int)bytes2u16(data + offset));
            if (md != NULL)
            {
                jsonRoot["moduleid"].append(md->name());
            }
        }
        offset += L;
    }
    return 0;
}

int syncGetData::b2jResp(const char* productId, const uint8_t *data, int dataLength, Json::Value &jsonRoot) const
{
    static const uint8_t t_moduledata = 1;

    int offset = 0;
    int bytes = 0;

    model* M = productModelRepo::instance()->getProductModel(productId);
    if (M == NULL)
    {
        SPDERROR("product model is not exist, product Id: {}", productId);
        return -1;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -2;
        }
        if (t_moduledata == T)
        {
            if (L < 4)
            {
                SPDERROR("Wrong syncGetData pacakge");
                return -3;
            }
            uint16_t index = bytes2u16(data + offset);
            uint16_t v = bytes2u16(data + offset + 2);
            module *md = M->getModule((int)index);
            if (md == NULL)
            {
                SPDERROR("module is not exist, product Id: {}, module index: {}", productId, (int)index);
                return -4;
            }
            Json::Value jTemp;
            md->dataB2J(data + offset + 4, L, jTemp);
            jTemp["module"] = md->name();
            jTemp["v"] = v;
            jsonRoot["moduledata"].append(jTemp);
        }
        offset += L;
    }
    return 0;
}

/************************************************************
组播测试请求
请求：
"msg":{
		"utc-ms":nnn
	}
应答
   无
************************************************************/
int multicastTestReq::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_utc_ms = 1;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    try
    {
        offset += paramJ2B_u64UTCMS(jsonRoot, "utc-ms", t_utc_ms, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int multicastTestReq::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    static const uint8_t t_utc_ms = 1;
    
    int offset = 0;
    int bytes = 0;

    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -1;
        }
        if (t_utc_ms == T)
        {
            if (L != 8)
            {
                SPDERROR("Wrong utc-ms length: {}", L);
                return -2;
            }
            jsonRoot["utc-ms"] = (uint64_t)bytes2u32(data + offset) * 1000 + (uint64_t)bytes2u32(data + offset + 4);
        }
        offset += L;
    }
    return 0;
}


/************************************************************
组播测试响应
请求：
"msg":{
		"utc-ms":nnn
	}
应答
   无
************************************************************/
int multicastTestResp::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_utc_ms = 1;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    try
    {
        offset += paramJ2B_u64UTCMS(jsonRoot, "utc-ms", t_utc_ms, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int multicastTestResp::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    static const uint8_t t_utc_ms = 1;
    
    int offset = 0;
    int bytes = 0;

    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -1;
        }
        if (t_utc_ms == T)
        {
            if (L != 8)
            {
                SPDERROR("Wrong utc-ms length: {}", L);
                return -2;
            }
            jsonRoot["utc-ms"] = (uint64_t)bytes2u32(data + offset) * 1000 + (uint64_t)bytes2u32(data + offset + 4);
        }
        offset += L;
    }
    return 0;
}

/************************************************************
开始拉流
请求：
"msg":{
		"url":"rtmp://192.168.1.6/app/stream"
	}
应答
"msg":{
		"streamid":nnn,
		"transfermode":2,
		"multicastaddr":"xxx.xxx.xxx.xxx",
		"mutilcastport":3232
	}
************************************************************/
int startStream::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    if (isAck)
    {
        return j2bResp(productId, jsonRoot, buff, buffSize);
    }
    else
    {
        return j2bReq(productId, jsonRoot, buff, buffSize);
    }
}

int startStream::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    if (isAck)
    {
        return b2jResp(productId, data, dataLength, jsonRoot);
    }
    else
    {
        return b2jReq(productId, data, dataLength, jsonRoot);
    }
}

int startStream::j2bReq(const char* productId, const Json::Value &jsonRoot, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_url = 1;
    static const uint8_t t_prestreamid = 2;
    static const int urlMaxLen = 256;
    int offset = 0;
    try
    {
        offset += paramJ2B_string(jsonRoot, "url", t_url, buff + offset, buffSize - offset, urlMaxLen);
        offset += paramJ2B_u32(jsonRoot, "prestreamid", t_prestreamid, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int startStream::j2bResp(const char* productId, const Json::Value &jsonRoot, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_streamid = 1;
    static const uint8_t t_transfermode = 2;
    static const uint8_t t_multicastaddr = 3;
    static const uint8_t t_multicastport = 4;

    int offset = 0;
    try
    {
        offset += paramJ2B_u32(jsonRoot, "streamid", t_streamid, buff + offset, buffSize - offset);
        offset += paramJ2B_u8(jsonRoot, "transfermode", t_transfermode, buff + offset, buffSize - offset);
        offset += paramJ2B_ip(jsonRoot, "multicastaddr", t_multicastaddr, buff + offset, buffSize - offset);
        offset += paramJ2B_u16(jsonRoot, "mutilcastport", t_multicastport, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int startStream::b2jReq(const char* productId, const uint8_t *data, int dataLength, Json::Value &jsonRoot) const
{
    static const uint8_t t_url = 1;
    static const uint8_t t_prestreamid = 2;
    int offset = 0;
    int bytes = 0;

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -1;
        }
        switch (T)
        {
        case t_url:
        {
            jsonRoot["url"] = std::string((const char*)data + offset, L);
        }
        break;
        case t_prestreamid:
        {
            if (L != 4)
            {
                SPDERROR("Wrong streamid length: {} in startStream request", L);
                return -2;
            }
            jsonRoot["prestreamid"] = bytes2u32(data + offset);
        }
        break;
        default:
            SPDERROR("Invalid parameter type: {} in start Stream  request", T);
            break;
        }
        offset += L;
    }

   
    return 0;
}

int startStream::b2jResp(const char* productId, const uint8_t *data, int dataLength, Json::Value &jsonRoot) const
{
    static const uint8_t t_streamid = 1;
    static const uint8_t t_transfermode = 2;
    static const uint8_t t_multicastaddr = 3;
    static const uint8_t t_multicastport = 4;
    
    int offset = 0;
    int bytes = 0;

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -1;
        }
        switch (T)
        {
            case t_streamid:
            {
                if (L != 4)
                {
                    SPDERROR("Wrong streamid length: {}", L);
                    return -2;
                }
                jsonRoot["streamid"] = bytes2u32(data + offset);
            }
            break;
            case t_transfermode:
            {
                if (L != 1)
                {
                    SPDERROR("Wrong transfermode length: {}", L);
                    return -2;
                }
                jsonRoot["transfermode"] = bytes2u8(data + offset);
            }
            break;
            case t_multicastaddr:
            {
                if (L != 4)
                {
                    SPDERROR("Wrong multicastaddr length: {}", L);
                    return -2;
                }
                jsonRoot["multicastaddr"] = ipu322str(bytes2u32(data + offset));
            }
            break;
            case t_multicastport:
            {
                if (L != 2)
                {
                    SPDERROR("Wrong mutilcastport length: {}", L);
                    return -2;
                }
                jsonRoot["mutilcastport"] = bytes2u16(data + offset);
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in start stream response", T);
            break;
        }

        offset += L;
    }
    return 0;
}

/************************************************************
停止拉流
请求：
"msg":{
		"streamid":nnn
	}
应答
"msg":{}
************************************************************/
int stopStream::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_streamid = 1;

    int offset = 0;
    if (isAck)
    {
        return 0;
    }
    try
    {
        offset += paramJ2B_u32(jsonRoot, "streamid", t_streamid, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int stopStream::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    static const uint8_t t_streamid = 1;
    
    int offset = 0;
    int bytes = 0;

    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in stop stream request");
            return -1;
        }
        switch (T)
        {
            case t_streamid:
                jsonRoot["streamid"] = bytes2u32(data + offset);
                break;
            default:
                SPDERROR("Invalid parameter type: {} in stop stream request", T);
                break;
        }
        offset += L;
    }
    return 0;
}

/************************************************************
拉流异常通知
请求：
"msg":{
		"streamid":nnn,
		"status":1,
		"desc":"xxxx"
	}
应答
"msg":{}
************************************************************/
int streamError::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_streamid = 1;
    static const uint8_t t_status = 2;
    static const uint8_t t_desc = 3;
    static const int descMaxLen = 32;

    int offset = 0;
    if (isAck)
    {
        return 0;
    }
    try
    {
        offset += paramJ2B_u32(jsonRoot, "streamid", t_streamid, buff + offset, buffSize - offset);
        offset += paramJ2B_u8(jsonRoot, "status", t_status, buff + offset, buffSize - offset);
        offset += paramJ2B_string(jsonRoot, "desc", t_desc, buff + offset, buffSize - offset, descMaxLen);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int streamError::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    static const uint8_t t_streamid = 1;
    static const uint8_t t_status = 2;
    static const uint8_t t_desc = 3;

    if (isAck)
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in stream error request");
            return -1;
        }
        switch (T)
        {
            case t_streamid:
            {
                if (L != 4)
                {
                    SPDERROR("Wrong streamid length: {} in stream error request", L);
                    return -2;
                }
                jsonRoot["streamid"] = bytes2u32(data + offset);
            }
            break;
            case t_status:
            {
                if (L != 1)
                {
                    SPDERROR("Wrong status length: {} in stream error request", L);
                    return -2;
                }
                jsonRoot["status"] = bytes2u8(data + offset);
            }
            break;
            case t_desc:
            {
                jsonRoot["desc"] = std::string((const char*)data + offset, L);
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in stream error request", T);
                break;
        }
        offset += L;
    }
    return 0;
}

/************************************************************
数据重传
请求：
"msg":{
		"streamid":nnn,
		"sequencenubmer":nnn
	}
应答
  无
************************************************************/
int resendData::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const
{
    static const uint8_t t_streamid = 1;
    static const uint8_t t_sequencenubmer = 2;

    int offset = 0;
    if (isAck)
    {
        return 0;
    }
    try
    {
        offset += paramJ2B_u32(jsonRoot, "streamid", t_streamid, buff + offset, buffSize - offset);
        offset += paramJ2B_u16(jsonRoot, "sequencenubmer", t_sequencenubmer, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int resendData::b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const
{
    static const uint8_t t_streamid = 1;
    static const uint8_t t_sequencenubmer = 2;

    if (isAck)
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in resend data");
            return -1;
        }
        switch (T)
        {
            case t_streamid:
            {
                if (L != 4)
                {
                    SPDERROR("Wrong streamid length: {} in resend data", L);
                    return -2;
                }
                jsonRoot["streamid"] = bytes2u32(data + offset);
            }
            break;
            case t_sequencenubmer:
            {
                if (L != 2)
                {
                    SPDERROR("Wrong sequencenubmer length: {} in resend data", L);
                    return -2;
                }
                jsonRoot["sequencenubmer"] = bytes2u16(data + offset);
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in resend data", T);
                break;
        }
        offset += L;
    }
    return 0;
}

/************************************************************
设置推流参数
请求：
"msg":{
        "streamid":nnn,
        "url":"http:\\aaaaaa"   
    }
应答
  无
************************************************************/
int setStreamData::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_streamid = 1;
    static const uint8_t t_pt = 2;
    static const uint8_t t_url = 3;
    static const int urlMaxLen = 256;
    int offset = 0;
    if (isAck)
    {
        return 0;
    }
    try
    {
        offset += paramJ2B_u32(jsonRoot, "streamid", t_streamid, buff + offset, buffSize - offset);
        offset += paramJ2B_u8(jsonRoot, "pt", t_pt, buff + offset, buffSize - offset);
        offset += paramJ2B_string(jsonRoot, "url", t_url, buff + offset, buffSize - offset, urlMaxLen);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int setStreamData::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot)const
{
    static const uint8_t t_streamid = 1;
    static const uint8_t t_pt = 2;
    static const uint8_t t_url = 3;

    if (isAck)
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in set streamData request");
            return -1;
        }
        switch (T)
        {
        case t_streamid:
        {
            if (L != 4)
            {
                SPDERROR("Wrong streamid length: {} in set streamData  request", L);
                return -2;
            }
            jsonRoot["streamid"] = bytes2u32(data + offset);
        }
        break;
        case t_pt:
        {
            if (L != 1)
            {
                SPDERROR("Wrong streamid length: {} in set streamData  request", L);
                return -2;
            }
            jsonRoot["pt"] = bytes2u8(data + offset);
        }
        break;
        case t_url:
        {

            jsonRoot["url"] = std::string((const char*)data + offset, L);
        }
        break;
        default:
            SPDERROR("Invalid parameter type: {} in set streamData  request", T);
            break;
        }
        offset += L;
    }
    return 0;
}

/************************************************************
获取推流参数
请求：
"msg":{
        "streamid":nnn,
    }
应答
  无
************************************************************/
int getStreamData::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_streamid = 1;

    int offset = 0;
    if (isAck)
    {
        return 0;
    }
    try
    {
        offset += paramJ2B_u32(jsonRoot, "streamid", t_streamid, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;

}

int getStreamData::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot)const
{
    static const uint8_t t_streamid = 1;

    int offset = 0;
    int bytes = 0;

    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in get streamData request");
            return -1;
        }
        switch (T)
        {
        case t_streamid:
            jsonRoot["streamid"] = bytes2u32(data + offset);
            break;
        default:
            SPDERROR("Invalid parameter type: {} in get streamData request", T);
            break;
        }
        offset += L;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/************************************************************
服务发现请求
请求：
"msg":{
        "service":"star_forwdsvr"
    }
应答
  无
************************************************************/
int serverDiscoverReq::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_service = 1;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    try
    {
        offset += paramJ2B_string(jsonRoot, "service", t_service, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR("serverDiscoverReq::j2b exception: {}", e.what());
        return -1;
    }

    return offset;
}

int serverDiscoverReq::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    static const uint8_t t_service = 1;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    int bytes = 0;

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        ++offset;

        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid serverDiscover message length");
            return -1;
        }
        if (t_service == T)
        {
            jsonRoot["service"] = std::string((const char*)data + offset, L);
        }
        offset += L;
    }
    return 0;
}


/************************************************************
服务发现响应
请求：
"msg":{
        "service":"star_forwdsvr"
        "id" : "服务ID"
        "ipv4":"服务ipv4地址" //"192.168.1.1"
        "port":端口号
}
应答
  无
************************************************************/
int serverDiscoverResp::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_service = 1;
    static const uint8_t t_id = 2;
    static const uint8_t t_ipv4 = 3;
    static const uint8_t t_port = 4;
    static const uint8_t t_ipv6 = 5;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    try
    {
        offset += paramJ2B_string(jsonRoot, "service", t_service, buff + offset, buffSize - offset);
        offset += paramJ2B_string(jsonRoot, "id", t_id, buff + offset, buffSize - offset);
        offset += paramJ2B_ip(jsonRoot, "ipv4", t_ipv4, buff + offset, buffSize - offset);
        offset += paramJ2B_u16(jsonRoot, "port", t_port, buff + offset, buffSize - offset);
        if (!jsonRoot.isMember("ipv6")
            || !jsonRoot["ipv6"].isString()
            || !(jsonRoot["ipv6"].asString().size() == 32))
        {
            SPDERROR("Wrong IPv6 ");
            return 0;
        }
        
        offset += paramJ2B_bytes(jsonRoot, "ipv6", t_ipv6, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int serverDiscoverResp::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    static const uint8_t t_service = 1;
    static const uint8_t t_id = 2;
    static const uint8_t t_ipv4 = 3;
    static const uint8_t t_port = 4;
    static const uint8_t t_ipv6 = 5;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    int bytes = 0;
    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length");
            return -1;
        }
        switch (T)
        {
        case t_service:
        {
            jsonRoot["service"] = std::string((const char*)data + offset, L);

        }
        break;
        case t_id:
        {
            jsonRoot["id"] = std::string((const char*)data + offset, L);
        }
        break;
        case t_ipv4:
        {
            if (L != 4)
            {
                SPDERROR("Wrong ip length: {}", L);
                return -2;
            }
            jsonRoot["ipv4"] =ipu322str(bytes2u32(data + offset));
        }
        break;
        case t_port:
        {
            if (L != 2)
            {
                SPDERROR("Wrong port length: {}", L);
                return -2;
            }
            jsonRoot["port"] = bytes2u16(data + offset);
        }
        break;
        case t_ipv6:
        {
            if (L != 16)
            {
                SPDERROR("Wrong IPv6 length: {}", L);
                return -2;
            }
            std::stringstream res;
            for (int i = 0; i < L; i++)
            {
                res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
            }
            jsonRoot["ipv6"] = res.str();
        }
        break;
        default:
            SPDERROR("Invalid parameter type: {} in server discover", T);
            break;
        }
        offset += L;
    }
    return 0;
}




/************************************************************
读取终端配置请求
请求：
"msg":{
        "type":nn,
        "reqid":nn,
        "getModuleId"; false
        "getModuleData":{
            "moduleName1":[
                "propertyName1",
                "propertyName2"
            ],
            "moduleName2":[
                "propertyName1",
                "propertyName2"
            ]
         }
    }
 "msg":{
        "type":nn,
        "reqid":nn,
        "getModuleId"; true
        "getModuleData":{}
    }
@param getModuleId 为真的时，getModuleData为空。
如果需要读取一个模块的所有属性，则将此模块设为空数组 "moduleName1":[]。

应答
  无
************************************************************/

int getTermConfigReq::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
	static const uint8_t t_type = 1;
    static const uint8_t t_reqid = 2;
    static const uint8_t t_getModuleId = 3;
    static const uint8_t t_getModuleData = 4;

    if (!jsonRoot.isMember("getModuleId") ||!jsonRoot["getModuleId"].isBool()
        || !jsonRoot.isMember("getModuleData"))
    {
        SPDERROR("param lack");
        return -1;
    }

	int offset = 0;
	if (isAck)
	{
		return 0;
	}
    try
    {
        offset += paramJ2B_u8(jsonRoot, "type", t_type, buff + offset, buffSize - offset);
        if (!jsonRoot.isMember("reqid") || !jsonRoot["reqid"].isString())
        {
            SPDERROR("reqid error");
            return -1;
        }
        if (jsonRoot["reqid"].asString().size() % 2 != 0)
        {
            SPDERROR("reqid Len error");
            return -1;
        }
        offset += paramJ2B_bytes(jsonRoot, "reqid", t_reqid, buff + offset, buffSize - offset);

       
        if (jsonRoot["getModuleId"].asBool())
        {
            int bytes = 0;
            offset += u8tobytes(t_getModuleId, buff + offset, buffSize - offset);
            setTlvLength(0, buff + offset, buffSize - offset, bytes);
            offset += bytes;
        }
        else
        {
            if (!jsonRoot["getModuleData"].isObject())
            {
                SPDERROR("param error");
                return -1;
            }

            offset += u8tobytes(t_getModuleData, buff + offset, buffSize - offset);

            Json::Value::Members members = jsonRoot["getModuleData"].getMemberNames();
            //计算L
            int moduleLen=0;
            for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
            {
                if (!jsonRoot["getModuleData"][*it].isArray())
                {
                    SPDERROR("Module param error");
                    return -1;
                }
                //module T.len
                ++moduleLen;

                //module L.len
                int propertysSum = 2*(jsonRoot["getModuleData"][*it].size());
                if (propertysSum > 127)
                {
                    moduleLen += 2;
                }
                else
                {
                    ++moduleLen;
                }

                moduleLen += propertysSum;
            }

            //设置长度
            int bytes = 0;
            setTlvLength(moduleLen, buff + offset, buffSize - offset, bytes);
            offset += bytes;

            model* M = productModelRepo::instance()->getProductModel(productId);
            for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
            {
                module *md = M->getModule((*it).c_str());
                if (NULL == md)
                {
                    SPDERROR("Unknown module {}", *it);
                    return -1;
                }
                offset += u8tobytes(md->index(), buff + offset, buffSize - offset);
                setTlvLength(2 * (jsonRoot["getModuleData"][*it].size()), buff + offset, buffSize - offset, bytes);
                offset += bytes;

                for (unsigned int i = 0; i < jsonRoot["getModuleData"][*it].size(); ++i)
                {
                    if (!jsonRoot["getModuleData"][*it][i].isString())
                    {
                        SPDERROR("property param error");
                        return -1;
                    }
                    std::string propertyName = jsonRoot["getModuleData"][*it][i].asString();
                    int propertyIndex = md->getProperty(propertyName.c_str())->index();

                    offset+= u8tobytes(propertyIndex, buff + offset, buffSize - offset);
                    setTlvLength(0, buff + offset, buffSize - offset, bytes);
                    offset+=bytes;

                }
            }

        }
        

    }
	catch (std::exception& e)
	{
		SPDERROR(e.what());
		return -1;
	}
	return offset;
}


int getTermConfigReq::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
	static const uint8_t t_type = 1;
    static const uint8_t t_reqid = 2; 
    static const uint8_t t_getModuleId = 3;
    static const uint8_t t_getModuleData = 4;

	if (isAck)
	{
		return 0;
	}
	int offset = 0;
	int bytes = 0;
	while (offset < dataLength)
	{
		uint8_t T = bytes2u8(data + offset);
		offset++;
		int L = getTlvLength(data + offset, bytes);
		offset += bytes;

		if (offset + L > dataLength)
		{
			SPDERROR("Invalid message length in getTermData");
			return -1;
		}
		switch (T)
		{
		case t_type:
		{
			if (L != 1)
			{
				SPDERROR("Wrong type length: {} in getTermData ", L);
				return -2;
			}
            
			jsonRoot["type"] = bytes2u8(data + offset);
		}
		break;
		case t_reqid:
		{
			if (L > 16)
			{
				SPDERROR("Wrong reqid length: {} in getTermData ", L);
				return -2;
			}
			std::stringstream res;
			for (int i = 0; i < L; i++)
			{
				res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset+i];
			}
            jsonRoot["reqid"] = res.str();
		}
		break;
        case t_getModuleId:
        {

            jsonRoot["getModuleId"] = true;
            jsonRoot["getModuleData"] = {};
        }
        break;
        case t_getModuleData:
        {
            jsonRoot["getModuleId"] = false;

            //遍历模块
            int moduleOffset = 0;
            model* M = productModelRepo::instance()->getProductModel(productId);
            Json::Value jsonModule;
            while (moduleOffset < L)
            {
                uint8_t moduleT = bytes2u8(data + offset + moduleOffset);
                moduleOffset++;
                int moduleL = getTlvLength(data + offset+ moduleOffset, bytes);
                moduleOffset += bytes;

                std::string moduleName;
                module *md = M->getModule(moduleT);
                if (NULL != md)
                {
                    moduleName = md->name();

                    //遍历property
                    int propertyOffset = 0;
                    while (propertyOffset < moduleL)
                    {
                        uint8_t propertyT = bytes2u8(data + offset + moduleOffset + propertyOffset);
                        propertyOffset++;
                        jsonModule[moduleName].append(md->getProperty(propertyT)->indentifer());
                        //propertyL为零，不取直接加偏移量就好
                        propertyOffset++;

                    }
                }

                moduleOffset += moduleL;
            }
            jsonRoot["getModuleData"] = jsonModule;
        }
        break;
		default:
			SPDERROR("Invalid parameter type: {} in getTermData ", T);
			break;
		}
		offset += L;
	}
    return 0;
}

/************************************************************
读取终端配置响应
请求：
"msg":{
        "reqid":nn,
        "getModuleId":["moduleName1","moduleName2"]
        "getmoduleData":{
            "moduleName1":{
                "propertyName1":"value",
                "propertyName2":"value"
            },
            "moduleName2":{
                "propertyName1":"value",
                "propertyName2":"value"
            }
          }
    }

@param getModuleId与getModuleData不能同时存在
应答
  无
************************************************************/

int getTermConfigResp::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_reqid = 1;
    static const uint8_t t_getModuleId = 3;
    static const uint8_t t_getModuleData = 4;

    //只能存在一个，无论都存在或者都不存在都报错
    if (!((jsonRoot.isMember("getModuleId") && jsonRoot["getModuleId"].isArray()))
        ^ (jsonRoot.isMember("getModuleData")&&jsonRoot["getModuleData"].isObject()))
    {
        SPDERROR("getTermConfigResp param error");
        return -1;
    }

    if (isAck)
    {
        return 0;
    }

	int offset = 0;
	int bytes = 0;
    try
    {
        if (!jsonRoot.isMember("reqid") || !jsonRoot["reqid"].isString())
        {
            SPDERROR("reqid error");
            return -1;
        }
        if (jsonRoot["reqid"].asString().size() % 2 != 0)
        {
            SPDERROR("reqid Len error");
            return -1;
        }

        offset += paramJ2B_bytes(jsonRoot, "reqid", t_reqid, buff + offset, buffSize - offset);

        if (jsonRoot.isMember("getModuleId"))
        {
            offset += u8tobytes(t_getModuleId, buff + offset, buffSize - offset);
            int moduleIdSize = jsonRoot["getModuleId"].size();
            setTlvLength(moduleIdSize*2, buff + offset, buffSize - offset, bytes);
            offset += bytes;

            //设置模块的TLV
            model* M = productModelRepo::instance()->getProductModel(productId);
            for (int i = 0; i < moduleIdSize; ++i)
            {
                if (jsonRoot["getModuleId"][i].isString())
                {
                    module *md = M->getModule(jsonRoot["getModuleId"][i].asString().c_str());
                    if (NULL == md)
                    {
                        SPDERROR("Unknown module: {}", jsonRoot["getModuleId"][i].asString());
                        return -1;
                    }
                    offset += u8tobytes(md->index(), buff + offset, buffSize - offset);
                    setTlvLength(0, buff + offset, buffSize - offset, bytes);
                    offset += bytes;
                }
                else
                {
                    SPDERROR("moduleName param error");
                    return -1;
                }
            }
        }
        else
        {
            offset += u8tobytes(t_getModuleData, buff + offset, buffSize - offset);
            //稍后设置L，暂时为L留一个字节的位置
            int modulesOffset = offset + 1;

            Json::Value::Members members = jsonRoot["getModuleData"].getMemberNames();
            model* M = productModelRepo::instance()->getProductModel(productId);
            for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
            {
                if (jsonRoot["getModuleData"][*it].isObject())
                {
                    module *md = M->getModule((*it).c_str());
                    if (NULL == md)
                    {
                        SPDERROR("Unknown module: {}", *it);
                        return -1;
                    }

                    //设置moduleT
                    modulesOffset += u8tobytes(md->index(), buff + modulesOffset, buffSize - modulesOffset);
                    int propertysDataSize = 0;

                    //稍后设置moduleL， 暂时为moduleL留一个字节的位置
                    int propertysOffset = modulesOffset + 1;
                    if (md->dataJ2B(jsonRoot["getModuleData"][*it], buff + propertysOffset, buffSize - propertysOffset, propertysDataSize) != 0)
                    {
                        SPDERROR("module data to binary failed, productId: {}, module: {}", productId, jsonRoot["getModuleData"][*it].asString());
                        offset--;
                        continue;
                    }

                    if (propertysDataSize > 127)
                    {
                        memmove(buff + propertysOffset+1, buff + propertysOffset, propertysDataSize);
                        setTlvLength(propertysDataSize, buff + modulesOffset, 2, bytes);
                        modulesOffset += bytes;
                    }
                    else
                    {
                        setTlvLength(propertysDataSize, buff + modulesOffset, 1, bytes);
                        modulesOffset += bytes;
                    }
                    modulesOffset += propertysDataSize;
                    
                }

            }
            if (modulesOffset - offset - 1 > 127)
            {
                memmove(buff + offset + 2, buff + offset + 1, modulesOffset - offset - 1);
                setTlvLength(modulesOffset - offset - 1, buff + offset, 2, bytes);
            }
            else
            {
                setTlvLength(modulesOffset - offset - 1, buff + offset, 1, bytes);
            }
            offset = modulesOffset;

        }
        
    }
	catch (std::exception& e)
	{
		SPDERROR(e.what());
		return -1;
	}
	return offset;
}


int getTermConfigResp::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
	static const uint8_t t_reqid = 1;
    static const uint8_t t_getModuleId = 3;
    static const uint8_t t_getModuleData = 4;

	if (isAck)
	{
		return 0;
	}

	int offset = 0;
	int bytes = 0;
    try
    {
        model* M = productModelRepo::instance()->getProductModel(productId);
        if (M == NULL)
        {
            SPDERROR("product model is not exist, product Id: {}", productId);
            return -1;
        }

        while (offset < dataLength)
        {
            uint8_t T = bytes2u8(data + offset);
            offset++;
            int L = getTlvLength(data + offset, bytes);
            offset += bytes;

            if (offset + L > dataLength)
            {
                SPDERROR("Invalid getTermDataResq message length");
                return -2;
            }
            switch (T)
            {
            case t_reqid:
            {
                if (L > 16)
                {
                    SPDERROR("Wrong reqid length: {} in getTermDataResq ", L);
                    return -2;
                }
                std::stringstream res;
                for (int i = 0; i < L; i++)
                {
                    res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
                }
                jsonRoot["reqid"] = res.str();
            }
            break;
            case t_getModuleId:
            {
                if (L % 2 != 0)
                {
                    SPDERROR("Wrong getModuleId length: {} in getTermDataResq ", L);
                    return -2;
                }

                model* M = productModelRepo::instance()->getProductModel(productId);
                for (int i = 0; i < L / 2; ++i)
                {
                    uint8_t moduleT = bytes2u8(data + offset);
                    offset++;

                    module *md = M->getModule(moduleT);
                    if (NULL != md)
                    {
                        jsonRoot["getModuleId"].append(md->name());
                    }
                    else
                    {
                        SPDERROR("Unknown module id: {}", moduleT);
                        // 不退出，尽量解析剩余数据
                    }

                    //由于L为零，不用读取直接跳过
                    offset++;
                }

            }
            break;
            case t_getModuleData:
            {
                model* M = productModelRepo::instance()->getProductModel(productId);
                int moduleOffset = 0;
                Json::Value jsonModules;
                while (moduleOffset < L)
                {
                    uint8_t moduleT = bytes2u8(data + offset + moduleOffset);
                    moduleOffset++;
                    int moduleL = getTlvLength(data + offset + moduleOffset, bytes);
                    moduleOffset += bytes;

                    Json::Value jsonProperties;
                    module *md = M->getModule(moduleT);
                    if (NULL == md)
                    {
                        SPDERROR("Unknown module: {}", moduleT);
                        // 不退出，尽量解析剩余数据
                    }
                    else
                    {
                        if (md->dataB2J(data + offset + moduleOffset, moduleL, jsonProperties) == 0)
                        {
                            jsonModules[md->name()] = jsonProperties;
                        }
                        else
                        {
                            SPDERROR("module data binary to josn failed, module id: {}", moduleT);
                            // 不退出，尽量解析剩余数据
                        }
                    }
                    moduleOffset += moduleL;
                }
                jsonRoot["getModuleData"] = jsonModules;
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in getTermDataResq request", T);
                break;
            }
            offset += L;
        }
    }
	catch (std::exception& e)
	{
		SPDERROR(e.what());
		return -1;
	}
    return 0;
}


/************************************************************
写入终端配置请求
请求：
"msg":{
        "type":nn,
        "reqid":nn,
        "setModuleData":{
            "moduleName1":{
                "propertyName1":"value",
                "propertyName2":"value"
            },
            "moduleName2":{
                "propertyName1":"value",
                "propertyName2":"value"
            }
        }
    }
应答
  无
************************************************************/

int setTermConfigReq::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_type = 1;
	static const uint8_t t_reqid = 2;
    static const uint8_t t_setModuleData = 5;

    if (!(jsonRoot.isMember("setModuleData") && jsonRoot["setModuleData"].isObject()))
    {
        SPDERROR("param error");
        return -1;
    }

	if (isAck)
	{
		return 0;
	}

	int offset = 0;
	int bytes = 0;
	try
	{

        offset += paramJ2B_u8(jsonRoot, "type", t_type, buff + offset, buffSize - offset);
        if (!jsonRoot.isMember("reqid") || !jsonRoot["reqid"].isString())
        {
            SPDERROR("reqid error");
            return -1;
        }
        if (jsonRoot["reqid"].asString().size() % 2 != 0)
        {
            SPDERROR("reqid Len error");
            return -1;
        }
		offset += paramJ2B_bytes(jsonRoot, "reqid", t_reqid, buff + offset, buffSize - offset);


        offset += u8tobytes(t_setModuleData, buff + offset, buffSize - offset);
        //稍后设置L，暂时为L留一个字节的位置
        int modulesOffset = offset + 1;

        Json::Value::Members members = jsonRoot["setModuleData"].getMemberNames();
        model* M = productModelRepo::instance()->getProductModel(productId);
        for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
        {
            if (jsonRoot["setModuleData"][*it].isObject())
            {
                module *md = M->getModule((*it).c_str());
                if (NULL == md)
                {
                    SPDERROR("Unknown module: {}", *it);
                    return -1;
                }

                //设置moduleT
                modulesOffset += u8tobytes(md->index(), buff + modulesOffset, buffSize - modulesOffset);
                int propertysDataSize = 0;

                //稍后设置moduleL， 暂时为moduleL留一个字节的位置
                int propertysOffset = modulesOffset + 1;
                if (md->dataJ2B(jsonRoot["setModuleData"][*it], buff + propertysOffset, buffSize - propertysOffset, propertysDataSize) != 0)
                {
                    SPDERROR("module data to binary failed, productId: {}, module: {}", productId, jsonRoot["setModuleData"][*it].asString());
                    offset--;
                    continue;
                }

                if (propertysDataSize > 127)
                {
                    memmove(buff + propertysOffset + 1, buff + propertysOffset, propertysDataSize);
                    setTlvLength(propertysDataSize, buff + modulesOffset, 2, bytes);
                    modulesOffset += bytes;
                }
                else
                {
                    setTlvLength(propertysDataSize, buff + modulesOffset, 1, bytes);
                    modulesOffset += bytes;
                }
                modulesOffset += propertysDataSize;

            }

        }
        if (modulesOffset - offset - 1 > 127)
        {
            memmove(buff + offset + 2, buff + offset + 1, modulesOffset - offset - 1);
            setTlvLength(modulesOffset - offset - 1, buff + offset, 2, bytes);
			modulesOffset++;
        }
        else
        {
            setTlvLength(modulesOffset - offset - 1, buff + offset, 1, bytes);
        }
        offset = modulesOffset;
		
	}
	catch (std::exception& e)
	{
		SPDERROR(e.what());
		return -1;
	}
	return offset;
}


int setTermConfigReq::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
	static const uint8_t t_type = 1;
    static const uint8_t t_reqid = 2;
    static const uint8_t t_setModuleData = 5;

	if (isAck)
	{
		return 0;
	}

	int offset = 0;
	int bytes = 0;
	try
	{
		model* M = productModelRepo::instance()->getProductModel(productId);
		if (M == NULL)
		{
			SPDERROR("setTermConfigReq product model is not exist, product Id: {}", productId);
			return -1;
		}

		while (offset < dataLength)
		{
			uint8_t T = bytes2u8(data + offset);
			offset++;
			int L = getTlvLength(data + offset, bytes);
			offset += bytes;

			if (offset + L > dataLength)
			{
				SPDERROR("Invalid setTermConfigReq message length");
				return -2;
			}
			switch (T)
			{
            case t_type:
            {
                if (L != 1)
                {
					SPDERROR("Wrong sequencenubmer length: {} in setTermConfigReq data", L);
					return -2;
                }
                jsonRoot["type"] = bytes2u8(data + offset);
            }
            break;
			case t_reqid:
			{
				if (L > 16)
				{
					SPDERROR("Wrong sequencenubmer length: {} in setTermConfigReq data", L);
					return -2;
				}
				std::stringstream res;
				for (int i = 0; i < L; i++)
				{
					res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
				}
				jsonRoot["reqid"] = res.str();
			}
			break;
			case t_setModuleData:
			{
                model* M = productModelRepo::instance()->getProductModel(productId);
                int moduleOffset = 0;
                Json::Value jsonModules;
                while (moduleOffset < L)
                {
                    uint8_t moduleT = bytes2u8(data + offset + moduleOffset);
                    moduleOffset++;
                    int moduleL = getTlvLength(data + offset + moduleOffset, bytes);
                    moduleOffset += bytes;

                    Json::Value jsonProperties;
                    module *md = M->getModule(moduleT);
                    if (NULL == md)
                    {
                        SPDERROR("Unknown module: {}", moduleT);
                        // 不退出，尽量解析剩余数据
                    }
                    else
                    {
                        if (md->dataB2J(data + offset + moduleOffset, moduleL, jsonProperties) == 0)
                        {
                            SPDERROR("module data binary to josn failed, module id: {}", moduleT);
                            // 不退出，尽量解析剩余数据
                        }
                        else
                        {
                            jsonModules[md->name()] = jsonProperties;
                        }
                    }
                    moduleOffset += moduleL;
                }
                jsonRoot["setModuleData"] = jsonModules;

			}
			break;
			default:
				SPDERROR("Invalid parameter type: {} in setTermConfigReq ", T);
				break;
			}
			offset += L;
		}
	}
	catch (std::exception& e)
	{
		SPDERROR(e.what());
		return -1;
	}
    return 0;
}



/************************************************************
写入终端配置响应
请求：
"msg":{
        "reqid":nn,
        "errorcode":"nnn"
        "errormsg:"nnnn"
    }
应答
  无
************************************************************/
int setTermConfigResp::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{

    static const uint8_t t_reqid = 1;
    static const uint8_t t_errorcode = 2;
    static const uint8_t t_errormsg = 3;

    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    try
    {
        if (!jsonRoot.isMember("reqid") || !jsonRoot["reqid"].isString())
        {
            SPDERROR("reqid error");
            return -1;
        }
        if (jsonRoot["reqid"].asString().size() % 2 != 0)
        {
            SPDERROR("reqid Len error");
            return -1;
        }
        offset += paramJ2B_bytes(jsonRoot, "reqid", t_reqid, buff + offset, buffSize - offset);
        offset += paramJ2B_u8(jsonRoot, "errorcode", t_errorcode, buff + offset, buffSize - offset);
        offset += paramJ2B_string(jsonRoot, "errormsg", t_errormsg, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int setTermConfigResp::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
	static const uint8_t t_reqid = 1;
	static const uint8_t t_errorcode = 2;
	static const uint8_t t_errormsg = 3;

	if (isAck)
	{
		return 0;
	}
	int offset = 0;
	int bytes = 0;
	while (offset < dataLength)
	{
		uint8_t T = bytes2u8(data + offset);
		offset++;
		int L = getTlvLength(data + offset, bytes);
		offset += bytes;

		if (offset + L > dataLength)
		{
			SPDERROR("Invalid message length in setTermConfigResp data");
			return -1;
		}
		switch (T)
		{
		case t_reqid:
		{
			if (L > 16)
			{
				SPDERROR("Wrong reqid length: {} in setTermConfigResp data", L);
				return -2;
			}
			std::stringstream res;
			for (int i = 0; i < L; i++)
			{
				res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
			}
			jsonRoot["reqid"] = res.str();
		}
		break;
		case t_errorcode:
		{
			if (L != 1)
			{
				SPDERROR("Wrong errorcode length: {} in setTermConfigResp data", L);
				return -2;
			}
            jsonRoot["errorcode"] = bytes2u8(data+offset);
		}
		break;
		case t_errormsg:
		{
			if (L > 64)
			{
				SPDERROR("Wrong errormsg length: {} in setTermConfigResp data", L);
				return -2;
			}
			jsonRoot["errormsg"]= std::string((const char*)data + offset, L);
		}
		break;
		default:
			SPDERROR("Invalid parameter type: {} in setTermConfigResp data", T);
			break;
		}
		offset += L;
	}
	return 0;
}


/************************************************************
编码心跳


**************************************************************/
int encoderHeartbeat::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    int offset = 0;
    return offset;
}

int encoderHeartbeat::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    return 0;
}


/************************************************************
读取固件请求
请求：
{
}
响应：
无

**************************************************************/
int readFirmwareRequest::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    int offset = 0;
    return offset;
}

int readFirmwareRequest::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    return 0;
}

/************************************************************
读取固件响应
请求：
{
    "firmware" : [
        {
            "hwcode": "123456",
            "version": "1.2.3.4"
        }
    ]
}
响应：
无
**************************************************************/
int readFirmwareResponse::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_firmware = 1;
    static const uint8_t t_hwcode = 1;
    static const uint8_t t_version = 2;
    
    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    int bytes = 0;
    try
    {
        if (!jsonRoot.isMember("firmware") || !jsonRoot["firmware"].isArray())
        {
            SPDERROR("Invalid readFirmwareResponse json: {}", jsonRoot.toStyledString());
            return -2;
        }

        for (Json::Value::const_iterator it = jsonRoot["firmware"].begin(); it != jsonRoot["firmware"].end(); ++it)
        {
            const Json::Value &item = *it;
            if (!item.isMember("hwcode") || !item["hwcode"].isString()
                || !item.isMember("version") || !item["version"].isString())
            {
                SPDERROR("Invalid readFirmwareResponse json: {}", jsonRoot.toStyledString());
                return -3;
            }
            // TLV -> Type
            offset += u8tobytes(t_firmware, buff + offset, buffSize - offset);
            // 稍后设置 TLV -> Length
            int lengthOffset = offset;
            offset++;

            offset += paramJ2B_string(item, "hwcode", t_hwcode, buff + offset, buffSize - offset, 32);
            offset += paramJ2B_ip(item, "version", t_version, buff + offset, buffSize - offset);
            int dataLength = offset - lengthOffset - 1;

            // 设置firmware TLV的长度
            if (dataLength > 127)
            {
                memmove(buff + lengthOffset + 2, buff + lengthOffset + 1, dataLength);
                setTlvLength(dataLength, buff + lengthOffset, 2, bytes);
                offset += 1;
            }
            else
            {
                setTlvLength(dataLength, buff + lengthOffset, 1, bytes);
            }
        }
    }
    catch (std::exception& e)
    {
        SPDERROR("readFirmwareResponse::j2b exception: {}", e.what());
        return -1;
    }

    return offset;
}

int readFirmwareResponse::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    static const uint8_t t_firmware = 1;
    static const uint8_t t_hwcode = 1;
    static const uint8_t t_version = 2;
    int offset = 0;
    int bytes = 0;
    
    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in readFirmwareResponse");
            return -2;
        }

        switch (T)
        {
            case t_firmware:
            {
                if (L < 4)
                {
                    SPDERROR("Wrong firmware data pacakge in readFirmwareResponse");
                    return -3;
                }
                int startOffset = offset;
                int readBytes = 0;
                std::string hwcode;
                std::string version;
                while (readBytes < L)
                {
                    uint8_t type = bytes2u8(data + offset);
                    offset++;
                    readBytes++;
                    int length = getTlvLength(data + offset, bytes);
                    offset += bytes;
                    readBytes += bytes;
                    if (readBytes + length > L)
                    {
                        SPDERROR("Invalid firmware info data in readFirmwareResponse");
                        return -2;
                    }
                    if (t_hwcode == type)
                    {
                        hwcode = std::string((const char*)data + offset, length);
                    }
                    else if (t_version == type)
                    {
                        version = ipu322str(bytes2u32(data + offset));
                    }
                    offset += length;
                    readBytes += length;
                }
                Json::Value jFirmware;
                jFirmware["hwcode"] = hwcode;
                jFirmware["version"] = version;
                jsonRoot["firmware"].append(jFirmware);
                if (startOffset + L != offset)
                {
                    SPDERROR("Wrong firmware data length");
                    return -4;
                }
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in readFirmwareResponse", T);
            break;
        }
    }

    return 0;
}

/************************************************************
开始升级请求
请求：
{
    "taskid": "0123456789ABCDEF0123456789ABCDEF",
    "url": "xxxxx",
    "size": 223,
    "md5": "0123456789ABCDEF0123456789ABCDEF",
    "firmware" : [
        {
            "hwcode": "123456",
            "version": "1.2.3.4"
        }
    ]
}
响应：
无
**************************************************************/
int startUpgradeRequest::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_taskid = 1;
    static const uint8_t t_url = 2;
    static const uint8_t t_size = 3;
    static const uint8_t t_md5 = 4;
    static const uint8_t t_firmware = 5;
    static const uint8_t t_hwcode = 1;
    static const uint8_t t_version = 2;
    
    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    int bytes = 0;

    try
    {
        if (!jsonRoot.isMember("taskid") || !jsonRoot["taskid"].isString()
            || !jsonRoot.isMember("url") || !jsonRoot["url"].isString()
            || !jsonRoot.isMember("size") || !jsonRoot["size"].isUInt()
            || !jsonRoot.isMember("md5") || !jsonRoot["md5"].isString()
            || !jsonRoot.isMember("firmware") || !jsonRoot["firmware"].isArray())
        {
            SPDERROR("Invalid startUpgradeRequest json: {}", jsonRoot.toStyledString());
            return -2;
        }
        if (jsonRoot["taskid"].asString().length() != 32)
        {
            SPDERROR("Invalid startUpgradeRequest taskid length error json: {}", jsonRoot.toStyledString());
            return -2;
        }
        if (jsonRoot["md5"].asString().length() != 32)
        {
            SPDERROR("Invalid startUpgradeRequest md5 length error json: {}", jsonRoot.toStyledString());
            return -2;
        }
        offset += paramJ2B_bytes(jsonRoot, "taskid", t_taskid, buff + offset, buffSize - offset);
        offset += paramJ2B_string(jsonRoot, "url", t_url, buff + offset, buffSize - offset, 256);
        offset += paramJ2B_u32(jsonRoot, "size", t_size, buff + offset, buffSize - offset);
        offset += paramJ2B_bytes(jsonRoot, "md5", t_md5, buff + offset, buffSize - offset);

        for (Json::Value::const_iterator it = jsonRoot["firmware"].begin(); it != jsonRoot["firmware"].end(); ++it)
        {
            const Json::Value &item = *it;
            if (!item.isMember("hwcode") || !item["hwcode"].isString()
                || !item.isMember("version") || !item["version"].isString())
            {
                SPDERROR("Invalid startUpgradeRequest json: {}", jsonRoot.toStyledString());
                return -3;
            }
            // TLV -> Type
            offset += u8tobytes(t_firmware, buff + offset, buffSize - offset);
            // 稍后设置 TLV -> Length
            int lengthOffset = offset;
            offset++;

            offset += paramJ2B_string(item, "hwcode", t_hwcode, buff + offset, buffSize - offset, 32);
            offset += paramJ2B_ip(item, "version", t_version, buff + offset, buffSize - offset);
            int dataLength = offset - lengthOffset - 1;

            // 设置firmware TLV的长度
            if (dataLength > 127)
            {
                memmove(buff + lengthOffset + 2, buff + lengthOffset + 1, dataLength);
                setTlvLength(dataLength, buff + lengthOffset, 2, bytes);
                offset += 1;
            }
            else
            {
                setTlvLength(dataLength, buff + lengthOffset, 1, bytes);
            }
        }
    }
    catch (std::exception& e)
    {
        SPDERROR("startUpgradeRequest::j2b exception: {}", e.what());
        return -1;
    }

    return offset;
}

int startUpgradeRequest::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    static const uint8_t t_taskid = 1;
    static const uint8_t t_url = 2;
    static const uint8_t t_size = 3;
    static const uint8_t t_md5 = 4;
    static const uint8_t t_firmware = 5;
    static const uint8_t t_hwcode = 1;
    static const uint8_t t_version = 2;
    int offset = 0;
    int bytes = 0;
    
    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in startUpgradeRequest");
            return -2;
        }

        switch (T)
        {
            case t_taskid:
            {
				if (L != 16)
				{
					SPDERROR("Wrong taskid length: {} in startUpgradeRequest data", L);
					return -2;
				}
				std::stringstream res;
				for (int i = 0; i < L; i++)
				{
					res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
				}
				jsonRoot["taskid"] = res.str();
                offset += L;
            }
            break;
            case t_url:
            {
                jsonRoot["url"] = std::string((const char*)data + offset, L);
                offset += L;
            }
            break;
            case t_size:
            {
                jsonRoot["size"] = bytes2u32(data + offset);
                offset += L;
            }
            break;
            case t_md5:
            {
				if (L != 16)
				{
					SPDERROR("Wrong md5 length: {} in startUpgradeRequest data", L);
					return -2;
				}
				std::stringstream res;
				for (int i = 0; i < L; i++)
				{
					res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
				}
				jsonRoot["md5"] = res.str();
                offset += L;
            }
            break;
            case t_firmware:
            {
                if (L < 4)
                {
                    SPDERROR("Wrong firmware data pacakge in startUpgradeRequest");
                    return -3;
                }
                int startOffset = offset;
                int readBytes = 0;
                std::string hwcode;
                std::string version;
                while (readBytes < L)
                {
                    uint8_t type = bytes2u8(data + offset);
                    offset++;
                    readBytes++;
                    int length = getTlvLength(data + offset, bytes);
                    offset += bytes;
                    readBytes += bytes;
                    if (readBytes + length > L)
                    {
                        SPDERROR("Invalid firmware info data in startUpgradeRequest");
                        return -2;
                    }
                    if (t_hwcode == type)
                    {
                        hwcode = std::string((const char*)data + offset, length);
                    }
                    else if (t_version == type)
                    {
                        version = ipu322str(bytes2u32(data + offset));
                    }
                    offset += length;
                    readBytes += length;
                }
                Json::Value jFirmware;
                jFirmware["hwcode"] = hwcode;
                jFirmware["version"] = version;
                jsonRoot["firmware"].append(jFirmware);
                if (startOffset + L != offset)
                {
                    SPDERROR("Wrong firmware data length in startUpgradeRequest");
                    return -4;
                }
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in startUpgradeRequest", T);
            break;
        }
    }

    return 0;
}

/************************************************************
开始升级响应
请求：
{
    "taskid": "xxxxx",
    "errorcode": 0,
    "errormsg": "xxxxx"
}
响应：
无
**************************************************************/
int startUpgradeResponse::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_taskid = 1;
    static const uint8_t t_errorcode = 2;
    static const uint8_t t_errormsg = 3;
    int offset = 0;
    
    if (isAck)
    {
        return 0;
    }

    try
    {
        if (!jsonRoot.isMember("taskid") || !jsonRoot["taskid"].isString()
            || !jsonRoot.isMember("errorcode") || !jsonRoot["errorcode"].isUInt()
            || !jsonRoot.isMember("errormsg") || !jsonRoot["errormsg"].isString())
        {
            SPDERROR("Invalid startUpgradeResponse json: {}", jsonRoot.toStyledString());
            return -2;
        }
        if (jsonRoot["taskid"].asString().length() != 32)
        {
            SPDERROR("Invalid startUpgradeResponse taskid length error json: {}", jsonRoot.toStyledString());
            return -2;
        }
        offset += paramJ2B_bytes(jsonRoot, "taskid", t_taskid, buff + offset, buffSize - offset);
        offset += paramJ2B_u8(jsonRoot, "errorcode", t_errorcode, buff + offset, buffSize - offset);
        offset += paramJ2B_string(jsonRoot, "errormsg", t_errormsg, buff + offset, buffSize - offset, 64);
    }
    catch (std::exception& e)
    {
        SPDERROR("startUpgradeResponse::j2b exception: {}", e.what());
        return -1;
    }

    return offset;
}

int startUpgradeResponse::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    static const uint8_t t_taskid = 1;
    static const uint8_t t_errorcode = 2;
    static const uint8_t t_errormsg = 3;
    int offset = 0;
    int bytes = 0;
    
    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in startUpgradeResponse");
            return -2;
        }

        switch (T)
        {
            case t_taskid:
            {
				if (L != 16)
				{
					SPDERROR("Wrong taskid length: {} in startUpgradeResponse data", L);
					return -2;
				}
				std::stringstream res;
				for (int i = 0; i < L; i++)
				{
					res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
				}
				jsonRoot["taskid"] = res.str();
                offset += L;
            }
            break;
            case t_errorcode:
            {
                jsonRoot["errorcode"] = bytes2u8(data + offset);
                offset += L;
            }
            break;
            case t_errormsg:
            {
                jsonRoot["errormsg"] = std::string((const char*)data + offset, L);
                offset += L;
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in startUpgradeResponse", T);
            break;
        }
    }

    return 0;
}

/************************************************************
停止升级请求
请求：
{
    "taskid": "xxxxx"
}
响应：
无
**************************************************************/
int stopUpgradeRequest::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_taskid = 1;
    int offset = 0;
    
    if (isAck)
    {
        return 0;
    }

    try
    {
        if (!jsonRoot.isMember("taskid") || !jsonRoot["taskid"].isString())
        {
            SPDERROR("Invalid stopUpgradeRequest json: {}", jsonRoot.toStyledString());
            return -2;
        }
        if (jsonRoot["taskid"].asString().length() != 32)
        {
            SPDERROR("Invalid stopUpgradeRequest taskid length error json: {}", jsonRoot.toStyledString());
            return -2;
        }
        offset += paramJ2B_bytes(jsonRoot, "taskid", t_taskid, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR("stopUpgradeRequest::j2b exception: {}", e.what());
        return -1;
    }

    return offset;
}

int stopUpgradeRequest::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    static const uint8_t t_taskid = 1;
    int offset = 0;
    int bytes = 0;
    
    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in stopUpgradeRequest");
            return -2;
        }

        switch (T)
        {
            case t_taskid:
            {
				if (L != 16)
				{
					SPDERROR("Wrong taskid length: {} in stopUpgradeRequest data", L);
					return -2;
				}
				std::stringstream res;
				for (int i = 0; i < L; i++)
				{
					res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
				}
				jsonRoot["taskid"] = res.str();
                offset += L;
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in stopUpgradeRequest", T);
            break;
        }
    }

    return 0;
}

/************************************************************
停止升级响应
请求：
{
    "taskid": "xxxxx",
    "errorcode": 0,
    "errormsg": "xxxxx"
}
响应：
无
**************************************************************/
int stopUpgradeResponse::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_taskid = 1;
    static const uint8_t t_errorcode = 2;
    static const uint8_t t_errormsg = 3;
    int offset = 0;
    
    if (isAck)
    {
        return 0;
    }

    try
    {
        if (!jsonRoot.isMember("taskid") || !jsonRoot["taskid"].isString()
            || !jsonRoot.isMember("errorcode") || !jsonRoot["errorcode"].isUInt()
            || !jsonRoot.isMember("errormsg") || !jsonRoot["errormsg"].isString())
        {
            SPDERROR("Invalid stopUpgradeResponse json: {}", jsonRoot.toStyledString());
            return -2;
        }
        if (jsonRoot["taskid"].asString().length() != 32)
        {
            SPDERROR("Invalid stopUpgradeResponse taskid length error json: {}", jsonRoot.toStyledString());
            return -2;
        }
        offset += paramJ2B_bytes(jsonRoot, "taskid", t_taskid, buff + offset, buffSize - offset);
        offset += paramJ2B_u8(jsonRoot, "errorcode", t_errorcode, buff + offset, buffSize - offset);
        offset += paramJ2B_string(jsonRoot, "errormsg", t_errormsg, buff + offset, buffSize - offset, 64);
    }
    catch (std::exception& e)
    {
        SPDERROR("stopUpgradeResponse::j2b exception: {}", e.what());
        return -1;
    }

    return offset;
}

int stopUpgradeResponse::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    static const uint8_t t_taskid = 1;
    static const uint8_t t_errorcode = 2;
    static const uint8_t t_errormsg = 3;
    int offset = 0;
    int bytes = 0;
    
    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in stopUpgradeResponse");
            return -2;
        }

        switch (T)
        {
            case t_taskid:
            {
				if (L != 16)
				{
					SPDERROR("Wrong taskid length: {} in stopUpgradeResponse data", L);
					return -2;
				}
				std::stringstream res;
				for (int i = 0; i < L; i++)
				{
					res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
				}
				jsonRoot["taskid"] = res.str();
                offset += L;
            }
            break;
            case t_errorcode:
            {
                jsonRoot["errorcode"] = bytes2u8(data + offset);
                offset += L;
            }
            break;
            case t_errormsg:
            {
                jsonRoot["errormsg"] = std::string((const char*)data + offset, L);
                offset += L;
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in stopUpgradeResponse", T);
            break;
        }
    }

    return 0;
}

/************************************************************
查询升级请求
请求：
{
}
响应：
无
**************************************************************/
int queryUpgradeRequest::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    int offset = 0;
    return offset;
}

int queryUpgradeRequest::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    return 0;
}

/************************************************************
查询升级响应
请求：
{
    "taskid": "xxxxxxx",
    "progress": 30,
    "stage": "downloading",
    "errorcode": 1,
    "errormdg": "ok"
}
响应：
无
**************************************************************/
int queryUpgradeResponse::j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const
{
    static const uint8_t t_taskid = 1;
    static const uint8_t t_progress = 2;
    static const uint8_t t_stage = 3;
    static const uint8_t t_errorcode = 4;
    static const uint8_t t_errormsg = 5;
    int offset = 0;
    
    if (isAck)
    {
        return 0;
    }

    try
    {
        if (!jsonRoot.isMember("taskid") || !jsonRoot["taskid"].isString()
            || !jsonRoot.isMember("progress") || !jsonRoot["progress"].isUInt()
            || !jsonRoot.isMember("stage") || !jsonRoot["stage"].isString()
            || !jsonRoot.isMember("errorcode") || !jsonRoot["errorcode"].isUInt()
            || !jsonRoot.isMember("errormsg") || !jsonRoot["errormsg"].isString())
        {
            SPDERROR("Invalid queryUpgradeResponse json: {}", jsonRoot.toStyledString());
            return -2;
        }
        if (jsonRoot["taskid"].asString().length() != 32)
        {
            SPDERROR("Invalid queryUpgradeResponse taskid length error json: {}", jsonRoot.toStyledString());
            return -2;
        }
        offset += paramJ2B_bytes(jsonRoot, "taskid", t_taskid, buff + offset, buffSize - offset);
        offset += paramJ2B_u8(jsonRoot, "progress", t_progress, buff + offset, buffSize - offset);
        offset += paramJ2B_string(jsonRoot, "stage", t_stage, buff + offset, buffSize - offset, 32);
        offset += paramJ2B_u8(jsonRoot, "errorcode", t_errorcode, buff + offset, buffSize - offset);
        offset += paramJ2B_string(jsonRoot, "errormsg", t_errormsg, buff + offset, buffSize - offset, 64);
    }
    catch (std::exception& e)
    {
        SPDERROR("queryUpgradeResponse::j2b exception: {}", e.what());
        return -1;
    }

    return offset;
}

int queryUpgradeResponse::b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const
{
    static const uint8_t t_taskid = 1;
    static const uint8_t t_progress = 2;
    static const uint8_t t_stage = 3;
    static const uint8_t t_errorcode = 4;
    static const uint8_t t_errormsg = 5;
    int offset = 0;
    int bytes = 0;
    
    if (isAck)
    {
        return 0;
    }

    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in queryUpgradeResponse");
            return -2;
        }

        switch (T)
        {
            case t_taskid:
            {
				if (L != 16)
				{
					SPDERROR("Wrong taskid length: {} in queryUpgradeResponse data", L);
					return -2;
				}
				std::stringstream res;
				for (int i = 0; i < L; i++)
				{
					res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[offset + i];
				}
				jsonRoot["taskid"] = res.str();
                offset += L;
            }
            break;
            case t_progress:
            {
                jsonRoot["progress"] = bytes2u8(data + offset);
                offset += L;
            }
            break;
            case t_stage:
            {
                jsonRoot["stage"] = std::string((const char*)data + offset, L);
                offset += L;
            }
            break;
            case t_errorcode:
            {
                jsonRoot["errorcode"] = bytes2u8(data + offset);
                offset += L;
            }
            break;
            case t_errormsg:
            {
                jsonRoot["errormsg"] = std::string((const char*)data + offset, L);
                offset += L;
            }
            break;
            default:
                SPDERROR("Invalid parameter type: {} in queryUpgradeResponse", T);
            break;
        }
    }

    return 0;
}

/************************************************************
考试模式心跳请求
请求：
{
}
响应：
无
**************************************************************/
int examHeartbeatReq::j2b(const char * productId, const Json::Value & jsonRoot, bool isAck, uint8_t * buff, int buffSize) const
{
    int offset = 0;
    return offset;
}

int examHeartbeatReq::b2j(const char * productId, const uint8_t * data, int dataLength, bool isAck, Json::Value & jsonRoot) const
{
    return 0;
}

/************************************************************
考试模式心跳响应
请求：
{
}
响应：
无
**************************************************************/
int examHeartbeatResp::j2b(const char * productId, const Json::Value & jsonRoot, bool isAck, uint8_t * buff, int buffSize) const
{
    int offset = 0;
    return offset;
}

int examHeartbeatResp::b2j(const char * productId, const uint8_t * data, int dataLength, bool isAck, Json::Value & jsonRoot) const
{
    return 0;
}


/************************************************************
拉流心跳请求
**************************************************************/
int pullStreamHeartReq::j2b(const char * productId, const Json::Value & jsonRoot, bool isAck, uint8_t * buff, int buffSize) const
{
    static const uint8_t t_url = 1;
    static const uint8_t t_streamDataTime = 2;
    static const int urlMaxLen = 256;
    int offset = 0;
    if (isAck)
    {
        return 0;
    }
    try
    {
        offset += paramJ2B_string(jsonRoot, "url", t_url, buff + offset, buffSize - offset, urlMaxLen);
        offset += paramJ2B_u32(jsonRoot, "streamDataTime", t_streamDataTime, buff + offset, buffSize - offset);
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }
    return offset;
}

int pullStreamHeartReq::b2j(const char * productId, const uint8_t * data, int dataLength, bool isAck, Json::Value & jsonRoot) const
{
    static const uint8_t t_url = 1;
    static const uint8_t t_streamDataTime = 2;

    if (isAck)
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    while (offset < dataLength)
    {
        uint8_t T = bytes2u8(data + offset);
        offset++;
        int L = getTlvLength(data + offset, bytes);
        offset += bytes;

        if (offset + L > dataLength)
        {
            SPDERROR("Invalid message length in pullStream heart request");
            return -1;
        }
        switch (T)
        {
        case t_url:
        {
            jsonRoot["url"] = std::string((const char*)data + offset, L);
        }
        break;
        case t_streamDataTime:
        {
            if (L != 4)
            {
                SPDERROR("Wrong streamid length: {} in pullStream heart  request", L);
                return -2;
            }
            jsonRoot["streamDataTime"] = bytes2u32(data + offset);
        }
        break;
        default:
            SPDERROR("Invalid parameter type: {} in pullStream heart request", T);
            break;
        }
        offset += L;
    }
    return 0;
}
/************************************************************
指令透传

************************************************************/

int commandPenetrate::j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const {
    return -1;
}

int commandPenetrate::j2b(const char * productId, const Json::Value & jsonRoot, bool isAck, uint8_t * buff, int buffSize, int topic, int cmd) const
{
    commandModel *M = commandModelRepo::instance()->getCommandModel(topic, cmd);
    if (M == NULL)
    {
        SPDERROR("command model is not exist, topic: {}");
        return -1;
    }
    
    int offset = 0;
    
    int propertysDataSize = 0;
    if (M->dataJ2B(jsonRoot, buff + offset, buffSize - offset, propertysDataSize) != 0)
    {
        SPDERROR("command message data to binary failed");
        offset--;
    }

    offset += propertysDataSize;

    return offset;

}

int commandPenetrate::b2j(const char * productId, const uint8_t * data, int dataLength, bool isAck, Json::Value & jsonRoot) const
{
    return -1;
}

int commandPenetrate::b2j(const char * productId, const uint8_t * data, int dataLength, bool isAck, Json::Value & jsonRoot, int topic, int cmd) const
{
    if (isAck)
    {
        return 0;
    }

    int offset = 0;
    int bytes = 0;

    try
    {
        if (offset < dataLength)
        {
            commandModel* M = commandModelRepo::instance()->getCommandModel(topic, cmd);
            if (M == NULL)
            {
                SPDERROR("command topic:{} cmd:{} is not exist", topic, cmd);
                return -1;
            }

            if (M->dataB2J(data + offset, dataLength, jsonRoot) != 0)
            {
                SPDERROR("command message data to binary failed");
                return -1;
            }
        }
    }
    catch (std::exception& e)
    {
        SPDERROR(e.what());
        return -1;
    }

    return 0;
}


/************************************************************
J2B数据转化模块




************************************************************/

int cmdParser::paramJ2B_u8(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const
{
    if (!jsonRoot.isMember(subKey) || !jsonRoot[subKey].isUInt())
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    if (buffSize < 3)
    {
        throw Exception("buff size is too small");
    }
    offset += u8tobytes(t, buff + offset, buffSize - offset);
    setTlvLength(1, buff + offset, buffSize - offset, bytes);
    offset += bytes;
    offset += u8tobytes((uint8_t)jsonRoot[subKey].asUInt(), buff + offset, buffSize - offset);
    return offset;
}

int cmdParser::paramJ2B_u16(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const
{
    if (!jsonRoot.isMember(subKey) || !jsonRoot[subKey].isUInt())
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    if (buffSize < 4)
    {
        throw Exception("buff size is too small");
    }
    offset += u8tobytes(t, buff + offset, buffSize - offset);
    setTlvLength(2, buff + offset, buffSize - offset, bytes);
    offset += bytes;
    offset += u16tobytes((uint16_t)jsonRoot[subKey].asUInt(), buff + offset, buffSize - offset);
    return offset;
}

int cmdParser::paramJ2B_u32(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const
{
    if (!jsonRoot.isMember(subKey) || !jsonRoot[subKey].isUInt())
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    if (buffSize < 6)
    {
        throw Exception("buff size is too small");
    }
    offset += u8tobytes(t, buff + offset, buffSize - offset);
    setTlvLength(4, buff + offset, buffSize - offset, bytes);
    offset += bytes;
    offset += u32tobytes(jsonRoot[subKey].asUInt(), buff + offset, buffSize - offset);
    return offset;
}

int cmdParser::paramJ2B_u64(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const
{
    if (!jsonRoot.isMember(subKey) || !jsonRoot[subKey].isUInt64())
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    if (buffSize < 10)
    {
        throw Exception("buff size is too small");
    }
    offset += u8tobytes(t, buff + offset, buffSize - offset);
    setTlvLength(8, buff + offset, buffSize - offset, bytes);
    offset += bytes;
    offset += u64tobytes(jsonRoot[subKey].asUInt64(), buff + offset, buffSize - offset);
    return offset;
}

int cmdParser::paramJ2B_u64UTCMS(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const
{
    if (!jsonRoot.isMember(subKey) || !jsonRoot[subKey].isUInt64())
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    if (buffSize < 10)
    {
        throw Exception("buff size is too small");
    }
    offset += u8tobytes(t, buff + offset, buffSize - offset);
    setTlvLength(8, buff + offset, buffSize - offset, bytes);
    offset += bytes;
    uint64_t utcms = jsonRoot[subKey].asUInt64();
    // seconds
    offset += u32tobytes((uint32_t)(utcms / 1000), buff + offset, buffSize - offset);
    // milliseconds
    offset += u32tobytes((uint32_t)(utcms % 1000), buff + offset, buffSize - offset);
    return offset;
}

int cmdParser::paramJ2B_string(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize, int maxLen) const
{
    if (!jsonRoot.isMember(subKey) || !jsonRoot[subKey].isString())
    {
        return 0;
    }
    std::string value = jsonRoot[subKey].asString();
    if (maxLen > 0 && (int)value.length() > maxLen)
    {
        value = value.substr(0, maxLen);
    }
    int offset = 0;
    int bytes = 0;
    int L = (int)value.length();
    if (buffSize < (L > 127 ? 2 + L : 3 + L))
    {
        throw Exception("buff size is too small");
    }
    offset += u8tobytes(t, buff + offset, buffSize - offset);
    setTlvLength(L, buff + offset, buffSize - offset, bytes);
    offset += bytes;
    memcpy(buff + offset, value.c_str(), L);
    offset += L;
    return offset;
}

int cmdParser::paramJ2B_ip(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const
{
    if (!jsonRoot.isMember(subKey) || !jsonRoot[subKey].isString())
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    if (buffSize < 6)
    {
        throw Exception("buff size is too small");
    }
    offset += u8tobytes(t, buff + offset, buffSize - offset);
    setTlvLength(4, buff + offset, buffSize - offset, bytes);
    offset += bytes;
    offset += u32tobytes(ipstr2u32(jsonRoot[subKey].asString().c_str()), buff + offset, buffSize - offset);
    return offset;
}

int cmdParser::paramJ2B_bytes(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const
{
    if (!jsonRoot.isMember(subKey) || !jsonRoot[subKey].isString())
    {
        return 0;
    }
    int offset = 0;
    int bytes = 0;
    offset += u8tobytes(t, buff + offset, buffSize - offset);

    std::string value = jsonRoot[subKey].asString();
    int length = (int)value.length() / 2;
    setTlvLength(length, buff + offset, buffSize - offset, bytes);
    offset += bytes;

    if (buffSize < offset + length)
    {
        throw Exception("buff size is too small");
    }

    int rc = hexStringToBytes(value, buff + offset, buffSize - offset);
    if (rc < 0)
    {
        throw Exception("hex string to bytes failed");
    }
    offset += rc;
    return offset;
}

}