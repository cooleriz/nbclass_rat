#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 的一些常见且经常可放心忽略的隐藏警告消息
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC 支持功能区和控制条

#define LIST_IMG_INDEX_UP 18
#define LIST_IMG_INDEX_DWON 31
#define LIST_IMG_INDEX_ERROR 27
#define LIST_IMG_INDEX_SUCCESS 41
#define LIST_IMG_INDEX_INIT 43

#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))
typedef struct
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILESIZE;



//连接视图列表索引
enum
{
	CONNECT_WIP = 0,			//外网ip
	CONNECT_NIP,					//内网ip
	CONNECT_OS,					//系统
	CONNECT_NAME,				//主机名
	CONNECT_USER,				//主机名
	CONNECT_GIS ,				//位置
	CONNECT_CPU	,				//cpu
	CONNECT_RAM,				//运行内存
	CONNECT_HD ,					//硬盘
	CONNECT_PING,				//延迟
	CONNECT_VIDEO ,			//视频
	CONNECT_BZ,					//备注
	CONNECT_INS,					//备注
	CONNECT_AV,				//杀毒
	CONNECT_CONTEXT		//结构
};

#include<assert.h>
// 高精度的睡眠函数
#define Sleep_m(ms) { timeBeginPeriod(1); Sleep(ms); timeEndPeriod(1); }

// 以步长n毫秒在条件C下等待T秒(n是步长，必须能整除1000)
#define WAIT_n(C, T, n) {assert(!(1000%(n)));int s=(1000*(T))/(n);do{Sleep(n);}while((C)&&(--s));}

// 在条件C成立时等待T秒(步长10ms)
#define WAIT(C, T) { timeBeginPeriod(1); WAIT_n(C, T, 10); timeEndPeriod(1); }

// 在条件C成立时等待T秒(步长1ms)
#define WAIT_1(C, T) { timeBeginPeriod(1); WAIT_n(C, T, 1); timeEndPeriod(1); }

//消息列表
enum
{
	WM_CLIENT_CONNECT = WM_APP + 0x1001,
	WM_CLIENT_CLOSE,
	WM_CLIENT_NOTIFY,
	WM_DATA_IN_MSG,
	WM_DATA_OUT_MSG,
	WM_ADDTOLIST = WM_USER + 102,	// 添加到列表视图中
	WM_ADDFINDGROUP,
	WM_REMOVEFROMLIST,				// 从列表视图中删除
	WM_OPENMANAGERDIALOG,			// 打开一个文件管理窗口
	WM_OPENSCREENSPYDIALOG,			// 打开一个屏幕监视窗口
	WM_OPENOLDSCREENSPYDIALOG,		//打开老屏幕
	WM_OPENDESKTOPSPYDIALOG,		//打开后台桌面窗口
	WM_OPENWEBCAMDIALOG,			// 打开摄像头监视窗口
	WM_OPENAUDIODIALOG,				// 打开一个语音监听窗口
	WM_OPENSPEAKERDIALOG,				//打开扬声器
	WM_OPENKEYBOARDDIALOG,			// 打开键盘记录窗口
	WM_OPENPSLISTDIALOG,			// 打开进程管理窗口
	WM_OPENPSERLISTDIALOG,		//打开服务管理窗口
	WM_OPENSHELLDIALOG,				// 打开shell窗口
	WM_OPENREGEDITDIALOG,			//打开注册表
	WM_RESETPORT,					// 改变端口
	WM_OPENSYSINFODIALOG,
	WM_OPENTASKDIALOG,
	WM_TOOLSCLASS,
	WM_NOTIFY_MAIN,								//托盘消息
//////////////////////////////////////////////////////////////////////////
FILEMANAGER_DLG = 1,
SCREENSPY_DLG,
DESKTOP_DLG,
WEBCAM_DLG,
AUDIO_DLG,
SPEAKER_DLG,
KEYBOARD_DLG,
SYSTEM_DLG,
SHELL_DLG,
TASK_DLG,
SYSINFO_DLG,
REGEDIT_DLG,
SERMANAGER_DLG,
PROXY_DLG,
CHAT_DLG,
DLL_DLG,
TOOLS_DLG,
OLDSCREENSPY_DLG
};

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


