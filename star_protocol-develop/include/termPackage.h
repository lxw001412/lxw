/**
 * @file termPackage.h
 * @brief  星广播平台终端控制报文解析类接口定义
 * @author Bill
 * @version 0.0.1
 * @date 2021-05-28
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>

class termPackage
{
public:
    /**
     * @brief  makeTermPackage 
     *         构建终端报文对象
     *
     * @return  终端报文对象指针
     */
    static termPackage* makeTermPackage();

	virtual ~termPackage(){};
    /**
     * @brief  init 
     *         初始化报文对象； 
     *         说明：不会复制报文数据到对象内部，使用对象成员函数时需保证报文数据缓存未被修改。
     * @param data 报文数据指针
     * @param size 报文长度
     * @param ownData 是否由本对象负责释放data指向的内存
     *
     * @return  0:  成功
     */
    virtual int init(const uint8_t* data, int size, bool ownData = false) = 0;
    
    /**
     * @brief  verify 
     *          校验报文（STAG、版本、签名）
     * @param customerNo 客户号，如果customerNo==NULL则从SN中取客户号
     * 
     * @return ture： 校验通过
     *         false：校验失败
     */
    virtual bool verify(const char* customerNo = NULL) const = 0;

    /**
     * @brief  makeSliceAck 
     *         构建分包确认报文
     * @param buff 报文数据缓存指针，大于等于28字节
     * @param size 缓存大小
     * @param customerNo 客户号
     *
     * @return  > 0:  分包确认报文长度（字节数）
     *          <= 0: 报文数据缓存小于分包确认报文所需字节数
     */
    virtual int makeSliceAck(uint8_t* buff, int size, const char* customerNo) const = 0;
    /**
     * 报文版本号
     */
    virtual int ver() const = 0;
    /**
     * 是否包含终端SN
     */
    virtual bool hasSn() const = 0;
    /**
     * qos 是否为1
     */
    virtual bool qos() const = 0;
    /**
     * 是否为ACK报文
     */
    virtual bool isAck() const = 0;
    /**
     * 是否为分片报文
     */
    virtual bool isSlicePkg() const = 0;
    /**
     * 是否为分片确认报文
     */
    virtual bool isSliceAckPkg() const = 0;
    /**
     * 是否包含扩展头
     */
    virtual bool hasExtData() const = 0;
    /**
     * 主题
     */
    virtual uint8_t topic() const = 0;
    /**
     * 命令字
     */
    virtual uint8_t cmd() const = 0;
    /**
     * 报文长度
     */
    virtual uint16_t length() const = 0;
    /**
     * 消息ID
     */
    virtual uint16_t msgId() const = 0;
    /**
     * 消息长度
     */
    virtual uint16_t msgLength() const = 0;
    /**
     * 终端序列号（64位无符号数）
     */
    virtual uint64_t sn() const = 0;
    /**
     * 报文分片序号
     */
    virtual uint8_t sliceIndex() const = 0;
    /**
     * 报文分片总数
     */
    virtual uint8_t sliceTotal() const = 0;
    /**
     * 报文body部分在终端消息中的偏移
     */
    virtual uint16_t sliceOffset() const = 0;
    /**
     * 扩展数据长度
     */
    virtual uint8_t extDataLength() const = 0;
    /**
     * 扩展数据指针
     */
    virtual const uint8_t* extData() const = 0;
    /**
     * body数据指针
     */
    virtual const uint8_t* body() const = 0;
    /**
     * body长度
     */
    virtual uint16_t bodyLength() const = 0;
    /**
     * 签名时间戳
     */
    virtual uint32_t timestamp() const = 0;
    /**
     * 签名
     */
    virtual uint32_t signature() const = 0;
    /**
     * 报文数据指针
     */
    virtual const uint8_t* data() const = 0;
    /**
     * 报文长度
     */
    virtual int size() const = 0;
};
