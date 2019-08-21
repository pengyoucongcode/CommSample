
// CommSampleDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "CommSample.h"
#include "CommSampleDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_USER_COMSENDMESSAGE	WM_USER+200
#define WM_USER_COMRECVMESSAGE	WM_USER+201


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCommSampleDlg 对话框



CCommSampleDlg::CCommSampleDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_COMMSAMPLE_DIALOG, pParent)
	, m_editSend(_T(""))
	, m_editRecv(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCommSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG, m_editLog);
	DDX_Text(pDX, IDC_EDIT_SEND, m_editSend);
	DDX_Text(pDX, IDC_EDIT_RECV, m_editRecv);
}

BEGIN_MESSAGE_MAP(CCommSampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CCommSampleDlg::OnButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CCommSampleDlg::OnButtonSend)
	ON_MESSAGE(WM_USER_COMSENDMESSAGE, &CCommSampleDlg::OnSendMsg)
	ON_MESSAGE(WM_USER_COMRECVMESSAGE, &CCommSampleDlg::OnRecvMsg)
END_MESSAGE_MAP()


// CCommSampleDlg 消息处理程序

BOOL CCommSampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCommSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCommSampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCommSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD CCommSampleDlg::StopWinThread(CWinThread *pThread, DWORD dwTimeout)
{
	if (pThread == NULL) return NULL;
	pThread->PostThreadMessage(WM_QUIT, 0, 0);
	::WaitForSingleObject(pThread->m_hThread, dwTimeout);
	DWORD nExitCode = 0;
	BOOL bFlag = ::GetExitCodeThread(pThread->m_hThread, &nExitCode);
	if (bFlag)
	{
		delete pThread;
	}
	return nExitCode;
}

// 打开串口
void CCommSampleDlg::OnButtonOpen()
{
	if (pThreadCom != NULL)  return;
	CString str;
	CString com = "COM3";

	pThreadCom = (CThreadCom*)AfxBeginThread(RUNTIME_CLASS(CThreadCom));
	pThreadCom->SetComStr(com);

	/*try
	{
	pThreadCom = (CThreadCom*)AfxBeginThread(RUNTIME_CLASS(CThreadCom));
	pThreadCom->SetComStr(com);
	}
	catch(CException e)
	{
	e.ReportError();
	pThreadCom->m_bDone=TRUE;
	StopWinThread((CWinThread*)pThreadCom,INFINITE);
	pThreadCom=NULL;
	str.Format("创建串口%s线程错误!", com);
	WriteLog(str);
	}   */


	if (pThreadCom->OpenCom(com, (CWnd*)this->GetSafeOwner(),
		WM_USER_COMSENDMESSAGE, WM_USER_COMRECVMESSAGE))
	{
		str.Format("打开串口%s成功", pThreadCom->GetComStr());
		WriteLog(str);
	}
	else
	{
		str.Format(pThreadCom->m_sError + ",请重新配置串口!");
		WriteLog(str);
		pThreadCom->m_bInit = FALSE;
		return;
	}
	m_bCom = TRUE;
	return;
}

void CCommSampleDlg::WriteLog(CString log)
{
	m_editLog.SetWindowText(log);
}

//发送数据通知
LRESULT CCommSampleDlg::OnSendMsg(WPARAM dwEvent, LPARAM dwLen)
{
	if (!dwLen)	return 0;
	BYTE* temp = new BYTE[dwLen + 1];
	memset(temp, 0x00, dwLen + 1);
	memcpy(temp, (const void*)dwEvent, dwLen);
	CString log;
	log.Format("\r\n发送数据=%s", (LPCTSTR)temp);

	if (m_editLog)
	{
		CEdit* editLog = (CEdit*)FromHandle(m_editLog);
		if (editLog->GetWindowTextLength() > 50000)
		{
			editLog->SetSel(0, -1);
			editLog->Clear();
			editLog->SetSel(0, 0);
			editLog->ReplaceSel(log);
		}
		else
		{
			editLog->SetSel(editLog->GetWindowTextLength(), editLog->GetWindowTextLength());
			editLog->ReplaceSel(log);
		}
	}
	return 0;
}

// 接收消息通知
LRESULT CCommSampleDlg::OnRecvMsg(WPARAM dwEvent, LPARAM dwLen)
{
	if (!dwLen)	return 0;
	BYTE* temp = new BYTE[dwLen + 1];
	memset(temp, 0x00, dwLen + 1);
	memcpy(temp, (const void*)dwEvent, dwLen);
	CString log;
	log.Format("\r\n接收数据=%s", (LPCTSTR)temp);

	if (m_editRecv.GetLength() > 50000)	m_editRecv = "";
	m_editRecv += log;
	UpdateData(FALSE);
	return 0;
}


void CCommSampleDlg::OnButtonSend()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	int iLen = m_editSend.GetLength();
	BYTE* s = new BYTE[iLen];
	memset(s, 0x00, iLen);
	memcpy(s, (LPCTSTR)m_editSend, iLen);
	pThreadCom->SendData((unsigned char*)s, iLen);
}
