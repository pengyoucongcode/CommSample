#include "stdafx.h"
#include "ThreadCom.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAXCOMINBUF	1024
#define MAXCOMOUTBUF	1024
IMPLEMENT_DYNCREATE(CThreadCom, CWinThread)

CThreadCom::CThreadCom(HANDLE hCom)
{
	m_hCom = hCom;
	m_bInit = FALSE;
	m_sCom = "";
	m_sError = "No Error!";
	m_hThread = NULL;
	m_dwSendMsgToParent = 0;
	m_dwRecvMsgToParent = 0;
	m_pWndParent = NULL;
	memset((unsigned char*)&m_overRead, 0, sizeof(OVERLAPPED));
	memset((unsigned char*)&m_overWrite, 0, sizeof(OVERLAPPED));
	m_overRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_overWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}


CThreadCom::~CThreadCom()
{
	CloseHandle(m_overRead.hEvent);
	CloseHandle(m_overWrite.hEvent);
}

BOOL CThreadCom::InitInstance()
{
	m_bAutoDelete = FALSE;
	m_bDone = FALSE;
	return TRUE;
}

BOOL CThreadCom::ExitInstance()
{
	BOOL bFlag = CloseCom();
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CThreadCom, CWinThread)
END_MESSAGE_MAP()

int CThreadCom::Run()
{
	DWORD CommMask;
	CommMask = 0
		| EV_BREAK	
		| EV_CTS		
		| EV_DSR		
		| EV_ERR		
		| EV_EVENT1	
		| EV_EVENT2	
		| EV_PERR		
		| EV_RING		
		| EV_RLSD		
		| EV_RX80FULL	
		| EV_RXCHAR	
		| EV_RXFLAG	
		| EV_TXEMPTY;	
	::SetCommMask(m_hCom, CommMask);

	::GetCommTimeouts(m_hCom, &m_Commtimeout);
	m_Commtimeout.ReadTotalTimeoutMultiplier = 5;
	m_Commtimeout.ReadTotalTimeoutConstant = 100;
	DWORD dwError, dwReadNum, dwByteRead, dwEvent;
	COMSTAT ComStat;
	BYTE rBuf[MAXCOMINBUF];
	while (!m_bDone)
	{
		while (m_hCom != INVALID_HANDLE_VALUE)
		{
			if (::WaitCommEvent(m_hCom, &dwEvent, NULL))
			{
				dwByteRead = 0;
				if ((dwEvent & EV_RXCHAR))
				{
					ClearCommError(m_hCom, &dwError, &ComStat);
					if (ComStat.cbInQue != 0)
					{
						dwReadNum = ComStat.cbInQue;
						dwByteRead = 0;
						if (dwReadNum > 200) dwReadNum = 200;
						memset(rBuf, 0, sizeof(rBuf));
						DWORD i = ::ReadFile(m_hCom, rBuf, dwReadNum, &dwByteRead, &m_overRead);
						for (i = dwByteRead; i < 1024; i++) rBuf[i] = 0;
					}
				}
				if (dwByteRead)
					if (m_pWndParent)
						m_pWndParent->SendMessage(m_dwRecvMsgToParent, (DWORD)rBuf, dwByteRead);
			}
		}
		Sleep(0);
	}
	return CWinThread::Run();
}

BOOL CThreadCom::CloseCom()
{
	if (m_hCom != INVALID_HANDLE_VALUE)
	{
		PurgeComm(m_hCom, PURGE_RXCLEAR);
		CloseHandle(m_hCom);
		m_hCom = INVALID_HANDLE_VALUE;
	}
	m_bInit = FALSE;
	return TRUE;
}

HANDLE CThreadCom::GetComHandle()
{
	return m_hCom;
}

BOOL CThreadCom::OpenCom(CString strCom, CWnd *pWndParent,
	DWORD dwSendMsgToParent, DWORD dwRecvMsgToParent)
{
	CloseCom();
	CString strLog;
	m_hCom = ::CreateFile(strCom, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		strLog.Format("Open %s Error", strCom);
		AfxMessageBox(strLog);
		return FALSE;
	}
	::SetupComm(m_hCom, MAXCOMINBUF, MAXCOMOUTBUF);
	DCB dcb;
	if (!GetCommState(m_hCom, &dcb))
	{
		AfxMessageBox("获取串口状态错误！");
		CloseHandle(m_hCom);
		m_hCom = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	if (!SetCommState(m_hCom, &dcb))
	{
		CloseHandle(m_hCom);
		m_hCom = INVALID_HANDLE_VALUE;
		strLog.Format("Set %s CommState Error!", strCom);
		DWORD nError = GetLastError();
		AfxMessageBox(strLog);
		return FALSE;
	}
	m_sError = "No Error";
	m_pWndParent = pWndParent;
	m_dwSendMsgToParent = dwSendMsgToParent;
	m_dwRecvMsgToParent = dwRecvMsgToParent;
	DWORD CommMask;
	CommMask = 0
		| EV_BREAK	
		| EV_CTS		
		| EV_DSR		
		| EV_ERR		
		| EV_EVENT1	
		| EV_EVENT2	
		| EV_PERR		
		| EV_RING		
		| EV_RLSD		
		| EV_RX80FULL	
		| EV_RXCHAR	
		| EV_RXFLAG	
		| EV_TXEMPTY;	
	::SetCommMask(m_hCom, CommMask);
	::GetCommTimeouts(m_hCom, &m_Commtimeout);
	m_Commtimeout.ReadTotalTimeoutMultiplier = 5;
	m_Commtimeout.ReadTotalTimeoutConstant = 100;

	m_bInit = TRUE;
	return TRUE;
}

BOOL CThreadCom::SendData(BYTE *s, DWORD dwLen)
{
	if (!dwLen) return TRUE;
	::GetCommTimeouts(m_hCom, &m_Commtimeout);
	m_Commtimeout.WriteTotalTimeoutMultiplier = 0;
	m_Commtimeout.WriteTotalTimeoutConstant = 2 * dwLen;
	::SetCommTimeouts(m_hCom, &m_Commtimeout);

	if (m_hCom != INVALID_HANDLE_VALUE)
	{
		DWORD dwSend;
		m_pWndParent->SendMessage(m_dwSendMsgToParent, (DWORD)s, dwLen);
		if (!WriteFile(m_hCom, s, dwLen, &dwSend, &m_overWrite))
		{
			DWORD len = GetLastError();
			m_sError = "串口发送数据错误";
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		m_sError = "串口句柄无效";
		return FALSE;
	}
}

CString CThreadCom::GetComStr()
{
	return m_sCom;
}

void CThreadCom::SetComStr(CString s)
{
	m_sCom = s;
}