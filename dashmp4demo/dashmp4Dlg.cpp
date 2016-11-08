// dashmp4Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "dashmp4.h"
#include "dashmp4Dlg.h"

#include "dashmp4lib.h"

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
SOCKET g_socket = INVALID_SOCKET;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDashmp4Dlg dialog

CDashmp4Dlg::CDashmp4Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDashmp4Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDashmp4Dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDashmp4Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDashmp4Dlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDashmp4Dlg, CDialog)
	//{{AFX_MSG_MAP(CDashmp4Dlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDashmp4Dlg message handlers

BOOL CDashmp4Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	{
		
		WORD wVersionRequested;  
		WSADATA wsaData;  
		int ret;  
		SOCKET sClient; //连接套接字  
		struct sockaddr_in saServer; //服务器地址信息  		
		BOOL fSuccess = TRUE;  
		
		//WinSock初始化  
		wVersionRequested = MAKEWORD(2, 2); //希望使用的WinSock DLL的版本  
		ret = WSAStartup(wVersionRequested, &wsaData);  //加载套接字库  
		if(ret!=0)  
		{  
			AfxMessageBox("WSAStartup() failed!\n");  
			//return 0;  
		}  
		//确认WinSock DLL支持版本2.2  
		if(LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=2)  
		{  
			WSACleanup();   //释放为该程序分配的资源，终止对winsock动态库的使用  
			AfxMessageBox("Invalid WinSock version!\n");  
			//return 0;  
		}  
		
		//创建Socket,使用TCP协议  
		g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
		if (g_socket == INVALID_SOCKET)  
		{  
			WSACleanup();  
			AfxMessageBox("socket() failed!\n");  
			//return 0;  
		}  
		
		//构建服务器地址信息  
		saServer.sin_family = AF_INET; //地址家族  
		saServer.sin_port = htons(8888); //注意转化为网络节序  
		saServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  
		
		//连接服务器  
		ret = connect(g_socket, (struct sockaddr *)&saServer, sizeof(saServer));  
		if (ret == SOCKET_ERROR)  
		{  
			AfxMessageBox("connect() failed!\n");  
			closesocket(sClient); //关闭套接字  
			WSACleanup();  
			//return 0;  
		}  
		
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDashmp4Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDashmp4Dlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDashmp4Dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}




void CDashmp4Dlg::OnOK() 
{
	u32 dwNowKeyFrames = 0;
	u32 dwNeededGops = 1;
	vector <H264Frame> vecFrames;
	
	
	FILE* pFile = fopen("./test.h264", "rb");
	if (!pFile)
	{
		AfxMessageBox("read file error");
		return;
	}
	
	u32 dwTime = 0;
	u32 dwLen = 0;	
	u32 dwSeqquenceNum = 0;
	while (1)
	{
		Sleep(40);
		int nRet = fread(&dwLen, 1, 4, pFile);
		if (4 != nRet) 
		{
			break;
		}
		u8* buffer = new u8[dwLen];
		nRet = fread(buffer, 1, dwLen, pFile);
		if (nRet != dwLen)
		{
			break;	
		}
		
		H264Frame tFrame(buffer, dwLen, dwTime);
		tFrame.dwTime = dwTime;
		dwTime += 40 * 90;
		{					
			if (0 == dwNowKeyFrames) 
			{
				if (tFrame.IsVideo() && !tFrame.IsKeyFrame())
				{
					// continue just drop it
					continue;
				}
			}
			
			if (tFrame.IsKeyFrame())
			{
				dwNowKeyFrames++;
				if (dwNowKeyFrames == dwNeededGops+1)
				{
					// flush all the frames
					// ......
					dwSeqquenceNum += 1;
					Mp4 mp4file(vecFrames, dwSeqquenceNum);
					u32 nFileSize = mp4file.GetMp4FileTotalSize();
					u8* pFileBuf = new u8[nFileSize];
					mp4file.WriteMp4ToBuffer(pFileBuf, nFileSize);

					/*CString strPathmp4;
					strPathmp4.Format("d:\\file\\%d.mp4", dwSeqquenceNum);
					FILE* pMp4 = fopen(strPathmp4, "wb");
					if (pMp4)
					{
						fwrite(pFileBuf, 1, nFileSize, pMp4);
						fclose(pMp4);						
					}*/
					//send to socket

					if (INVALID_SOCKET != g_socket) 
					{
						::send(g_socket, (const char*)(&nFileSize), 4, 0);
						::send(g_socket, (const char*)pFileBuf, nFileSize, 0);
					}

					delete [] pFileBuf;
					
					dwNowKeyFrames -= dwNeededGops;
					for(int inx = 0; inx < vecFrames.size(); inx++)
					{
						if (vecFrames[inx].tBuf.pBuffer)
						{
							delete [] vecFrames[inx].tBuf.pBuffer;
							vecFrames[inx].tBuf.pBuffer = NULL;
						}
					}
					vecFrames.clear();
				}												
			}
			
		}		
		vecFrames.push_back(tFrame);
	}
	
	fclose(pFile);	
}
