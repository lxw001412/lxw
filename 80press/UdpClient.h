#pragma once
#include "CHandle.h"
#include "CJson.h"
#include "common.h"
#include "List_Node.h"
#include "MemoryPool.h"
#include <vector>
#include <thread>
typedef unsigned char byte_t;
#define RECV_BUFFER_SIZE 2000


struct TermUdpMessage
{
	std::string msg;
	ACE_INET_Addr addr;
};

typedef struct _rtp_header_t
{
	uint8_t cc : 4;
	uint8_t x : 1;
	uint8_t p : 1;
	uint8_t v : 2;
	uint8_t pt : 7;
	uint8_t m : 1;
	uint16_t seqno;
	uint32_t timestamp;
	uint32_t ssrc;
}rtp_header_t;

#define RTP_HEADER_SIZE (sizeof(rtp_header_t))
#define RTP_VERSION 2
struct ModInfo;
class CUdpClient : public IHandler , public ACE_Event_Handler
{
public:
	CUdpClient(ModInfo& info, const ACE_INET_Addr & devaddr,int id, int mantissa);
	~CUdpClient();
	static void* operator new(size_t);
	static void operator delete(void* p);
	void Init();
	virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);
public:
	void Close();
	void StartTimer();
	virtual int handle(const Json::Value &data, const ACE_INET_Addr &from_addr);
	virtual int handle(const char* buf, const ACE_INET_Addr &from_addr, size_t size);

	virtual int sendto(const Json::Value& data, const ACE_INET_Addr &to_addr);
	virtual int sendto(const char* buf, const ACE_INET_Addr &to_addr, size_t size);
protected:
	virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
	virtual ACE_HANDLE get_handle(void) const
	{
		return m_sock.get_handle();
	}
	void UpdataSerialNumber(std::vector<std::string>& str);
public:
	std::string m_sn;
	bool m_exit;
	bool m_start;
	int m_streamDataTime;
	int m_streamid;
	JsonData  m_data;
	JsonData data;
	JsonData data1;
	JsonData data2;
	JsonData data3;
	ModInfo*    m_info;
	ACE_SOCK_Dgram m_sock;
	ReactorSvr* m_network;
	ACE_INET_Addr m_devaddr;
	static std::atomic<int> g_msgid;
	std::vector<int> m_idlist;
	std::vector<std::string> m_snlist;
private:
	HANDLE	m_hTimer;
	HANDLE	m_hTimerQueueTimer;
	std::vector<std::thread*> m_threads;
};

