#include "termMessageImp.h"
#include "termPackageImp.h"
#include "protocolUtils.h"
#include "cmdParser.h"
#include "spdlogging.h"
#include "crc32.h"

#include <time.h>
#include <string.h>
#if defined(_WIN32) || defined(WIN32)  || defined(_WIN64) || defined(_WINDOWS)
#include <Windows.h>
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif  // WIN32


termMessage* termMessage::makeTermMessage()
{
	return new star_protocol::termMessageImp();
}

namespace star_protocol
{


termMessageImp::termMessageImp() : 
    m_sn(0), 
    m_qos(false),
    m_ack(false),
    m_topic(0),
    m_cmd(0),
    m_messageId(0),
    m_packageCount(0),
    m_msgData(NULL),
    m_msgDataLength(0)
{

}

termMessageImp::~termMessageImp()
{
    clear();
}

void termMessageImp::clear()
{
    if (m_msgData != NULL)
    {
        delete []m_msgData;
        m_msgData = NULL;
    }
    m_msgDataLength = 0;
    for (std::vector<termPackage*>::iterator it = m_packages.begin(); it != m_packages.end(); ++it)
    {
        delete *it;
    }
    m_packages.clear();
}

uint16_t termMessageImp::messageId() const
{
    return m_messageId;
}

uint64_t termMessageImp::sn() const
{
    return m_sn;
}

bool termMessageImp::qos() const
{
    return m_qos;
}

uint8_t termMessageImp::topic() const
{
    return m_topic;
}

uint8_t termMessageImp::cmd() const
{
    return m_cmd;
}

bool termMessageImp::isComplete() const
{
    return m_msgData != NULL && m_sliceMap.size() == 0;
}

int termMessageImp::initFromTermPackage(const termPackage* pkg, const char* productId)
{
    // 支持多线程安全
    std::lock_guard<std::mutex> guard(m_sliceMapMutex);
    int offset = 0;
    if (m_msgData == NULL)
    {
        m_sn = pkg->sn();
        m_qos = pkg->qos();
        m_ack = pkg->isAck();
        m_topic = pkg->topic();
        m_cmd = pkg->cmd();
        m_messageId = pkg->msgId();
        m_productId = productId;
        m_msgDataLength = pkg->msgLength();
        m_msgData = new uint8_t[m_msgDataLength];
        if (pkg->isSlicePkg())
        {
            offset = pkg->sliceOffset();
            for (int i = 0; i < pkg->sliceTotal(); ++i)
            {
                m_sliceMap[i] = false;
            }
            m_packageCount = pkg->sliceTotal();
        }
    }
    if (pkg->isSlicePkg())
    {
        std::map<uint8_t, bool>::iterator it = m_sliceMap.find(pkg->sliceIndex());
        if (it == m_sliceMap.end())
        {
            return isComplete() ? 1 : 0;
        }
        offset = pkg->sliceOffset();
        m_sliceMap.erase(it);
    }
    memcpy(m_msgData + offset, pkg->body(), pkg->bodyLength());
    return isComplete() ? 1 : 0;
}

int termMessageImp::initFromJson(const Json::Value &jsonRoot, const char* productId, bool withSn, const char* customerNo)
{
    clear();
    if (jsonRoot["cmd"].asInt() != 1 && jsonRoot["topic"].asInt() != 2)
    {
        SPDDEBUG("Create message: {}", jsonRoot.toStyledString());
    }
    m_productId = productId;
    if (jsonRoot.isMember("sn") && jsonRoot["sn"].isString())
    {
        m_sn = devidCache::instance()->str2devid(jsonRoot["sn"].asString());
    }
    else
    {
        m_sn = 0;
    }
    if (jsonRoot.isMember("qos") && jsonRoot["qos"].isInt())
    {
        m_qos = jsonRoot["qos"].asInt() == 1;
    }
    else
    {
        SPDERROR("invalid message, qos is not exist");
        return -1;
    }
    if (jsonRoot.isMember("ack") && jsonRoot["ack"].isInt())
    {
        m_ack = jsonRoot["ack"].asInt() == 1;
    }
    else
    {
        SPDERROR("invalid message, ack is not exist");
        return -1;
    }
    if (jsonRoot.isMember("topic") && jsonRoot["topic"].isUInt())
    {
        m_topic = (uint8_t)jsonRoot["topic"].asUInt();
    }
    else
    {
        SPDERROR("invalid message, topic is not exist");
        return -1;
    }
    if (jsonRoot.isMember("cmd") && jsonRoot["cmd"].isUInt())
    {
        m_cmd = (uint8_t)jsonRoot["cmd"].asUInt();
    }
    else
    {
        SPDERROR("invalid message, cmd is not exist");
        return -1;
    }
    if (jsonRoot.isMember("msgid") && jsonRoot["msgid"].isUInt())
    {
        m_messageId = (uint16_t)jsonRoot["msgid"].asUInt();
    }
    else
    {
        SPDERROR("invalid message, msgid is not exist");
        return -1;
    }
    if (jsonRoot.isMember("msg") && jsonRoot["msg"].isObject())
    {
        const cmdParser *parser = cmdParserRepo::instance()->getCmdParser(m_topic, m_cmd);
        if (NULL == parser)
        {
            SPDERROR("command parser is not exsit for topic:{}, cmd:{}", m_topic, m_cmd);
            return -2;
        }
        int bufLength = (int)jsonRoot["msg"].toStyledString().length();
        uint8_t *buf = new uint8_t[bufLength];
        int msgDataLength = 0;
        if (parser->isPenetrate())
        {
            msgDataLength = parser->j2b(m_productId.c_str(), jsonRoot["msg"], m_ack, buf, bufLength, m_topic, m_cmd);
        }
        else
        {
            msgDataLength = parser->j2b(m_productId.c_str(), jsonRoot["msg"], m_ack, buf, bufLength);
        }
        if (msgDataLength < 0)
        {
            SPDERROR("message json to binary failed, msg:{}, productId:{}", jsonRoot.toStyledString(), m_productId);
            delete []buf;
            return -3;
        }
        m_msgDataLength = msgDataLength;
        m_msgData = buf;
    }

    return makePackages(withSn, customerNo);
}

int termMessageImp::makePackages(bool withSn, const char* customerNo)
{
    if (customerNo == NULL)
    {
        // 未指定客户号时直接通过m_sn计算客户号
        customerNo = devidCache::instance()->customerId2Str(CUSTOM_ID_FROM_DEVID(m_sn));
    }
    // 从消息长度计算分片数量
    int sliceCount = ((m_msgDataLength - 1) / STARTP_BODY_MAX_SIZE) + 1;
    if (sliceCount > 0xFF)
    {
        SPDERROR("message length {} is too bigger", m_msgDataLength);
        return -1;
    }

    // 计算报文头长度，是否包含序列号，是否包含分片信息
    int offset = 0;
    int headerSize = STARTP_BASE_HEADER_SIZE;
    if (withSn)
    {
        headerSize += STARTP_SN_LENGTH;
    }
    if (sliceCount > 1)
    {
        headerSize += STARTP_SLICE_INFO_LENGTH;
    }

    // 生成报文
    for (int i = 0; i < sliceCount; ++i)
    {
        int bodySize = (m_msgDataLength - offset) > STARTP_BODY_MAX_SIZE ? STARTP_BODY_MAX_SIZE : (m_msgDataLength - offset);
        int pkgSize = headerSize + bodySize + STARTP_SIGN_DATA_LENGTH;
        uint8_t *buff = new uint8_t[pkgSize];
        int buffOffset = 0;
        ctsp_protocol_header_base* header = (ctsp_protocol_header_base*)buff;
        header->stag1 = STARTP_STAG1_VALUE;             // 报文起始标志
        header->stag2 = STARTP_STAG2_VALUE;             // 报文起始标志
        header->ver = STARTP_VERSION;                   // 版本号
        header->snf = withSn ? 1 : 0;                   // 是否包含SN
        header->qos = m_qos ? 1 : 0;                    // QOS
        header->ack = m_ack ? 1 : 0;                    // 应答标志
        header->sf = (sliceCount > 1) ? 1 : 0;          // 分片标志
        header->saf = 0;                                // 分片确认标志
        header->exf = 0;                                // 扩展数据标志
        header->reserved = 0;                           // 保留字段
        header->topic = m_topic;                        // 主题
        header->cmd = m_cmd;                            // 命令字
        header->pkgLength = htons(pkgSize);                    // 报文长度
        header->msgId = htons(m_messageId);             // 消息ID
        header->msgLength = htons(m_msgDataLength);     // 消息长度
        buffOffset += STARTP_BASE_HEADER_SIZE;

        if (withSn)
        {
            // 序列号
            u64tobytes(m_sn, buff + buffOffset, STARTP_SN_LENGTH);
            buffOffset += STARTP_SN_LENGTH;
        }
        if (sliceCount > 1)
        {
            // 分片信息
            ((ctsp_protocol_header_sliceinfo*)(buff + buffOffset))->findex = i;
            ((ctsp_protocol_header_sliceinfo*)(buff + buffOffset))->ftotal = sliceCount;
            ((ctsp_protocol_header_sliceinfo*)(buff + buffOffset))->foffset = htons(offset);
            buffOffset += STARTP_SLICE_INFO_LENGTH;
        }

        // 报文体
        memcpy(buff + buffOffset, m_msgData + offset, bodySize);
        offset += bodySize;
        buffOffset += bodySize;

        // 时间戳
        *(uint32_t*)(buff + buffOffset) = htonl((uint32_t)time(NULL));
        buffOffset += 4;
        // CRC32
        *(uint32_t*)(buff + buffOffset) = htonl(crc32(buff, buffOffset, (const uint8_t*)customerNo, (int)strlen(customerNo)));

        termPackageImp *pkg = new termPackageImp();
        pkg->init(buff, pkgSize, true);
        m_packages.push_back(pkg);
    }
    m_packageCount = sliceCount;
    return 0;
}

int termMessageImp::termPackageCount()
{
    return m_packageCount;
}

const termPackage* termMessageImp::getTermPackage(int index)
{
    if (index >= m_packageCount)
    {
        return NULL;
    }

    return m_packages[index];
}

const Json::Value& termMessageImp::getJsonMsg()
{
    if (!isComplete())
    {
        SPDERROR("message is not complete");
        return m_jsonMsgRoot;
    }
    m_jsonMsgRoot.clear();
    m_jsonMsgRoot["sn"] = m_sn != 0 ? devidCache::instance()->devid2str(m_sn) : "";
    m_jsonMsgRoot["qos"] = m_qos ? 1 : 0;
    m_jsonMsgRoot["ack"] = m_ack ? 1 : 0;
    m_jsonMsgRoot["topic"] = m_topic;
    m_jsonMsgRoot["cmd"] = m_cmd;
    m_jsonMsgRoot["msgid"] = m_messageId;
    const cmdParser *parser = cmdParserRepo::instance()->getCmdParser(m_topic, m_cmd);
    if (NULL == parser)
    {
        SPDERROR("command parser is not exsit for topic:{}, cmd:{}", m_topic, m_cmd);
        m_jsonMsgRoot.clear();
        return m_jsonMsgRoot;
    }
    if (parser->isPenetrate())
    {
        parser->b2j(m_productId.c_str(), m_msgData, m_msgDataLength, m_ack, m_jsonMsgRoot["msg"], m_topic, m_cmd);
    }
    else
    {
        parser->b2j(m_productId.c_str(), m_msgData, m_msgDataLength, m_ack, m_jsonMsgRoot["msg"]);
    }
    SPDDEBUG("binary to json msg: {}", m_jsonMsgRoot.toStyledString());
    return m_jsonMsgRoot;
}

}