/**
 * @file termMessage.h
 * @brief  星广播平台终端消息封装接口类
 * @author Bill
 * @version 0.0.1
 * @date 2021-05-28
 */

#pragma once

#include "termPackage.h"
#include <stdint.h>
#include <json/json.h>

class termMessage
{
public:
	virtual ~termMessage(){};
    /**
     * @brief  makeTermMessage 
     *         构建终端消息对象
     *
     * @return  终端消息对象指针
     */
    static termMessage* makeTermMessage();

    /**
     * @brief  messageId 
     *         消息ID
     *
     * @return  消息ID
     */
    virtual uint16_t messageId() const = 0;
    /**
     * @brief  sn 
     *         终端序列号
     *
     * @return  终端序列号(64位无符号数)
     */
    virtual uint64_t sn() const = 0;
    /**
     * @brief  qos 
     *         qos是否为1
     *
     * @return  qos是否为1
     */
    virtual bool qos() const = 0;
    /**
     * @brief  topic 
     *         主题
     *
     * @return  主题
     */
    virtual uint8_t topic() const = 0;
    /**
     * @brief  cmd 
     *         命令字
     *
     * @return  命令字
     */
    virtual uint8_t cmd() const = 0;
    /**
     * @brief  isComplete 
     *         使用终端报文对象初始化终端消息，是否已构造完整的终端消息
     *
     * @return  true: 完整
     *          false: 未收到完整的消息
     */
    virtual bool isComplete() const = 0;
    /**
     * @brief  initFromTermPackage 
     *         使用终端报文对象初始化终端消息对象
     *         说明：将复制终端报文数据到终端消息对象内部，调用完本接口后终端报文对象可以释放。
     * @param pkg  终端报文对象指针
     * @param productId 产品ID
     *
     * @return  0:  成功
     *          1:  已组成完整的消息
     *          <0: 错误
     */
    virtual int initFromTermPackage(const termPackage* pkg, const char* productId) = 0;
    /**
     * @brief  initFromJson 
     *         使用JSON格式终端消息初始化
     * @param jsonRoot  JSON格式终端消息根节点
     * @param productId 产品ID
     * @param withSn 构造的终端报文中是否包含终端SN（服务器发给终端的报文可以不带SN）
     * @param customerNo 客户号，如果customerNo = NULL，则从终端SN中取客户号。
     *
     * @return  0:  成功
     *          <0: 失败
     */
    virtual int initFromJson(const Json::Value &jsonRoot, const char* productId, bool withSn, const char* customerNo = NULL) = 0;
    /**
     * @brief  termPackageCount 
     *         终端报文个数
     *
     * @return  终端报文个数
     */
    virtual int termPackageCount() = 0;
    /**
     * @brief  getTermPackage 
     *         取终端报文
     *         说明：返回的终端报文对象指针，可用生命周期与终端消息对象生命周期相同。
     *              调用者不能delete终端报文对象指针，由终端消息对象析构时释放。
     * @param index  终端报文序号，从0开始
     * @param productId 产品ID
     *
     * @return  终端报文对象指针
     */
    virtual const termPackage* getTermPackage(int index) = 0;
    /**
     * @brief  getJsonMsg 
     *         取JOSN格式终端消息
     *         说明：返回的JOSN格式终端消息对象指针，可用生命周期与终端消息对象生命周期相同。
     *              调用者不能deleteJOSN格式终端消息对象指针，由终端消息对象析构时释放。
     *
     * @return  JOSN格式终端消息对象指针
     *          NULL: 失败，终端消息不完整
     */
    virtual const Json::Value& getJsonMsg() = 0;
};
