#pragma once
#include <queue>
#include "common.h"
#include <thread>
#include <map>
// CPullStreamDialog 对话框
class CUdpClient;
class CPullStreamDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CPullStreamDialog)

public:
	CPullStreamDialog(INetwork & network,CWnd* pParent = nullptr);   // 标准构造函数
	void Close();
	virtual ~CPullStreamDialog();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_StreamDialog };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 
	void InitAddres(ACE_INET_Addr& addr);
	void SetNumber(int id, void* arg);
	DECLARE_MESSAGE_MAP()
private:
	std::thread*  m_Thread;
public:
	bool m_testing;
	int  m_hart;
	bool m_exit;
	int  m_size;
	bool m_status;
	TestType m_type;
	INetwork*     m_network;               //网络
	ACE_INET_Addr m_addr;                  //目标地址
	std::queue<CUdpClient*> m_clientlist;
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop2();
	CIPAddressCtrl m_ip;
	CEdit m_port;
	afx_msg void OnBnClickedExit();
	CListCtrl m_listclient;
	std::map<LPCTSTR, int> m_maplist;     //id 对应某一行
	std::string m_TargeAddr;              // 拉流地址  
};
