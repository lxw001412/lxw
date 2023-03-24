
// 80-PressTestDlg.h: 头文件
//
#include <queue>
#include "common.h"
#include <thread>
#include <map>
#include <queue>
#include "ReactorSvr.h"
#pragma once

struct ModInfo;
class CUdpClient;
class CPullStreamDialog;

// CMy80PressTestDlg 对话框
class CMy80PressTestDlg : public CDialogEx
{
// 构造
public:
	CMy80PressTestDlg(CWnd* pParent = nullptr);	// 标准构造函数
	int GetInfo();
	void Close();
	void InitAddres(ModInfo& info, ACE_INET_Addr & addr);
	~CMy80PressTestDlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MainDialog };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
// 实现
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	std::thread*  m_Thread;
public:
	bool    m_exit;
	bool    m_status;
	bool    m_testing;
	ReactorSvr m_Network;
	ModInfo m_info;
	CEdit   m_port;
	ACE_INET_Addr m_addr;                  //目标地址
	CIPAddressCtrl m_ip;
	CListCtrl      m_listclient;
	std::map<LPCTSTR, int> m_maplist;
	std::queue<CUdpClient*> m_clientlist;
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop2();
	afx_msg void OnBnClickedExit();

};
