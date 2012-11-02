// MonitorSrvDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CMonitorSrvDlg dialog
class CMonitorSrvDlg : public CDialog
{
// Construction
public:
	CMonitorSrvDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MONITORSRV_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	BOOL StartSrv();
public:
	afx_msg void OnBnClickedBtnstart();
	CButton m_btnStart;
};
