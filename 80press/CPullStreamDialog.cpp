// CPullStreamDialog.cpp: 实现文件
//

#include "pch.h"
#include "80-PressTest.h"
#include "CPullStreamDialog.h"
#include "afxdialogex.h"
#include "UdpClient.h"
#include "common.h"

// CPullStreamDialog 对话框

static void ThreadWorker(void* arg)
{
	CPullStreamDialog* thiz = (CPullStreamDialog*)arg;
	if (!thiz)return;
	int size = thiz->m_size;
	while (true)
	{
		if (thiz->m_status)
		{
			while (size-- && (!thiz->m_exit))
			{
				CUdpClient* client = new CUdpClient();
				if (!client)
				{
					LOG_E("创建客户端失败");
				}
				client->Init(thiz->m_type, thiz->m_network, thiz->m_addr, thiz->m_hart, thiz->m_TargeAddr);
				int nCount = thiz->m_size - size - 1; //行数
				thiz->m_maplist[*client] = nCount;
				thiz->m_clientlist.push(client);
				thiz->m_listclient.InsertItem(nCount, *client);
				thiz->m_listclient.SetItemText(nCount, 1, _T("ESTABLISHED"));
			}
			thiz->m_status = false;
		}
		else if (thiz->m_exit)
		{
			thiz->m_listclient.DeleteAllItems();
			int nCount = -1;
			CString str;
			while (!thiz->m_clientlist.empty())
			{
				if (thiz->m_clientlist.front())
				{
					str.Format(_T("%d"), thiz->m_clientlist.size());
					nCount = thiz->m_maplist[*thiz->m_clientlist.front()];
					thiz->m_listclient.InsertItem(nCount, str.AllocSysString());
					if (!thiz->m_listclient.SetItemText(nCount, 1, _T("CLOSED")))
					{
						LOG_E("ERROR");
					}
					delete thiz->m_clientlist.front();
					thiz->m_clientlist.front() = nullptr;
					thiz->m_clientlist.pop();
				}
			}
			thiz->m_exit = false;
			break;
		}
		else
		{
			Sleep(1);
		}
	}
}

IMPLEMENT_DYNAMIC(CPullStreamDialog, CDialogEx)

CPullStreamDialog::CPullStreamDialog(INetwork & network, CWnd* pParent)
	: CDialogEx(IDD_StreamDialog, pParent)
{
	m_network = &network;
	m_status = false;
	m_exit = false;
	m_Thread = nullptr;
	m_hart = -1;
	m_size = -1;
	m_testing = false;
	m_type = None;
}

CPullStreamDialog::~CPullStreamDialog()
{
	Close();
}

void CPullStreamDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IP, m_ip);
	DDX_Control(pDX, IDC_Port, m_port);
	DWORD   dwIP = inet_addr(IP);
	unsigned   char   *pIP = (unsigned   char*)&dwIP;   // BYTE也可以
	m_ip.SetAddress(pIP[0], pIP[1], pIP[2], pIP[3]);
	m_port.SetWindowText(_T(PORT));
	DDX_Control(pDX, IDC_LIST1, m_listclient);
	GetDlgItem(IDC_Adress_)->SetWindowText(_T("rtmp://113.240.243.236/live?vhost=live.test/test-0309"));
	m_listclient.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES); // 整行选择、网格线
	m_listclient.InsertColumn(0, _T("客户端ID"), LVCFMT_CENTER, 250);
	m_listclient.InsertColumn(1, _T("连接状态"), LVCFMT_CENTER, 404); // 插入第2列的列名
}

void CPullStreamDialog::Close()
{
	m_testing = false;
	m_status = false;
	m_exit = true;
	m_maplist.clear();
	if (m_Thread)
	{
		m_Thread->detach();
		delete m_Thread;
		m_Thread = nullptr;
	}
}

void CPullStreamDialog::InitAddres(ACE_INET_Addr & addr)
{
	CString str;
	m_port.GetWindowText(str);
	const char* s = CString2C(str);
	int port = atoi(s);
	delete s;
	s = nullptr;
	if (m_ip.IsBlank())return;
	BYTE IPByte[4];
	m_ip.GetAddress(IPByte[0], IPByte[1], IPByte[2], IPByte[3]);
	CString strIP2;
	char temp1[10], temp2[10], temp3[10], temp4[10];
	_itoa(IPByte[0], temp1, 10);
	_itoa(IPByte[1], temp2, 10);
	_itoa(IPByte[2], temp3, 10);
	_itoa(IPByte[3], temp4, 10);
	strIP2 += temp1;
	strIP2 += ".";
	strIP2 += temp2;
	strIP2 += ".";
	strIP2 += temp3;
	strIP2 += ".";
	strIP2 += temp4;
	s = CString2C(strIP2);
	if (str.IsEmpty())
	{
		return;
	}
	ACE_INET_Addr add(port, s);
	delete s;
	s = nullptr;
	addr = add;
}

void CPullStreamDialog::SetNumber(int id, void * arg)
{
	CString str;
	const char* temp = nullptr;
	GetDlgItem(id)->GetWindowText(str); //拉流地址
	temp = CString2C(str);
	arg = (void*)temp;
	delete temp;
	temp = nullptr;
}


BEGIN_MESSAGE_MAP(CPullStreamDialog, CDialogEx)
	ON_BN_CLICKED(IDC_Start, &CPullStreamDialog::OnBnClickedStart)
	ON_BN_CLICKED(IDC_Stop2, &CPullStreamDialog::OnBnClickedStop2)
	ON_BN_CLICKED(IDC_Exit, &CPullStreamDialog::OnBnClickedExit)
END_MESSAGE_MAP()


// CPullStreamDialog 消息处理程序

void CPullStreamDialog::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_testing && m_exit)return; //正在运行或者线程还未停止
	m_listclient.DeleteAllItems();
	m_testing = true;
	int status = ((CButton*)GetDlgItem(IDC_Check))->GetCheck();
	m_type = status == 1 ? Examination : PullStream;

	InitAddres(m_addr);
	if (!m_network)return;
	CString str;
	const char* s = CString2C(str);
	GetDlgItem(IDC_Adress_)->GetWindowText(str); //拉流地址
	s = CString2C(str);
	m_TargeAddr = s;
	delete s;
	s = nullptr;

	GetDlgItem(IDC_Hart_)->GetWindowText(str);
	s = CString2C(str);
	m_hart = atoi(s);
	delete s;
	s = nullptr;
	GetDlgItem(IDC_stream_count)->GetWindowText(str);
	s = CString2C(str);
	m_size = atoi(s);
	delete s;
	s = nullptr;
	if (m_size <= 0)
	{
		MessageBox(_T("请求数量不能为空"));
		return;
	}
	m_status = true;
	if (m_Thread)return;
	m_Thread = new std::thread(&ThreadWorker, this);
}

void CPullStreamDialog::OnBnClickedStop2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_testing)return;
	Close();
}


void CPullStreamDialog::OnBnClickedExit()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_testing)
	{
		MessageBox(_T("程序正在运行"));
		return;
	}
	if (m_exit)
	{
		MessageBox(_T("等待客户端退出"));
		return;
	}
	AfxGetMainWnd()->SendMessage(WM_CLOSE);
}
