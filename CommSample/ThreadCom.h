#pragma once
#include <afxwin.h>


class CThreadCom : public CWinThread
{
	DECLARE_DYNCREATE(CThreadCom)
public:
	CThreadCom(HANDLE hCom=INVALID_HANDLE_VALUE);
	virtual ~CThreadCom();
public:
	void SetComStr(CString s);
	CString GetComStr();

	BOOL m_bInit;
	BOOL SendData(BYTE *s, DWORD dwLen);
	BOOL CThreadCom::OpenCom(CString strCom, CWnd *pWndParent, 
		DWORD dwSendMsgToParent, DWORD dwRecvMsgToParent);
	CString m_sError;
	BOOL m_bDone;

	COMMTIMEOUTS m_Commtimeout;
	HANDLE GetComHandle();
	OVERLAPPED m_overWrite;
	OVERLAPPED m_overRead;
	BOOL CloseCom();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();

protected:
	DECLARE_MESSAGE_MAP()
private:
	DCB m_db;
	CString m_sCom;
	DWORD m_dwSendMsgToParent;
	DWORD m_dwRecvMsgToParent;
	CWnd* m_pWndParent;
	HANDLE m_hCom;
};

