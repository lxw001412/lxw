
// 80-PressTestDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "80-PressTest.h"
#include "80-PressTestDlg.h"
#include "afxdialogex.h"
#include "UdpClient.h"
#include "common.h"
#include "ace/Init_ACE.h"
// CPullStreamDialog 对话框
static int g_sn = 0;
static void ThreadWorker(void* arg) /////////考试模式下，一个client对应50个请求，非考试下一个client对应1000个请求////////////////
{
	CMy80PressTestDlg* thiz = (CMy80PressTestDlg*)arg;
	if (!thiz)return;
	int size = thiz->m_info.m_size;
	int count;
	int sn = 0;
	CString str;
	CUdpClient* client = nullptr;
	while (true)
	{
		if (thiz->m_status)
		{
			if (thiz->m_info.m_isExam)//考试模式
			{
				count = (size % EXAMINATION) == 0 ? (size / EXAMINATION) : ((size / EXAMINATION) + 1);
				while (count--)
				{
					if (size < EXAMINATION)
					{
						client = new CUdpClient(thiz->m_info, thiz->m_addr, g_sn, size);
						while (size--)
						{
							str.Format(_T("%d"), g_sn++);
							thiz->m_listclient.InsertItem(sn, str.AllocSysString());
							thiz->m_listclient.SetItemText(sn++, 1, _T("ESTABLISHED"));
						}
					}
					else
					{
						client = new CUdpClient(thiz->m_info, thiz->m_addr,g_sn, (int)EXAMINATION);
						for (int i = 0; i < EXAMINATION; i++)
						{
							str.Format(_T("%d"), g_sn++);
							thiz->m_listclient.InsertItem(sn, str.AllocSysString());
							thiz->m_listclient.SetItemText(sn++, 1, _T("ESTABLISHED"));
						}
					}
					client->Init();
					thiz->m_clientlist.push(client);
					size -= EXAMINATION;
				}
			}
			else//非考试模式
			{
				count = (size % NORMAL) == 0 ? (size / NORMAL) : ((size / NORMAL) + 1);
				while (count--)
				{
					if (size < NORMAL)
					{
						client = new CUdpClient(thiz->m_info, thiz->m_addr, g_sn, size);
						while (size--)
						{
							str.Format(_T("%d"), g_sn++);
							thiz->m_listclient.InsertItem(sn, str.AllocSysString());
							thiz->m_listclient.SetItemText(sn++, 1, _T("ESTABLISHED"));
						}
						client->Init();
					}
					else
					{
						client = new CUdpClient(thiz->m_info, thiz->m_addr, g_sn, (int)NORMAL);
						for (int i = 0; i < NORMAL; i++)
						{
							str.Format(_T("%d"), g_sn++);
							thiz->m_listclient.InsertItem(sn, str.AllocSysString());
							thiz->m_listclient.SetItemText(sn++, 1, _T("ESTABLISHED"));
						}
						client->Init();
					}
					thiz->m_clientlist.push(client);
					size -= NORMAL;
				}
			}
			thiz->m_status = false;
		}
		else if (thiz->m_exit)
		{
			CString str;
			while (!thiz->m_clientlist.empty())
			{
				if (thiz->m_clientlist.front())
				{
					delete thiz->m_clientlist.front();
					thiz->m_clientlist.front() = nullptr;
					thiz->m_clientlist.pop();
				}
			}
			while (thiz->m_info.m_size--)
			{
				thiz->m_listclient.SetItemText(thiz->m_info.m_size, 1, _T("CLOSED"));
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMy80PressTestDlg 对话框



CMy80PressTestDlg::CMy80PressTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MY80PRESSTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_status = false;
	m_exit = false;
	m_Thread = nullptr;
	m_testing = false;
	ACE::init();
	m_Network.start(false);
	memset(&m_info, 0, sizeof(ModInfo));
	m_info.m_network = &m_Network;
}

int CMy80PressTestDlg::GetInfo()
{
	int status = ((CButton*)GetDlgItem(IDC_Check))->GetCheck();
	m_info.m_isExam = status == 1 ? true : false;

	InitAddres(m_info, m_addr);

	CString str;
	const char* s = nullptr;

	GetDlgItem(IDC_Adress_)->GetWindowText(str); //拉流地址
	s = CString2C(str);
	m_info.m_TargeAddr = s;
	delete s;
	s = nullptr;

	GetDlgItem(IDC_ID)->GetWindowText(str);
	s = CString2C(str);
	g_sn = atoi(s);
	delete s;
	s = nullptr;

	GetDlgItem(IDC_Hart_)->GetWindowText(str);
	s = CString2C(str);
	m_info.m_hart = atoi(s);
	delete s;
	s = nullptr;

	GetDlgItem(IDC_ExamFre)->GetWindowText(str); //拉流地址
	s = CString2C(str);
	m_info.m_examhart = atoi(s);
	delete s;
	s = nullptr;

	GetDlgItem(IDC_stream_count)->GetWindowText(str);
	s = CString2C(str);
	m_info.m_size = atoi(s);
	delete s;
	s = nullptr;

	if (m_info.m_size <= 0)
	{
		MessageBox(_T("请求数量不能为空"));
		return -1;
	}

	if (m_info.m_hart <= 0)
	{
		MessageBox(_T("心跳频率不能为空"));
		return -1;
	}
	m_status = true;
	return 0;
}

void CMy80PressTestDlg::Close()
{
	m_testing = false;
	m_status = false;
	m_exit = true;
	if (m_Thread)
	{
		m_Thread->detach();
		delete m_Thread;
		m_Thread = nullptr;
	}
}

void CMy80PressTestDlg::InitAddres(ModInfo& info, ACE_INET_Addr & addr)
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

CMy80PressTestDlg::~CMy80PressTestDlg()
{
	Close();
	//m_Network.stop();
}

void CMy80PressTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IP, m_ip);
	DDX_Control(pDX, IDC_Port, m_port);
	DWORD   dwIP = inet_addr(IP);
	unsigned   char   *pIP = (unsigned   char*)&dwIP;   // BYTE也可以
	m_ip.SetAddress(pIP[0], pIP[1], pIP[2], pIP[3]);
	m_port.SetWindowText(_T(PORT));
	DDX_Control(pDX, IDC_LIST1, m_listclient);
	GetDlgItem(IDC_Adress_)->SetWindowText(_T(RTMP_ADDR));
	GetDlgItem(IDC_ID)->SetWindowText(_T("0"));
	m_listclient.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES); // 整行选择、网格线
	m_listclient.InsertColumn(0, _T("ID"), LVCFMT_CENTER, 250);
	m_listclient.InsertColumn(1, _T("连接状态"), LVCFMT_CENTER, 404); // 插入第2列的列名
}

BEGIN_MESSAGE_MAP(CMy80PressTestDlg, CDialogEx)
	ON_BN_CLICKED(IDC_Start, &CMy80PressTestDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_Stop2, &CMy80PressTestDlg::OnBnClickedStop2)
	ON_BN_CLICKED(IDC_Exit, &CMy80PressTestDlg::OnBnClickedExit)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CMy80PressTestDlg 消息处理程序

BOOL CMy80PressTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	//m_iWidth = GetSystemMetrics(SM_CXSCREEN); //获取屏幕水平分辨率
	//m_iHeight = GetSystemMetrics(SM_CYSCREEN); //获取屏幕垂直分辨率
	//double role = ((double)m_iHeight / (double)m_iWidth) * 1.5;

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	// TODO: 在此添加额外的初始化代码
	MoveWindow(0, 0, 840, 610);
	return TRUE;
}

void CMy80PressTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMy80PressTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMy80PressTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CMy80PressTestDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{

		return TRUE;
	}
	else
		return CDialogEx::PreTranslateMessage(pMsg);
}

void CMy80PressTestDlg::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_testing && m_exit)return; //正在运行或者线程还未停止
	m_listclient.DeleteAllItems();
	if (GetInfo() != 0)return;
	if (m_Thread)return;
	m_Thread = new std::thread(&ThreadWorker, this);
	m_testing = true;
}

void CMy80PressTestDlg::OnBnClickedStop2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_testing)return;
	Close();
}


void CMy80PressTestDlg::OnBnClickedExit()
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