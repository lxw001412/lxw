#include "property.h"
#include "protocolUtils.h"
#include "spdlogging.h"
#include <string.h>
#include <sstream>
#include <iomanip>
#include <stdint.h>
#if defined(_WIN32) || defined(WIN32)  || defined(_WIN64) || defined(_WINDOWS)
#include <Windows.h>
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif  // WIN32

namespace star_protocol
{

property::property() : m_index(0), m_required(false), m_dataType(NULL)
{

}

property::~property()
{
    if (m_dataType != NULL)
    {
        delete m_dataType;
        m_dataType = NULL;
    }
}

int property::init(const Json::Value &jsonRoot)
{
    int ret = 0;
    if (!jsonRoot.isObject()
        || !jsonRoot.isMember("identifier") || !jsonRoot["identifier"].isString()
        || !jsonRoot.isMember("index") || !jsonRoot["index"].isInt()
        || !jsonRoot.isMember("dataType") || !jsonRoot["dataType"].isObject()
        || !jsonRoot.isMember("name") || !jsonRoot["name"].isString())
    {
        return -1;
    }
    m_indentifer = jsonRoot["identifier"].asString();
    m_index = jsonRoot["index"].asInt();
    m_name = jsonRoot["name"].asString();
    m_required = jsonRoot["required"].asBool();
    m_dataType = dataType::makeDataType(jsonRoot["dataType"]);
    if (NULL == m_dataType)
    {
        return -2;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////
// dataType
///////////////////////////////////////////////////////////////////////
dataType* dataType::makeDataType(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isMember("type") || !jsonRoot["type"].isString()
        || !jsonRoot.isMember("specs"))
    {
        return NULL;
    }
    std::string type = jsonRoot["type"].asString();
    dataType* dt = NULL;
    int initRet = 0;

    if (type == "string")
    {
        dt = new dataTypeString();
        initRet = dt->init(jsonRoot["specs"]);
    }
    else if (type == "int8")
    {
        dt = new dataTypeNumber(pt_int8);
        initRet = dt->init(jsonRoot["specs"]);
    }
    else if (type == "int16")
    {
        dt = new dataTypeNumber(pt_int16);
        initRet = dt->init(jsonRoot["specs"]);
    }
    else if (type == "int32")
    {
        dt = new dataTypeNumber(pt_int32);
        initRet = dt->init(jsonRoot["specs"]);
    }
    else if (type == "int64")
    {
        dt = new dataTypeNumber(pt_int64);
        initRet = dt->init(jsonRoot["specs"]);
    }
    else if (type == "bool")
    {
        dt = new dataTypeBool();
        initRet = dt->init(jsonRoot["specs"]);
    }
    else if (type == "enum")
    {
        dt = new dataTypeEnum();
        initRet = dt->init(jsonRoot["specs"]);
    }
    else if (type == "struct")
    {
        dt = new dataTypeStruct();
        initRet = dt->init(jsonRoot["specs"]);
    }
    else if (type == "array")
    {
        dt = new dataTypeArray();
        initRet = dt->init(jsonRoot["specs"]);
    }
    else if (type == "bytes")
    {
        dt = new dataTypeBytes();
        initRet = dt->init(jsonRoot["specs"]);
    }
    if (initRet != 0)
    {
        delete dt;
        return NULL;
    }

    return dt;
}

///////////////////////////////////////////////////////////////////////
// dataTypeNumber
///////////////////////////////////////////////////////////////////////
dataTypeNumber::dataTypeNumber(propertyType type)
{
    m_type = type;
    switch (m_type)
    {
    case pt_int8:
        m_length = 1;
        break;
    case pt_int16:
        m_length = 2;
        break;
    case pt_int32:
        m_length = 4;
        break;
    case pt_int64:
        m_length = 8;
        break;
    default:
        m_length = 0;
    }
}

dataTypeNumber::~dataTypeNumber()
{

}

int dataTypeNumber::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject()
        || !jsonRoot.isMember("unit") || !jsonRoot["unit"].isString()
        || !jsonRoot.isMember("min") || !jsonRoot["min"].isUInt64()
        || !jsonRoot.isMember("max") || !jsonRoot["max"].isUInt64()
        || !jsonRoot.isMember("step") || !jsonRoot["step"].isUInt64())
    {
        return -1;
    }
    m_unit = jsonRoot["unit"].asString();
    m_min = jsonRoot["min"].asUInt64();
    m_max = jsonRoot["max"].asUInt64();
    m_step = jsonRoot["step"].asUInt64();
    return 0;
}

bool dataTypeNumber::validate(const char* value) const
{
    uint64_t v = (uint64_t)std::atoll(value);
    return v >= m_min && v <= m_max;
}

int dataTypeNumber::b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const
{
    uint64_t value = 0;
    switch (m_type)
    {
    case pt_int8:
        if (length != 1)
        {
            return -1;
        }
        value = (uint64_t)*data;
        break;
    case pt_int16:
        if (length != 2)
        {
            return -1;
        }
        value = (uint64_t)ntohs(*(uint16_t*)data);
        break;
    case pt_int32:
        if (length != 4)
        {
            return -1;
        }
        value = (uint64_t)ntohl(*(uint32_t*)data);
        break;
    case pt_int64:
        if (length != 8)
        {
            return -1;
        }
        value = (uint64_t)ntohl(*(uint32_t*)data);
        value = (value << 32) | (uint64_t)ntohl(*(uint32_t*)(data + 4));
        break;
    default:
        return -1;
    }
    jsonRoot[key] = value;
    return 0;
}

int dataTypeNumber::j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const
{
    if (!jsonRoot.isUInt64())
    {
        return -1;
    }
    uint64_t value = jsonRoot.asUInt64();
    int vBytes = 0;
    switch (m_type)
    {
    case pt_int8:
    {
        vBytes = 1;
        if (bufSize < vBytes + 2)
        {
            return -2;
        }
        *(buf + 2) = (uint8_t)value;
    }
    break;
    case pt_int16:
    {
        vBytes = 2;
        if (bufSize < vBytes + 2)
        {
            return -2;
        }
        uint16_t v = htons((uint16_t)value);
        memcpy(buf + 2, &v, 2);
    }
    break;
    case pt_int32:
    {
        vBytes = 4;
        if (bufSize < vBytes + 2)
        {
            return -2;
        }
        uint32_t v = htonl((uint32_t)value);
        memcpy(buf + 2, &v, 4);
    }
    break;
    case pt_int64:
    {
        vBytes = 8;
        if (bufSize < vBytes + 2)
        {
            return -2;
        }
        uint32_t v = htonl((uint32_t)(value >> 32));
        memcpy(buf + 2, &v, 4);
        v = htonl((uint32_t)value);
        memcpy(buf + 6, &v, 4);
    }
    break;
    default:
        return -3;
    }
    dataLength = 2 + vBytes;
    *buf = (uint8_t)index;
    *(buf + 1) = vBytes;

    return 0;
}

///////////////////////////////////////////////////////////////////////
// dataTypeString
///////////////////////////////////////////////////////////////////////
dataTypeString::dataTypeString() : m_length(0)
{
}

dataTypeString::~dataTypeString()
{

}

int dataTypeString::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject()
        || !jsonRoot.isMember("length") || !jsonRoot["length"].isInt())
    {
        return -1;
    }
    m_length = jsonRoot["length"].asInt();
    return 0;
}

bool dataTypeString::validate(const char* value) const
{
    return true;
}

int dataTypeString::b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const
{
    // trim right zreo
    int len = length;
    while (len > 0 && *(data + len - 1) == 0) len--;
    jsonRoot[key] = std::string((const char*)data, len);
    return 0;
}

int dataTypeString::j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const
{
    if (!jsonRoot.isString())
    {
        return -1;
    }
    int length = (int)jsonRoot.asString().length();
    if (length > m_length)
    {
        length = m_length;
    }
    if (bufSize < length + 3)
    {
        return -2;
    }
    *buf = index;
    int bytes = 0;
    if (0 != setTlvLength(length, buf + 1, bufSize - 1, bytes))
    {
        return -3;
    }
    memcpy(buf + 1 + bytes, jsonRoot.asString().c_str(), length);
    dataLength = 1 + bytes + length;
    return 0;
}

///////////////////////////////////////////////////////////////////////
// dataTypeBytes
///////////////////////////////////////////////////////////////////////
dataTypeBytes::dataTypeBytes() : m_length(0)
{
}

dataTypeBytes::~dataTypeBytes()
{

}

int dataTypeBytes::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject()
        || !jsonRoot.isMember("length") || !jsonRoot["length"].isInt()
        || !jsonRoot.isMember("codec") || !jsonRoot["codec"].isString())
    {
        return -1;
    }
    m_length = jsonRoot["length"].asInt();
    std::string codec = jsonRoot["codec"].asString();
    if (codec == "hexstring")
    {
        m_codec = hexstring;
    }
    else if (codec == "base64")
    {
        m_codec = base64;
        SPDERROR("Unsupport hexstring codec: {}", codec);
        return -1;
    }
    else
    {
        SPDERROR("Unsupport hexstring codec: {}", codec);
        return -1;
    }
    return 0;
}

bool dataTypeBytes::validate(const char* value) const
{
    return true;
}
int dataTypeBytes::b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const
{
    if (m_codec == hexstring)
    {
        return b2j_hexstring(key, data, length, jsonRoot);
    }
    else if (m_codec == base64)
    {
        // unsupport
        return -1;
    }
    else
    {
        // unsupport
        return -1;
    }
}

int dataTypeBytes::j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const
{
    if (m_codec == hexstring)
    {
        return j2b_hexstring(jsonRoot, index, buf, bufSize, dataLength);
    }
    else if (m_codec == base64)
    {
        // unsupport
        return -1;
    }
    else
    {
        // unsupport
        return -1;
    }
}


int dataTypeBytes::b2j_hexstring(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const
{
    std::stringstream res;
    for (int i = 0; i < length; i++)
    {
        res << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)data[i];
    }
    jsonRoot[key] = res.str();
    return 0;
}

int dataTypeBytes::j2b_hexstring(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const
{
    if (!jsonRoot.isString())
    {
        return -1;
    }
    int length = (int)jsonRoot.asString().length();
    if (length / 2 > m_length)
    {
        // 丢弃多出的数据
        length = m_length * 2;
    }
    *buf = index;
    int bytes = 0;
    if (0 != setTlvLength(length / 2, buf + 1, bufSize - 1, bytes))
    {
        return -2;
    }
    if (bufSize < length + 1 + bytes)
    {
        return -3;
    }
    int rc = hexStringToBytes(jsonRoot.asString().substr(0, length), buf + 1 + bytes, bufSize - 1 - bytes);
    if (rc < 0)
    {
        return -4;
    }
    dataLength = 1 + bytes + rc;
    return 0;
}

///////////////////////////////////////////////////////////////////////
// dataTypeBool
///////////////////////////////////////////////////////////////////////
dataTypeBool::dataTypeBool()
{

}

dataTypeBool::~dataTypeBool()
{

}

int dataTypeBool::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject()
        || !jsonRoot.isMember("1") || !jsonRoot["1"].isString()
        || !jsonRoot.isMember("0") || !jsonRoot["0"].isString())
    {
        return -1;
    }
    m_trueDesc = jsonRoot["1"].asString();
    m_falseDesc = jsonRoot["0"].asString();
    return 0;
}

bool dataTypeBool::validate(const char* value) const
{
    return strcmp(value, "true") == 0 || strcmp(value, "false") == 0;
}

std::string dataTypeBool::toString(const char* value) const
{
    if (strcmp(value, "true") == 0)
    {
        return m_trueDesc;
    }
    else if (strcmp(value, "false") == 0)
    {
        return m_falseDesc;
    }
    else
    {
        return "";
    }
}

std::string dataTypeBool::toValue(const char* str) const
{
    if (m_trueDesc == str)
    {
        return "true";
    }
    else if (m_falseDesc == str)
    {
        return "false";
    }
    else
    {
        return "";
    }
}

int dataTypeBool::b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const
{
    if (length != 1)
    {
        return -1;
    }
    jsonRoot[key] = (*data == 1);
    return 0;
}

int dataTypeBool::j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const
{
    if (!jsonRoot.isBool())
    {
        return -1;
    }
    if (bufSize < 3)
    {
        return -2;
    }
    *buf = index;
    *(buf + 1) = 1;
    *(buf + 2) = jsonRoot.asBool() ? 1 : 0;
    dataLength = 3;
    return 0;
}

///////////////////////////////////////////////////////////////////////
// dataTypeEnum
///////////////////////////////////////////////////////////////////////
dataTypeEnum::dataTypeEnum()
{

}

dataTypeEnum::~dataTypeEnum()
{

}

int dataTypeEnum::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject())
    {
        return -1;
    }
    Json::Value::Members members = jsonRoot.getMemberNames();
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
    {
        int key = std::atoi(it->c_str());
        if (!jsonRoot[*it].isString())
        {
            return -2;
        }
        m_valueNameMap[key] = jsonRoot[*it].asString();
    }
    return 0;
}

bool dataTypeEnum::validate(const char* value) const
{
    int key = std::atoi(value);
    return m_valueNameMap.find(key) != m_valueNameMap.end();
}

std::string dataTypeEnum::toString(const char* value) const
{
    int key = std::atoi(value);
    std::map<int, std::string>::const_iterator it = m_valueNameMap.find(key);
    if (it != m_valueNameMap.end())
    {
        return it->second;
    }
    return "";
}

std::string dataTypeEnum::toValue(const char* str) const
{
    for (std::map<int, std::string>::const_iterator it = m_valueNameMap.begin();
        it != m_valueNameMap.end(); ++it)
    {
        if (it->second == str)
        {
            std::stringstream ss;
            ss << it->first;
            return ss.str();
        }
    }
    return "";
}

int dataTypeEnum::b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const
{
    if (length != 1)
    {
        return -1;
    }
    jsonRoot[key] = (int)*data;
    return 0;
}

int dataTypeEnum::j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const
{
    if (!jsonRoot.isUInt())
    {
        return -1;
    }
    if (bufSize < 3)
    {
        return -2;
    }
    *buf = index;
    *(buf + 1) = 1;
    *(buf + 2) = (uint8_t)jsonRoot.asUInt();
    dataLength = 3;
    return 0;
}

///////////////////////////////////////////////////////////////////////
// dataTypeArray
///////////////////////////////////////////////////////////////////////
dataTypeArray::dataTypeArray() : m_size(0), m_property(NULL)
{

}

dataTypeArray::~dataTypeArray()
{
    if (m_property != NULL)
    {
        delete m_property;
        m_property = NULL;
    }
}

int dataTypeArray::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isObject()
        || !jsonRoot.isMember("item") || !jsonRoot["item"].isObject()
        || !jsonRoot.isMember("size") || !jsonRoot["size"].isInt())
    {
        return -1;
    }
    m_size = jsonRoot["size"].asInt();
    m_property = new property();
    return m_property->init(jsonRoot["item"]);
}

bool dataTypeArray::validate(const char* value) const
{
    return true;
}

int dataTypeArray::b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const
{
    Json::Value item;
    if (m_property == NULL)
    {
        return -1;
    }
    if (m_property->getDataType()->b2j("item", data, length, item) != 0)
    {
        return -2;
    }
    jsonRoot[key].append(item["item"]);
    return 0;
}

int dataTypeArray::j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const
{
    if (!jsonRoot.isArray())
    {
        return -1;
    }
    dataLength = 0;
    int length = 0;
    for (Json::Value::const_iterator it = jsonRoot.begin(); it != jsonRoot.end(); ++it)
    {
        if (m_property->getDataType()->j2b(*it, index, buf + dataLength, bufSize - dataLength, length) != 0)
        {
            return -2;
        }
        dataLength += length;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////
// dataTypeStruct
///////////////////////////////////////////////////////////////////////
dataTypeStruct::dataTypeStruct()
{

}

dataTypeStruct::~dataTypeStruct()
{
    m_propertyIndexMap.clear();
    m_propertyNameMap.clear();
    for (std::vector<property*>::iterator it = m_properties.begin(); it != m_properties.end(); ++it)
    {
        delete *it;
    }
    m_properties.clear();
}

int dataTypeStruct::init(const Json::Value &jsonRoot)
{
    if (!jsonRoot.isArray())
    {
        return -1;
    }
    for (unsigned int i = 0; i < jsonRoot.size(); ++i)
    {
        property* p = new property();
        if (p->init(jsonRoot[i]) != 0)
        {
            delete p;
            return -1;
        }
        m_properties.push_back(p);
        m_propertyIndexMap[p->index()] = p;
        m_propertyNameMap[p->indentifer()] = p;
    }
    return 0;
}

const property* dataTypeStruct::getProperty(int index) const
{
    std::map<int, property*>::const_iterator it = m_propertyIndexMap.find(index);
    if (it == m_propertyIndexMap.end())
    {
        return NULL;
    }
    return it->second;
}

const property* dataTypeStruct::getProperty(const char* name) const
{
    std::map<std::string, property*>::const_iterator it = m_propertyNameMap.find(name);
    if (it == m_propertyNameMap.end())
    {
        return NULL;
    }
    return it->second;
}

int dataTypeStruct::b2j(const char* key, const uint8_t *data, int length, Json::Value &jsonRoot) const
{
    int offset = 0;
    Json::Value obj;
    while (offset < length)
    {
        int type = *(data + offset);
        offset++;
        int bytes;
        int tlvLength = getTlvLength(data + offset, bytes);
        offset += bytes;
        const uint8_t* pData = data + offset;
        offset += tlvLength;
        if (offset > length)
        {
            break;
        }
        const property* prop = getProperty(type);
        if (prop == NULL)
        {
            continue;
        }
        prop->getDataType()->b2j(prop->indentifer().c_str(), pData, tlvLength, obj);
    }
    jsonRoot[key] = obj;
    return 0;
}

int dataTypeStruct::j2b(const Json::Value &jsonRoot, uint8_t index, uint8_t *buf, int bufSize, int &dataLength) const
{
    if (!jsonRoot.isObject())
    {
        return -1;
    }
    if (bufSize < 3)
    {
        return -2;
    }
    *buf = index;
    int offset = 2;
    int length = 0;
    int temp = 0;
    Json::Value::Members members = jsonRoot.getMemberNames();
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
    {
        const property* p = getProperty(it->c_str());
        if (NULL == p)
        {
            continue;
        }
        if (0 != p->getDataType()->j2b(jsonRoot[*it],
            p->index(), buf + offset, bufSize - offset, temp))
        {
            return -3;
        }
        offset += temp;
        length += temp;
    }
    if (length <= 0x7F)
    {
        *(buf + 1) = length;
    }
    else
    {
        // 长度大于0x7F，TLV长度部分占用两个字节，将数据部分后移一个字节
        offset++;
        if (bufSize < offset)
        {
            return -4;
        }
        memmove(buf + 3, buf + 2, length);
        int bytes = 0;
        if (0 != setTlvLength(length, buf + 1, 2, bytes) || bytes != 2)
        {
            return -5;
        }
    }
    dataLength = offset;

    return 0;
}
}