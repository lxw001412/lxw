/**
 * @file termPackageImp.h
 * @brief  星广播平台终端控制报文解析类
 * @author Bill
 * @version 0.0.1
 * @date 2021-05-28
 */

#pragma once

#include "termPackage.h"


namespace star_protocol
{

class termPackageImp : public termPackage
{
public:
    termPackageImp();
    virtual ~termPackageImp();
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
    virtual int init(const uint8_t* data, int size, bool ownData = false);
    
    /**
     * @brief  verify 
     *          校验报文（STAG、版本、签名）
     * @param customerNo 客户号，如果customerNo==NULL则从SN中取客户号
     * 
     * @return ture： 校验通过
     *         false：校验失败
     */
    virtual bool verify(const char* customerNo = NULL) const;

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
    virtual int makeSliceAck(uint8_t* buff, int size, const char* customerNo) const;
    /**
     * 报文版本号
     */    
    virtual int ver() const;
    /**
     * 是否包含终端SN
     */
    virtual bool hasSn() const;
    /**
     * qos 是否为1
     */
    virtual bool qos() const;
    /**
     * 是否为ACK报文
     */
    virtual bool isAck() const;
    /**
     * 是否为分片报文
     */
    virtual bool isSlicePkg() const;
    /**
     * 是否为分片确认报文
     */
    virtual bool isSliceAckPkg() const;
    /**
     * 是否包含扩展头
     */
    virtual bool hasExtData() const;
    /**
     * 主题
     */
    virtual uint8_t topic() const;
    /**
     * 命令字
     */
    virtual uint8_t cmd() const;
    /**
     * 报文长度
     */
    virtual uint16_t length() const;
    /**
     * 消息ID
     */
    virtual uint16_t msgId() const;
    /**
     * 消息长度
     */
    virtual uint16_t msgLength() const;
    /**
     * 终端序列号（64位无符号数）
     */
    virtual uint64_t sn() const;
    /**
     * 报文分片序号
     */
    virtual uint8_t sliceIndex() const;
    /**
     * 报文分片总数
     */
    virtual uint8_t sliceTotal() const;
    /**
     * 报文body部分在终端消息中的偏移
     */
    virtual uint16_t sliceOffset() const;
    /**
     * 扩展数据长度
     */
    virtual uint8_t extDataLength() const;
    /**
     * 扩展数据指针
     */
    virtual const uint8_t* extData() const;
    /**
     * body数据指针
     */
    virtual const uint8_t* body() const;
    /**
     * body长度
     */
    virtual uint16_t bodyLength() const;
    /**
     * 签名时间戳
     */
    virtual uint32_t timestamp() const;
    /**
     * 签名
     */
    virtual uint32_t signature() const;
    /**
     * 报文数据指针
     */
    virtual const uint8_t* data() const;
    /**
     * 报文长度
     */
    virtual int size() const;

private:
    const uint8_t* m_data;
    int m_size;
    bool m_ownData;
};

// 传输报文标志
#define STARTP_STAG1_VALUE         0xF5
#define STARTP_STAG2_VALUE         0xAA
// 报文版本
#define STARTP_VERSION             0
// 基础包头长度
#define STARTP_BASE_HEADER_SIZE    12
// 最小报头长度
#define STARTP_MIN_PACKAGE_SIZE    20
// SN偏移
#define STARTP_SN_OFFSET           12
// SN长度
#define STARTP_SN_LENGTH           8
// 分片信息长度
#define STARTP_SLICE_INFO_LENGTH   4
// 签名数据长度（包括时间戳）
#define STARTP_SIGN_DATA_LENGTH    8
// 分片确认报文长度
#define STARTP_SLICE_ACK_LENGTH_WITH_SN    (STARTP_BASE_HEADER_SIZE + STARTP_SN_LENGTH + STARTP_SLICE_INFO_LENGTH + STARTP_SIGN_DATA_LENGTH)
#define STARTP_SLICE_ACK_LENGTH            (STARTP_BASE_HEADER_SIZE + STARTP_SLICE_INFO_LENGTH + STARTP_SIGN_DATA_LENGTH)
// 报文体最大长度
#define STARTP_BODY_MAX_SIZE       1024


#pragma pack(1)
// 固定报文头
typedef struct _ctsp_protocol_header_base
{
    uint8_t stag1;      // 报文起始标志
    uint8_t stag2;      // 报文起始标志
    uint8_t ver : 2;    // 版本号
    uint8_t snf : 1;    // 是否包含SN
    uint8_t qos : 1;    // QOS
    uint8_t ack : 1;    // 应答标志
    uint8_t sf  : 1;    // 分片标志
    uint8_t saf : 1;    // 分片确认标志
    uint8_t exf : 1;    // 扩展数据标志
    uint8_t reserved;   // 保留字段
    uint8_t topic;      // 主题
    uint8_t cmd;        // 命令字
    uint16_t msgId;     // 消息ID
    uint16_t pkgLength; // 报文长度
    uint16_t msgLength; // 消息长度
}ctsp_protocol_header_base;

// 分片信息
typedef struct _ctsp_protocol_header_sliceinfo
{
    uint8_t findex;     // 分片序号，从0开始
    uint8_t ftotal;     // 分片数量
    uint16_t foffset;   // 报文体在消息数据中的偏移
}ctsp_protocol_header_sliceinfo;

// 签名信息
typedef struct _ctsp_signature_data
{
    uint32_t timestamp;     // 时间戳
    uint32_t signature;     // CRC32
}ctsp_signature_data;
#pragma pack()

}