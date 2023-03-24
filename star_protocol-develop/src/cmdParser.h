/**
 * @file cmdParser.h
 * @brief  星广播平台终端协议解析
 * @author Bill
 * @version 0.0.1
 * @date 2021-06-01
 */

#pragma once
#include <stdint.h>
#include <json/json.h>
#include <vector>
#include <map>
#include <string>
#include <mutex>

namespace star_protocol
{

class cmdParser;

class cmdParserRepo
{
public:
    static cmdParserRepo* instance();
    static void destory();

    ~cmdParserRepo();

    const cmdParser* getCmdParser(uint8_t topic, uint8_t cmd);

protected:
    cmdParserRepo();

    std::map<uint16_t, cmdParser*> m_cmdMap;

    cmdParser *m_cmdPenetrate; // 指令透传

    static cmdParserRepo* m_instance;
};

class cmdParser
{
public:
    virtual ~cmdParser(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const = 0;
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize, int topic, int cmd) const {
        return j2b(productId, jsonRoot, isAck, buff, buffSize);
    }
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const = 0;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot, int topic, int cmd) const {
        return b2j(productId, data, dataLength, isAck, jsonRoot);
    }
    virtual bool isPenetrate() const {
        return false;
    }

protected:
    inline int paramJ2B_u8(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const;
    inline int paramJ2B_u16(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const;
    inline int paramJ2B_u32(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const;
    inline int paramJ2B_u64(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const;
    inline int paramJ2B_u64UTCMS(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const;
    inline int paramJ2B_string(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize, int maxLen=0) const;
    inline int paramJ2B_ip(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const;
    inline int paramJ2B_bytes(const Json::Value &jsonRoot, const char* subKey, uint8_t t, uint8_t *buff, int buffSize) const;
};

class syncHeartbeatReq : public cmdParser
{
public:
    virtual ~syncHeartbeatReq(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

class syncHeartbeatResp : public cmdParser
{
public:
    virtual ~syncHeartbeatResp(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

class syncSetData : public cmdParser
{
public:
    virtual ~syncSetData(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

class syncGetData : public cmdParser
{
public:
    virtual ~syncGetData(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;

private:
    int j2bReq(const char* productId, const Json::Value &jsonRoot, uint8_t *buff, int buffSize) const;
    int j2bResp(const char* productId, const Json::Value &jsonRoot, uint8_t *buff, int buffSize) const;
    int b2jReq(const char* productId, const uint8_t *data, int dataLength, Json::Value &jsonRoot) const;
    int b2jResp(const char* productId, const uint8_t *data, int dataLength, Json::Value &jsonRoot) const;
};

class multicastTestReq : public cmdParser
{
public:
    virtual ~multicastTestReq(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

class multicastTestResp : public cmdParser
{
public:
    virtual ~multicastTestResp(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

class startStream : public cmdParser
{
public:
    virtual ~startStream(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;

private:
    int j2bReq(const char* productId, const Json::Value &jsonRoot, uint8_t *buff, int buffSize) const;
    int j2bResp(const char* productId, const Json::Value &jsonRoot, uint8_t *buff, int buffSize) const;
    int b2jReq(const char* productId, const uint8_t *data, int dataLength, Json::Value &jsonRoot) const;
    int b2jResp(const char* productId, const uint8_t *data, int dataLength, Json::Value &jsonRoot) const;
};

class stopStream : public cmdParser
{
public:
    virtual ~stopStream(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

class streamError : public cmdParser
{
public:
    virtual ~streamError(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

class resendData : public cmdParser
{
public:
    virtual ~resendData(){};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

class setStreamData : public cmdParser
{
public:
    virtual ~setStreamData() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

class getStreamData : public cmdParser
{
public:
    virtual ~getStreamData() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

class getTermConfigReq : public cmdParser
{
public:
	virtual ~getTermConfigReq() {};
	virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
	virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

class getTermConfigResp :public cmdParser
{
public:
    ~getTermConfigResp() {};
	virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
	virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;

};

class serverDiscoverReq : public cmdParser
{
public:
    virtual ~serverDiscoverReq() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

class serverDiscoverResp : public cmdParser
{
public:
    virtual ~serverDiscoverResp() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

class setTermConfigReq : public cmdParser
{
public:
	virtual ~setTermConfigReq() {};
	virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
	virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

class setTermConfigResp :public cmdParser
{
public:
	virtual ~setTermConfigResp() {};
	virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
	virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;

};

class encoderHeartbeat :public cmdParser
{
public:
    virtual ~encoderHeartbeat() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

// 读取固件请求
class readFirmwareRequest :public cmdParser
{
public:
    virtual ~readFirmwareRequest() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

// 读取固件响应
class readFirmwareResponse :public cmdParser
{
public:
    virtual ~readFirmwareResponse() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

// 开始升级请求
class startUpgradeRequest :public cmdParser
{
public:
    virtual ~startUpgradeRequest() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

// 开始升级响应
class startUpgradeResponse :public cmdParser
{
public:
    virtual ~startUpgradeResponse() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

// 停止升级请求
class stopUpgradeRequest :public cmdParser
{
public:
    virtual ~stopUpgradeRequest() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

// 停止升级响应
class stopUpgradeResponse :public cmdParser
{
public:
    virtual ~stopUpgradeResponse() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

// 查询升级请求
class queryUpgradeRequest :public cmdParser
{
public:
    virtual ~queryUpgradeRequest() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

// 查询升级响应
class queryUpgradeResponse :public cmdParser
{
public:
    virtual ~queryUpgradeResponse() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

// 考试模式心跳请求
class examHeartbeatReq : public cmdParser
{
public:
    virtual ~examHeartbeatReq() {};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

// 考试模式心跳响应
class examHeartbeatResp : public cmdParser
{
public:
    virtual ~examHeartbeatResp() {};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
};

// 指令透传
class commandPenetrate : public cmdParser
{
public:
    virtual ~commandPenetrate() {};
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize) const; 
    virtual int j2b(const char* productId, const Json::Value &jsonRoot, bool isAck, uint8_t *buff, int buffSize, int topic, int cmd) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot) const;
    virtual int b2j(const char* productId, const uint8_t *data, int dataLength, bool isAck, Json::Value &jsonRoot, int topic, int cmd) const;
    virtual bool isPenetrate() const {
        return true;
    }
};

// 拉流心跳
class pullStreamHeartReq : public cmdParser
{
public:
    virtual ~pullStreamHeartReq() {};
    virtual int j2b(const char* productId, const Json::Value& jsonRoot, bool isAck, uint8_t* buff, int buffSize) const;
    virtual int b2j(const char* productId, const uint8_t* data, int dataLength, bool isAck, Json::Value& jsonRoot) const;
};

};

