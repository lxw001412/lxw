/**
 * @file termMessage.h
 * @brief  星广播平台终端消息实现类
 * @author Bill
 * @version 0.0.1
 * @date 2021-05-28
 */

#pragma once

#include "termMessage.h"
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include <json/json.h>


namespace star_protocol
{


class termMessageImp : public termMessage
{
public:
    termMessageImp();
    virtual ~termMessageImp();
    virtual uint16_t messageId() const;
    virtual uint64_t sn() const;
    virtual bool qos() const;
    virtual uint8_t topic() const;
    virtual uint8_t cmd() const;
    virtual bool isComplete() const;
    virtual int initFromTermPackage(const termPackage* pkg, const char* productId);
    virtual int initFromJson(const Json::Value &jsonRoot, const char* productId, bool withSn, const char* customerNo = NULL);
    virtual int termPackageCount();
    virtual const termPackage* getTermPackage(int index);
    virtual const Json::Value& getJsonMsg();

private:
    int makePackages(bool withSn, const char* customerNo = NULL);
    void clear();

private:
    uint64_t m_sn;
    bool m_qos;
    bool m_ack;
    uint8_t m_topic;
    uint8_t m_cmd;
    uint16_t m_messageId;
    uint8_t m_packageCount;
    uint8_t *m_msgData;
    uint16_t m_msgDataLength;
    std::string m_productId;
    std::map<uint8_t, bool> m_sliceMap;
    Json::Value m_jsonMsgRoot;
    std::vector<termPackage*> m_packages;
    std::mutex m_sliceMapMutex;
};

}