#include "pch.h"
#include "UdpClient.h"
#include <time.h>
#include <atomic>
#include "termMessage.h"
#include "termPackage.h"
static bool isRtpPacket(const uint8_t *data, int size)
{
	const rtp_header_t *h = (const rtp_header_t*)data;
	return size > RTP_HEADER_SIZE && h->v == RTP_VERSION;
}
std::atomic<int> count = 0;
std::atomic<int> count1 = 0;
std::atomic<int> count2 = 0;
std::atomic<int> count3 = 0;
bool g_flag = false;
MemoryPool* mp = MemoryPool::GetInstance();
std::atomic<int> CUdpClient::g_msgid = 1;
std::atomic<int> g_msgid1 = 1;
std::atomic<int> g_msgid2 = 1;
std::atomic<int> g_msgid3 = 1;
std::atomic<long long> g_RecvCount = 0;
std::atomic<long long> g_SendCount = 0;

static void SendHartPacket(void* arg)
{
	CUdpClient* thiz = (CUdpClient*)arg;
	if (!thiz)return;
	while (!g_flag)
	{
		if (count)
		{
	/*		for (auto sn : thiz->m_snlist)
			{
				thiz->data.m_root["sn"] = sn;
				thiz->data.m_root["msgid"] = thiz->g_msgid++;
				if ((thiz->sendto(thiz->data, thiz->m_devaddr)) != 0)
				{
					LOG_E("考试心跳发送失败!");
				}
			}*/
			for (int i = 0; i < 40; i++)
			{
				thiz->data.m_root["sn"] = thiz->m_snlist[i];
				thiz->data.m_root["msgid"] = g_msgid1++;
				if ((thiz->sendto(thiz->data, thiz->m_devaddr)) != 0)
				{
					LOG_E("考试心跳发送失败!");
				}
			}
			--count;
			LOG_E("心跳");
		}
		else
		{
			Sleep(1);
		}
	}
}
static void SendHartPacket1(void* arg)
{
	CUdpClient* thiz = (CUdpClient*)arg;
	if (!thiz)return;
	while (!g_flag)
	{
		if (count1)
		{
			for (int i = 40; i < 80; i++)
			{
				thiz->data1.m_root["sn"] = thiz->m_snlist[i];
				thiz->data1.m_root["msgid"] = g_msgid1++;
				if ((thiz->sendto(thiz->data1, thiz->m_devaddr)) != 0)
				{
					LOG_E("考试心跳发送失败!");
				}
			}
			--count1;
			//LOG_E("心跳");
		}
		else
		{
			Sleep(1);
		}
	}
}
static void SendHartPacket2(void* arg)
{
	CUdpClient* thiz = (CUdpClient*)arg;
	if (!thiz)return;
	while (!g_flag)
	{
		if (count2)
		{
			for (int i = 80; i < 120; i++)
			{
				thiz->data2.m_root["sn"] = thiz->m_snlist[i];
				thiz->data2.m_root["msgid"] = g_msgid2++;
				if ((thiz->sendto(thiz->data2, thiz->m_devaddr)) != 0)
				{
					LOG_E("考试心跳发送失败!");
				}
			}
			--count2;
			//LOG_E("心跳");
		}
		else
		{
			Sleep(1);
		}
	}
}
static void SendHartPacket3(void* arg)
{
	CUdpClient* thiz = (CUdpClient*)arg;
	if (!thiz)return;
	while (!g_flag)
	{
		if (count3)
		{
			for (int i = 120; i < 160; i++)
			{
				thiz->data3.m_root["sn"] = thiz->m_snlist[i];
				thiz->data3.m_root["msgid"] = g_msgid3++;
				if ((thiz->sendto(thiz->data3, thiz->m_devaddr)) != 0)
				{
					LOG_E("考试心跳发送失败!");
				}
			}
			--count3;
			//LOG_E("心跳");
		}
		else
		{
			Sleep(1);
		}
	}
}


static VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired) //拉流心跳
{
	CUdpClient* thiz = (CUdpClient*)lpParam;
	if (!thiz || !thiz->m_network)return;
	if (!thiz->m_exit)
	{
		JsonData data_;
		data_.m_root["qos"] = 0;
		data_.m_root["ack"] = 0;
		data_.m_root["topic"] = 2;
		data_.m_root["cmd"] = 11;
		data_.m_root["msg"]["url"] = thiz->m_info->m_TargeAddr.c_str();
		data_.m_root["msg"]["streamDataTime"] = thiz->m_streamDataTime;
		for (auto sn : thiz->m_snlist)
		{
			data_.m_root["sn"] = sn;
			data_.m_root["msgid"] = thiz->g_msgid++;
			if ((thiz->sendto(data_, thiz->m_devaddr)) != 0)
			{
				LOG_E("心跳发送失败!");
			}
		}
		if (!thiz->m_start)thiz->m_start = true;
	}
}

CUdpClient::CUdpClient(ModInfo& info, const ACE_INET_Addr & devaddr, int id, int mantissa)
{
	m_exit = false;
	m_start = false;
	m_streamid = -1;
	m_info = &info;
	m_devaddr = devaddr;
	m_network = nullptr;
	m_streamDataTime = 0;
	m_hTimer = INVALID_HANDLE_VALUE;
	m_hTimerQueueTimer = INVALID_HANDLE_VALUE;
	ACE_INET_Addr sendsockbindaddr(LOCALADDR);
	m_sock.open(sendsockbindaddr);

	int bufsiz = 64 * 1024 * 1024;
	int re = m_sock.set_option(SOL_SOCKET, SO_SNDBUF, &bufsiz, sizeof(bufsiz));
	re = m_sock.set_option(SOL_SOCKET, SO_RCVBUF, &bufsiz, sizeof(bufsiz));
	for (int i = 0; i < mantissa; i++)m_idlist.push_back(++id);
	std::thread* thread = new std::thread(SendHartPacket, this);
	std::thread* thread1 = new std::thread(SendHartPacket1, this);
	std::thread* thread2 = new std::thread(SendHartPacket2, this);
	std::thread* thread3 = new std::thread(SendHartPacket3, this);
	m_threads.push_back(thread);
	m_threads.push_back(thread1);
	m_threads.push_back(thread2);
	m_threads.push_back(thread3);
}

CUdpClient::~CUdpClient()
{
	Close();
}

void CUdpClient::Init()
{
	m_network = m_info->m_network;
	if (!m_network)return;
	UpdataSerialNumber(m_snlist);
	m_network->register_handler(this);
	for (auto sn : m_snlist)//开始拉流请求
	{
		m_data.Init(sn, m_info->m_TargeAddr, -1, true);
		int len = this->sendto(m_data, m_devaddr);
		if (len != 0)
		{
			LOG_E("拉流请求发送失败!");
			return;
		}
	}
	StartTimer(); //拉流心跳
	if (m_info->m_isExam)m_network->register_timer_handler(this, NULL, 0, m_info->m_examhart);//考试心跳
}

int CUdpClient::handle_timeout(const ACE_Time_Value & tv, const void * arg)
{
	if (m_exit)return -1;//停止测试
	if (m_start)
	{
		++count;
		++count1;
		++count2;
		++count3;
	}
	return 0;
}

void CUdpClient::Close()
{
	m_exit = true;
	for (auto sn : m_snlist)
	{
		m_data.Init(sn, m_info->m_TargeAddr, -1, false);
		int len = this->sendto(m_data, m_devaddr);
		if (len != 0)
		{
			LOG_E("停止拉流发送失败!");
		}
	}
	g_flag = true;
	if (!m_threads.empty())
	{
		for (auto thread : m_threads)
		{
			if (thread)
			{
				if (thread->joinable())
				{
					thread->join();
					delete thread;
					thread = nullptr;
				}
			}
		}
	}
	if (m_hTimerQueueTimer != INVALID_HANDLE_VALUE)
	{
		DeleteTimerQueueTimer(m_hTimer, m_hTimerQueueTimer, INVALID_HANDLE_VALUE);
		m_hTimerQueueTimer = INVALID_HANDLE_VALUE;
	}
	if (m_hTimer != INVALID_HANDLE_VALUE)
	{
		DeleteTimerQueueEx(m_hTimer, INVALID_HANDLE_VALUE);
		m_hTimer = INVALID_HANDLE_VALUE;
	}
	if (m_network)
	{
		m_network->remove_timer_handler(this);
		m_network->remove_handler(this);
		m_network = nullptr;
	}
	m_sock.close();
}

void CUdpClient::StartTimer()
{
	if (m_hTimer == INVALID_HANDLE_VALUE && m_hTimerQueueTimer == INVALID_HANDLE_VALUE)
	{
		m_hTimer = CreateTimerQueue();
		if (m_hTimer != INVALID_HANDLE_VALUE)
		{
			if (!CreateTimerQueueTimer(&m_hTimerQueueTimer, m_hTimer, TimerRoutine, this, 0, m_info->m_hart * 1000, WT_EXECUTEDEFAULT))
			{
				m_hTimer = INVALID_HANDLE_VALUE;
				m_hTimerQueueTimer = INVALID_HANDLE_VALUE;
			}
		}
		else
		{
			m_hTimer = INVALID_HANDLE_VALUE;
			m_hTimerQueueTimer = INVALID_HANDLE_VALUE;
		}
	}
}

void CUdpClient::UpdataSerialNumber(std::vector<std::string>& str)
{
	char sn[64];
	time_t timep;
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
	for (auto id : m_idlist)
	{
		int len = sprintf(sn, "015-001-08394-%04d%02d%02d-%05d", 1900 + p->tm_year, p->tm_mon + 1, p->tm_mday, id);
		m_snlist.push_back(sn);
	}
	data.m_root.clear();
	data.m_root["qos"] = 0;
	data.m_root["ack"] = 0;
	data.m_root["topic"] = 2;
	data.m_root["cmd"] = 9;

	data1.m_root.clear();
	data1.m_root["qos"] = 0;
	data1.m_root["ack"] = 0;
	data1.m_root["topic"] = 2;
	data1.m_root["cmd"] = 9;

	data2.m_root.clear();
	data2.m_root["qos"] = 0;
	data2.m_root["ack"] = 0;
	data2.m_root["topic"] = 2;
	data2.m_root["cmd"] = 9;

	data3.m_root.clear();
	data3.m_root["qos"] = 0;
	data3.m_root["ack"] = 0;
	data3.m_root["topic"] = 2;
	data3.m_root["cmd"] = 9;
	m_sn = m_snlist[0];
}

void * CUdpClient::operator new(size_t)
{
	return mp->Alloc(sizeof(CUdpClient));
}

void CUdpClient::operator delete(void * p)
{
	mp->FreeAlloc(p);
}

int CUdpClient::handle(const Json::Value & data, const ACE_INET_Addr & from_addr)
{
	if (data["cmd"].asInt() == 3 && data["topic"].asInt() == 2) //拉流应答
	{
		m_streamid = data["msg"]["streamid"].asInt();
	}
	if (data["cmd"].asInt() == 10 && data["topic"].asInt() == 2) //考试应答
	{
		g_RecvCount++;
	}
	return 0;
}

int CUdpClient::handle(const char * buf, const ACE_INET_Addr & from_addr, size_t size)
{
	return 0;
}

int CUdpClient::sendto(const Json::Value & data, const ACE_INET_Addr & to_addr)
{
	if (!m_network)return -1;
	if (data["topic"].asInt() != 2)
	{
		LOG_E("[Network] Sendto error,topic!=2");
		return -1;
	}
	int tp = data["topic"].asInt();
	int cmd = data["cmd"].asInt();
	//将json格式数据转换成二进制数据
	std::shared_ptr<termMessage> sendMsg(termMessage::makeTermMessage());
	if (0 != sendMsg->initFromJson(data, "", true))
	{
		LOG_E("[Network] Senddata initFromJson error");
		return -1;
	}
	for (int i = 0; i < sendMsg->termPackageCount(); ++i)
	{
		const termPackage* pkg = sendMsg->getTermPackage(i);
		int size = pkg->size();
		u_short port = to_addr.get_port_number();
		int ret = (int)m_sock.send(pkg->data(), pkg->size(), to_addr);
		if (ret != pkg->size())
		{
			int err = ACE_OS::last_error();
			LOG_E("[Network] Send socket send error");
			return -1;
		}
	}
	return 0;
}

int CUdpClient::sendto(const char * buf, const ACE_INET_Addr & to_addr, size_t size)
{
	if (!m_network)return -1;
	int ret = m_sock.send(buf, size, to_addr);
	if (ret != size)
	{
		int er = ACE_OS::last_error();
		LOG_E("[Network] RTP data send error");
		return -1;
	}
	return 0;
}

int CUdpClient::handle_input(ACE_HANDLE fd)
{
	if (!m_network)return -1;
	//收取终端数据
	byte_t buff[1600];
	ACE_INET_Addr remoteAddr;
	ACE_Time_Value t(1);
	//所连接的远程地址  
	int res = (int)m_sock.recv(buff, sizeof(buff), remoteAddr, 0, &t);
	int err = ACE_OS::last_error();
	//判断收到的包是星协议还是rtp数据,如果都不是则不进行转发
	if (res >= 13 && buff[0] == 0xf5)
	{
		//将二进制数据转换为Json格式
		std::shared_ptr<termPackage> pkg(termPackage::makeTermPackage());
		pkg->init(buff, res);
		if (!pkg->verify())
		{
			LOG_E("[Network] packet init error");
			return 1;
		}
		std::shared_ptr<termMessage> msg(termMessage::makeTermMessage());
		msg->initFromTermPackage(pkg.get(), "");
		if (!msg->isComplete())
		{
			LOG_E("[Network] initFromTermPackage error");
			return 1;
		}
		const Json::Value &jsonMsg = msg->getJsonMsg();

		//只有当handler存在且topic等于51时才会转发协议给测试项
		if (jsonMsg.isMember("topic") && (jsonMsg["topic"].asInt() == 2))
		{
			this->handle(jsonMsg, remoteAddr);
		}
	}
	else if (isRtpPacket(buff, res))
	{
		this->handle((const char*)buff, remoteAddr, res);
	}
	return 0;
}
