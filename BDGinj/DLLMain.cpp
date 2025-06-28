#include <Windows.h>
#include <share.h>
#include <vector>
#include <cassert>

//#include "src/objects/union.h"

//#include "include/v8.h"

#include "minhook/MinHook.h"
#include "general.h"

#include "bodog\WebsocketHooks.cpp"  // NOTE: Unity build
#include "bodog\TestHooks.cpp"  // NOTE: Unity build
// #include "Input.h" 
#include "IniParser.h"

#include "hw/HwHook.hpp"


void binitialize() {
   
}


// Structure for thread naming
typedef struct tagTHREADNAME_INFO {
    DWORD dwType;     // Must be 0x1000
    LPCSTR szName;    // Pointer to the thread name (null-terminated string)
    DWORD dwThreadID; // Thread ID (-1 for the current thread)
    DWORD dwFlags;    // Reserved, must be 0
} THREADNAME_INFO;

LONG WINAPI DllExceptionHandler(PEXCEPTION_POINTERS exception)
{
    // TODO: Proper logging and error processing
    // Move to separate class/file

    //char Buffer[1024];
    //snprintf(Buffer, sizeof(Buffer), "=== Exception %x: %x\n",
    //        exception->ExceptionRecord->ExceptionCode,
    //        exception->ExceptionRecord->ExceptionInformation);
    //log(Buffer);


    if (exception->ExceptionRecord->ExceptionCode == 0x406D1388) {
        THREADNAME_INFO* threadInfo = reinterpret_cast<THREADNAME_INFO*>(exception->ExceptionRecord->ExceptionInformation);
        logfff(0, "ThreadID: %d, name: %s\n", threadInfo->dwThreadID, threadInfo->szName);
    }

    if (exception->ExceptionRecord->ExceptionCode == 0xE06D7363) {
        // std::cerr << "C++ Exception Caught!" << std::endl;
        // std::cerr << "Exception Address: 0x" << exception->ExceptionRecord->ExceptionAddress << std::endl;
        logfff(0, "C++ Exception Caught! Address 0x%x\n", exception->ExceptionRecord->ExceptionAddress);

        // // Check if this is a C++ exception
        // if (exception->ExceptionRecord->NumberParameters >= 3 &&
        //         exception->ExceptionRecord->ExceptionInformation[0] == 0x19930520) {
        //     std::cerr << "Thrown Object: 0x" << std::hex << exception->ExceptionRecord->ExceptionInformation[1] << std::endl;
        //     std::cerr << "ThrowInfo: 0x" << exception->ExceptionRecord->ExceptionInformation[2] << std::endl;
        // }
    }

    logfff(0,
        "ExceptionCode: %x\n"
        "ExceptionFlags: %x\n"
        "ExceptionRecord: %x\n"
        "ExceptionAddress: %x\n"
        "NumberParameters: %x\n"
        // "--: %s\n",
        "ExceptionInformation: %x\n\n",

        exception->ExceptionRecord->ExceptionCode,
        exception->ExceptionRecord->ExceptionFlags,
        exception->ExceptionRecord->ExceptionRecord,
        exception->ExceptionRecord->ExceptionAddress,
        exception->ExceptionRecord->NumberParameters,

        // exception->ExceptionRecord->ExceptionInformation,
        exception->ExceptionRecord->ExceptionInformation
    );

    return EXCEPTION_CONTINUE_SEARCH;
}











BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        logfff(0, "DLL_PROCESS_ATTACH \n");

        std::string dllBaseDir = getDllBaseDir();
        std::string iniPath = dllBaseDir + "\\settings.ini";
        std::string dllLog = dllBaseDir + "\\program.log";

        hw::SetProfile(dllBaseDir + "\\Profile.json");

        logfff(0, "DLL base dir: %s\n", dllBaseDir.c_str());
        logfff(0, "settings path: %s\n", iniPath.c_str());
        logfff(0, "log path: %s\n", dllLog.c_str());

        HMODULE hModule = GetModuleHandle(NULL);
        logfff(0, "Module start: %x\n", hModule);

        IniParser config;
        config.load(iniPath);
        // TODO: Verify all required config data and that data is correct.

        // logfff("Server IP: %s\n", config.get("Server", "ServerIp", "").c_str());
        // logfff("Server port: %s\n", config.get("Server", "ServerPort", "").c_str());

        // NOTE: Tempory window. Remove later.
        {
            AllocConsole();
            freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
            freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
        }

        AddVectoredExceptionHandler(TRUE, &DllExceptionHandler);
        
        MH_Initialize();


        createWebsocketHooks(config);
        hw::CreateHooks();
        // createTestHooks();
        
        //testStartInputLoop();

        MH_EnableHook(MH_ALL_HOOKS);

        logfff(0, "---Done attaching\n");

    } break;


    //case DLL_THREAD_ATTACH: { // Do thread-specific initialization.
    //    logfff("DLL_THREAD_ATTACH \n");
    //} break;
    //case DLL_THREAD_DETACH: {// Do thread-specific cleanup.
    //    logfff("DLL_THREAD_DETACH \n");
    //}   break;

    case DLL_PROCESS_DETACH:
    {
        logfff(0, "DLL_PROCESS_DETACH \n");
        if (lpvReserved != nullptr)
        {
            break; // do not do cleanup if process termination scenario
        }
        
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        fclose(logFileGeneral);
    } break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
