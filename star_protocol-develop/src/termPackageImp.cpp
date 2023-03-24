#include "termPackageImp.h"
#include "protocolUtils.h"
#include "crc32.h"
#include "Exception.h"
#include "spdlogging.h"

#include <time.h>
#include <string.h>
#include <exception>
#if defined(_WIN32) || defined(WIN32)  || defined(_WIN64) || defined(_WINDOWS)
#include <Windows.h>
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif  // WIN32


termPackage* termPackage::makeTermPackage()
{
	return new star_protocol::termPackageImp();
}

namespace star_protocol
{


termPackageImp::termPackageImp() : m_data(NULL), m_size(0), m_ownData(false)
{

}

termPackageImp::~termPackageImp()
{
    if (m_ownData && m_data != NULL)
    {
        delete []m_data;
    }
}

int termPackageImp::init(const uint8_t* data, int size, bool ownData)
{
    m_data = data;
    m_size = size;
    m_ownData = ownData;
    return 0;
}

int termPackageImp::ver() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return (int)((ctsp_protocol_header_base*)m_data)->ver;
}

bool termPackageImp::hasSn() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ((ctsp_protocol_header_base*)m_data)->snf == 1;
}

bool termPackageImp::qos() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ((ctsp_protocol_header_base*)m_data)->qos == 1;
}

bool termPackageImp::isAck() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ((ctsp_protocol_header_base*)m_data)->ack == 1;
}

bool termPackageImp::isSlicePkg() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ((ctsp_protocol_header_base*)m_data)->sf == 1;
}

bool termPackageImp::isSliceAckPkg() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ((ctsp_protocol_header_base*)m_data)->saf == 1;
}

bool termPackageImp::hasExtData() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ((ctsp_protocol_header_base*)m_data)->exf == 1;
}

uint8_t termPackageImp::topic() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ((ctsp_protocol_header_base*)m_data)->topic;
}

uint8_t termPackageImp::cmd() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ((ctsp_protocol_header_base*)m_data)->cmd;
}

uint16_t termPackageImp::length() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ntohs(((ctsp_protocol_header_base*)m_data)->pkgLength);
}

uint16_t termPackageImp::msgId() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ntohs(((ctsp_protocol_header_base*)m_data)->msgId);
}

uint16_t termPackageImp::msgLength() const
{
    if (m_data == NULL || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
        throw Exception("term package not init");
    }
    return ntohs(((ctsp_protocol_header_base*)m_data)->msgLength);
}

uint64_t termPackageImp::sn() const
{
    if (!this->hasSn())
    {
        return 0;
    }
    return bytes2u64(m_data + STARTP_BASE_HEADER_SIZE);
}

uint8_t termPackageImp::sliceIndex() const
{
    if (!this->isSlicePkg() && !this->isSliceAckPkg())
    {
        return 0;
    }
    if (this->hasSn())
    {
        return ((ctsp_protocol_header_sliceinfo*)(m_data + STARTP_BASE_HEADER_SIZE + STARTP_SN_LENGTH))->findex;
    }
    else
    {
        return ((ctsp_protocol_header_sliceinfo*)(m_data + STARTP_BASE_HEADER_SIZE))->findex;
    }
}

uint8_t termPackageImp::sliceTotal() const
{
    if (!this->isSlicePkg() && !this->isSliceAckPkg())
    {
        return 1;
    }
    if (this->hasSn())
    {
        return ((ctsp_protocol_header_sliceinfo*)(m_data + STARTP_BASE_HEADER_SIZE + STARTP_SN_LENGTH))->ftotal;
    }
    else
    {
        return ((ctsp_protocol_header_sliceinfo*)(m_data + STARTP_BASE_HEADER_SIZE))->ftotal;
    }
}

uint16_t termPackageImp::sliceOffset() const
{
    if (!this->isSlicePkg() && !this->isSliceAckPkg())
    {
        return 0;
    }
    if (this->hasSn())
    {
        return ntohs(((ctsp_protocol_header_sliceinfo*)(m_data + STARTP_BASE_HEADER_SIZE + STARTP_SN_LENGTH))->foffset);
    }
    else
    {
        return ntohs(((ctsp_protocol_header_sliceinfo*)(m_data + STARTP_BASE_HEADER_SIZE))->foffset);
    }
}

uint8_t termPackageImp::extDataLength() const
{
    if (!this->hasExtData())
    {
        return 0;
    }
    int offset = STARTP_BASE_HEADER_SIZE;
    if (this->hasSn())
    {
        offset += STARTP_SN_LENGTH;
    }
    if (this->isSlicePkg() || this->isSliceAckPkg())
    {
        offset += STARTP_SLICE_INFO_LENGTH;
    }
    return *(m_data + offset);
}

const uint8_t* termPackageImp::extData() const
{
    int offset = STARTP_BASE_HEADER_SIZE;
    if (this->hasSn())
    {
        offset += STARTP_SN_LENGTH;
    }
    if (this->isSlicePkg() || this->isSliceAckPkg())
    {
        offset += STARTP_SLICE_INFO_LENGTH;
    }
    return m_data + offset + 1;
}

const uint8_t* termPackageImp::body() const
{
    int offset = STARTP_BASE_HEADER_SIZE;
    if (this->hasSn())
    {
        offset += STARTP_SN_LENGTH;
    }
    if (this->isSlicePkg() || this->isSliceAckPkg())
    {
        offset += STARTP_SLICE_INFO_LENGTH;
    }
    if (this->hasExtData())
    {
        offset += this->extDataLength() + 1;
    }
    return m_data + offset;
}

uint16_t termPackageImp::bodyLength() const
{
    int offset = STARTP_BASE_HEADER_SIZE;
    if (this->hasSn())
    {
        offset += STARTP_SN_LENGTH;
    }
    if (this->isSlicePkg() || this->isSliceAckPkg())
    {
        offset += STARTP_SLICE_INFO_LENGTH;
    }
    if (this->hasExtData())
    {
        offset += this->extDataLength() + 1;
    }
    return m_size - offset - STARTP_SIGN_DATA_LENGTH;
}

uint32_t termPackageImp::timestamp() const
{
    return ntohl(((ctsp_signature_data*)(m_data + m_size - STARTP_SIGN_DATA_LENGTH))->timestamp);
}

uint32_t termPackageImp::signature() const
{
    return ntohl(((ctsp_signature_data*)(m_data + m_size - STARTP_SIGN_DATA_LENGTH))->signature);
}

bool termPackageImp::verify(const char* customerNo) const
{
    if (NULL == m_data)
    {
        SPDERROR("Package data is NULL !!!");
        return false;
    }
    if (*m_data != STARTP_STAG1_VALUE
        || *(m_data + 1) != STARTP_STAG2_VALUE
        || this->ver() != STARTP_VERSION
        || m_size < STARTP_MIN_PACKAGE_SIZE)
    {
		SPDERROR("Invalid term package, STAG: {}, version: {}, size: {}", *m_data, this->ver(), m_size);
        return false;
    }
    if (customerNo == NULL)
    {
        customerNo = devidCache::instance()->customerId2Str(CUSTOM_ID_FROM_DEVID(sn()));
    }
    uint32_t crc = crc32(m_data, m_size - 4, (const uint8_t*)customerNo, (int)strlen(customerNo));
    bool result =  this->signature() == crc;
    if (!result)
    {
	    SPDERROR("crc32 error, expect: {}, actual: {}, customerNo: {}", this->signature(), crc, customerNo);
    }
    return result;
}

const uint8_t* termPackageImp::data() const
{
    return m_data;
}

int termPackageImp::size() const
{
    return m_size;
}

int termPackageImp::makeSliceAck(uint8_t* buff, int size, const char* customerNo) const
{
    int offset = 0;
    if (size < STARTP_SLICE_ACK_LENGTH)
    {
        return -1;
    }
    // 复制基础报文头
    memcpy(buff, m_data, STARTP_BASE_HEADER_SIZE);

    ctsp_protocol_header_base* header = (ctsp_protocol_header_base*)buff;
    // 设置分片确认报文
    header->saf = 1;
    header->qos = 0;
    header->ack = 0;
    header->sf = 0;
    header->exf = 0;
    
    offset = STARTP_BASE_HEADER_SIZE;
    if (this->hasSn())
    {
        memcpy(buff + offset, m_data + STARTP_BASE_HEADER_SIZE, STARTP_SN_LENGTH + STARTP_SLICE_INFO_LENGTH);
        offset += STARTP_SN_LENGTH;
        offset += STARTP_SLICE_INFO_LENGTH;
    }
    else
    {
        memcpy(buff + offset, m_data + STARTP_BASE_HEADER_SIZE, STARTP_SLICE_INFO_LENGTH);
        offset += STARTP_SLICE_INFO_LENGTH;
    }
    *(uint32_t*)(buff + offset) = htonl((uint32_t)time(NULL));
    offset += 4;
    *(uint32_t*)(buff + offset) = htonl(crc32(buff, offset, (const uint8_t*)customerNo, (int)strlen(customerNo)));
    offset += 4;
    
    // 设置报文长度
    header->pkgLength = offset;

    return offset;
}

}