#include "TaskManager.h"
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")
#include <comdef.h>


CTaskManager::CTaskManager(CClientSocket* pClient) : CManager(pClient)
{
   
    //初始化com库
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hr))
    {
        pClient->Disconnect();
    }
    // 设置安全等级
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
     //实例化计划任务
    hr = CoCreateInstance(CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void**)&pService);

    if (FAILED(hr))
    {
        pClient->Disconnect();
    }

    //  连接计划任务
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
    
    //释放连接的
    pService->Release();
    //卸载com资源
    CoUninitialize();
   
}

void CTaskManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
    
    switch (lpBuffer[0])
    {
        case COMMAND_TASKLIST:          //获取任务
            GetRoot();
            break;
        case COMMAND_TASKCREAT:          //创建任务
            CreateTask(lpBuffer+1);
            break;
        case COMMAND_TASKDEL:          //删除任务
            DelTask(lpBuffer + 1);
            break;
        case COMMAND_TASKSTOP:          //停止任务
            RunOrStopTask(lpBuffer + 1,0);
            break;
        case COMMAND_TASKSTART:          //开始任务
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
    // 动态扩展缓冲区
    if (nBufferSize  < (offset  + sum))
    {
        nBufferSize = nBufferSize + 1024*4;
        lpList = (BYTE*)LocalReAlloc(lpList, nBufferSize, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if (!lpList)
        {
            return;
        }
    }

    //拷贝数据
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
    IRegisteredTaskCollection* pTaskCollection = NULL;   //所有任务集合
    LONG numTasks = 0;                                                  //任务个数

    //获取所有任务
    HRESULT hr = pFolder->GetTasks(TASK_ENUM_HIDDEN, &pTaskCollection);
    pFolder->Release();

    if (FAILED(hr))
    {
        return NULL;
    }

    //获取任务个数
    hr = pTaskCollection->get_Count(&numTasks);
    if (numTasks == 0)
    {
        pTaskCollection->Release();
        return NULL;
    }

    
    //开始遍历任务
    for (LONG i = 0; i < numTasks; i++)
    {
        IRegisteredTask* pRegisteredTask = NULL;
        hr = pTaskCollection->get_Item(_variant_t(i + 1), &pRegisteredTask);            //下标是从1开始的
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
                    status = "未知状态";
                    break;
                case TASK_STATE_DISABLED:
                    status = "已禁用";
                    break;
                case TASK_STATE_QUEUED:
                    status = "排队中";
                    break;
                case TASK_STATE_READY:
                    status = "已准备";
                    break;
                case TASK_STATE_RUNNING:
                    status = "运行中";
                    break;
                default:
                    break;
            }
            //保存数据到buff中
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
    ITaskFolderCollection* pFolders = NULL;         //子文件夹集合
    LONG Count = 0;                                             //子文件夹个数

    HRESULT hr = pFolder->GetFolders(0, &pFolders);
    if (FAILED(hr))
    {
        return 0;
    }

    //获取文件夹个数
    hr = pFolders->get_Count(&Count);
    if (FAILED(hr))
    {
        return 0;
    }

    if (Count == 0)
    {
        pFolders->Release();
        //获取文件夹下的任务
        GetTaskAll(pFolder);
    }
    else
    {
        //先遍历目录下的任务
        GetTaskAll(pFolder);

        //递归遍历文件夹
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
    //先分10kb 大小
    nBufferSize = 1024 * 10; // 先分配10K的缓冲区
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

//创建
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

    //选择计划任务注册在哪个文件夹下
    HRESULT hr =  pService->GetFolder(_bstr_t(rootpath), &pRootFolder);
    if (SUCCEEDED(hr))
    {
        //创建任务定义对象
        hr = pService->NewTask(0,&pDefinition);
        if (SUCCEEDED(hr))
        {
            //获取注册信息的实例用来填写注册信息
            hr = pDefinition->get_RegistrationInfo(&pRegistrationInfo);
            //获取设置主体信息的实例
            HRESULT hr2 = pDefinition->get_Principal(&pPrincipal);
            //设置任务相关信息
            HRESULT hr3 = pDefinition->get_Settings(&pSettings);
            //设置触发器
            HRESULT hr4 = pDefinition->get_Triggers(&pTriggers);
            //设置执行操作
            HRESULT hr5 = pDefinition->get_Actions(&pActions);

            if (SUCCEEDED(hr) && SUCCEEDED(hr2) && SUCCEEDED(hr3) && SUCCEEDED(hr4) && SUCCEEDED(hr5))
            {
                 //作者
                 pRegistrationInfo->put_Author(_bstr_t(Author));
                 //描述
                 pRegistrationInfo->put_Description(_bstr_t(Description));
                  //  设置登陆类型 (用户必须已经登录。 该任务将仅在现有的交互式会话中运行。 )
                  pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
                    // 设置运行权限 (任务将以最高权限运行。 )
                  pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
                  //设置被结束后自动延迟10分钟运行
                  pSettings->put_StartWhenAvailable(VARIANT_TRUE);
                  //创建触发器
                  ITrigger* pTrigger;
                  hr = pTriggers->Create(TASK_TRIGGER_LOGON, &pTrigger);

                  IAction* pAction;
                  //设置执行
                  pActions->Create(TASK_ACTION_EXEC, &pAction);
                  pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
                  //设置程序路径等信息
                  pExecAction->put_Path(_bstr_t(path));
                  //注册
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
//执行 或 停止
BOOL CTaskManager::RunOrStopTask(LPBYTE lpBuffer,BOOL Action)
{
    char* rootpath = (char*)lpBuffer;
    char* taskname = rootpath + strlen(rootpath) + 1;


    BOOL ret = FALSE;
    ITaskFolder* pRootFolder = NULL;
    IRegisteredTask* ppTask = NULL;
    VARIANT params = {0};
    //选择计划任务注册在哪个文件夹下
    HRESULT hr = pService->GetFolder(_bstr_t(rootpath), &pRootFolder);
    if (SUCCEEDED(hr))
    {
        hr = pRootFolder->GetTask(_bstr_t(taskname),&ppTask);
        if (SUCCEEDED(hr))
        {
            if (Action)  //执行
            {
                hr = ppTask->Run(params, NULL);
                if (SUCCEEDED(hr))
                {
                    ret = TRUE;

                }
            }
            else  //停止
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

//删除
BOOL CTaskManager::DelTask(LPBYTE lpBuffer)
{


    char* rootpath = (char*)lpBuffer;
    char* taskname = rootpath + strlen(rootpath) + 1;

    BOOL ret = FALSE;
    ITaskFolder* pRootFolder = NULL;
    //选择计划任务注册在哪个文件夹下
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

