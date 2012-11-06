// MonitorSrvDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MonitorSrv.h"
#include "MonitorSrvDlg.h"
#include "ClientSocket.h"
#include "KernelManager.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMonitorSrvDlg dialog




CMonitorSrvDlg::CMonitorSrvDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMonitorSrvDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMonitorSrvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTNSTART, m_btnStart);
}

BEGIN_MESSAGE_MAP(CMonitorSrvDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTNSTART, &CMonitorSrvDlg::OnBnClickedBtnstart)
END_MESSAGE_MAP()


// CMonitorSrvDlg message handlers

BOOL CMonitorSrvDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMonitorSrvDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMonitorSrvDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CMonitorSrvDlg::StartSrv()
{
	char* lpszHost = "127.0.0.1";
	DWORD dwPort = 8077;

	CClientSocket socketClient;
	if (!socketClient.Connect(lpszHost, dwPort))
		AfxMessageBox(_T("Connect Fail!"));
	
	DWORD dwExitCode = SOCKET_ERROR;
	
	CKernelManager	manager(&socketClient, NULL, 0, NULL, lpszHost, dwPort);
	//socketClient.setManagerCallBack(&manager);

	DWORD	dwIOCPEvent;
	do
	{
		dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
		Sleep(500);
	} while(dwIOCPEvent != WAIT_OBJECT_0);

	return TRUE;
}
void CMonitorSrvDlg::OnBnClickedBtnstart()
{
	m_btnStart.EnableWindow(FALSE);

	StartSrv();
}
