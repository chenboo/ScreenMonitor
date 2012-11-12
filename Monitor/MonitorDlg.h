// MonitorDlg.h : header file
//

#pragma once
#include "iocp/IOCPServer.h"

// CMonitorDlg dialog
class CMonitorDlg : public CDialog
{
// Construction
public:
	CMonitorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MONITOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg	LRESULT OnOpenScreenSpyDialog(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

public:
	void Activate(UINT nPort, UINT nMaxConnections);

	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode);
	static void ProcessReceive(ClientContext *pContext);
	static void ProcessReceiveComplete(ClientContext *pContext);
	
	afx_msg void OnBnClickedBtnmonitor();
};
