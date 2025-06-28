#include <iostream>
#include <string>
#include <vector>

#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <comdef.h>
#include <WbemCli.h>
#include <span>

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include "Test.h"

#include "Utils.hpp"

//#include <filesystem>
//namespace fs = std::filesystem;


#include "nlohmann/json.hpp"


// NOTE: 'NtQueryInformationProcess' is internal command that is not 
// part of public API. It is documented but could be changed any time in
// the future (per documentation). There are other ways to achieve getting
// information about process command line but this is most straightforward
// and fastest. Later maybe make a backup function to so that could be
// easily switched just in case if API breaks. Next solution could be either
// using WMI or just hooking dll to a process and just getting diret access to
// memory location.
typedef  NTSTATUS (NTAPI * NtQueryInformationProcessProc)(
    HANDLE           ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID            ProcessInformation,
    ULONG            ProcessInformationLength,
    PULONG           ReturnLength
);
NtQueryInformationProcessProc QueryProcessInfo;


bool isOurExeToEmbed(const PROCESSENTRY32W& entry, const wchar_t* exeName, const wchar_t* filter) {

    if (std::wstring(entry.szExeFile) != exeName) {
        return 0;
    }

    HANDLE TokenHandle;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle);

    LUID  Luid;
    TOKEN_PRIVILEGES oldPriv;
    TOKEN_PRIVILEGES priv;
    LookupPrivilegeValueA(0, "SeDebugPrivilege", &priv.Privileges[0].Luid);

    unsigned long retLength = 0;
    priv.PrivilegeCount = 1;
    priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    oldPriv = priv;
    AdjustTokenPrivileges(TokenHandle, false, &priv, sizeof(oldPriv), &oldPriv, &retLength);

    HANDLE handle = OpenProcess(
        PROCESS_ALL_ACCESS,
        FALSE,
        entry.th32ProcessID
    );
    if (handle == 0) {
        wprintf(L"Couldn't get handle for the process [%d]. Error code: 0x%x, %s\n", entry.th32ProcessID, GetLastError(), entry.szExeFile);
        return 0;
        //exit(14); // TODO: Proper error handling
    }
    
    PROCESS_BASIC_INFORMATION basicInfo = {};
    unsigned long processInformationLength = sizeof(basicInfo);
    unsigned long returnLength = 0;
    PROCESSINFOCLASS processInfoClass = ProcessBasicInformation;
    
    NTSTATUS status = QueryProcessInfo(
        handle,
        (PROCESSINFOCLASS)0,
        (void*)&basicInfo,
        processInformationLength,
        &returnLength
    );

    PPEB ppeb = (PPEB)((PVOID*)&basicInfo)[1];
    PPEB ppebCopy = (PPEB)malloc(sizeof(PEB));
    BOOL result = ReadProcessMemory(handle,
        ppeb,
        ppebCopy,
        sizeof(PEB),
        NULL);

    PRTL_USER_PROCESS_PARAMETERS pRtlProcParam = ppebCopy->ProcessParameters;
    PRTL_USER_PROCESS_PARAMETERS pRtlProcParamCopy =
        (PRTL_USER_PROCESS_PARAMETERS)malloc(sizeof(RTL_USER_PROCESS_PARAMETERS));
    result = ReadProcessMemory(handle,
        pRtlProcParam,
        pRtlProcParamCopy,
        sizeof(RTL_USER_PROCESS_PARAMETERS),
        NULL);

    PWSTR wBuffer = pRtlProcParamCopy->CommandLine.Buffer;
    USHORT len = pRtlProcParamCopy->CommandLine.Length;
    PWSTR wCommandLineBufferCopy = (PWSTR)malloc(len);
    result = ReadProcessMemory(handle,
        wBuffer,
        wCommandLineBufferCopy, // command line goes here
        len,
        NULL);

    if (filter != NULL) {
        // wprintf(L"===: %s\n", wCommandLineBufferCopy);
        if (wcsstr(wCommandLineBufferCopy, filter) != NULL) {
            wprintf(L"Found '%s' [%d]: % s\n", exeName, entry.th32ProcessID, wCommandLineBufferCopy);
            return 1;
        } else {
            // printf("-----\n");
        }
    }
   
    // TODO: Cleanup

    return 0; // Didn't find
}

BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam) {
    const auto& pids = *reinterpret_cast<std::vector<DWORD>*>(lParam);

    DWORD winId;
    GetWindowThreadProcessId(hwnd, &winId);

    //for (DWORD pid : pids) {
    //    if (winId == pid) {
    //        std::wstring title(GetWindowTextLength(hwnd) + 1, L'\0');
    //        GetWindowTextW(hwnd, &title[0], title.size()); //note: C++11 only
    //        std::cout << "Found window:\n";
    //        std::cout << "Process ID: " << pid << '\n';
    //        std::wcout << "Title: " << title << "\n\n";
    //    }
    //}

    return TRUE;
}

int startupV1(const wchar_t* processNameFilter) {
    std::vector<DWORD> pids;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    auto cleanupSnap = [snap] { CloseHandle(snap); };

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof entry;

    if (!Process32FirstW(snap, &entry)) {
        cleanupSnap();
        return 0;
    }

    do {
        // NOTE: Currenly most communication is done over process with "type" of 
        // network.mojom.NetworkService there is a little networking on main top
        // process but it is not releavant for now. But in the future we can also
        // attach to it if needed
        //int shouldAttach = isOurExeToEmbed(entry, L"Lobby.exe", L"network.mojom.NetworkService");
        int shouldAttach = isOurExeToEmbed(entry, L"Lobby.exe", processNameFilter);
        if (shouldAttach) {
            pids.emplace_back(entry.th32ProcessID);
        }
    } while (Process32NextW(snap, &entry));
    cleanupSnap();

    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&pids));

    if (pids.size() > 0) {
        return pids[0];
    }
    else {
        return 0;
    }
}

void injectDllIntoRemoteProcess(uint32_t processId, const char* dllPath) {

    // handle to kernel32 and pass it to GetProcAddress
    HMODULE hKernel32 = GetModuleHandleA("Kernel32");
    VOID* lb = GetProcAddress(hKernel32, "LoadLibraryA");

    int dllPathLength = strlen(dllPath);

    printf("PID: %i\n", processId);

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DWORD(processId));

    LPVOID remoteBuffer = VirtualAllocEx(processHandle, NULL, dllPathLength, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);

    bool didWriteMemory = WriteProcessMemory(processHandle, remoteBuffer, dllPath, dllPathLength, NULL);
    printf("DidWriteMemory: %d\n", didWriteMemory);

    HANDLE remoteThread = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)lb, remoteBuffer, 0, NULL);
    printf("RemoteThread: %d\n", remoteThread);
    CloseHandle(processHandle);
}



int main() {
    // "C:\Program Files (x86)\Bodog Poker\Lobby.exe" launcher='launcher' --storageFolder "C:\Program Files (x86)\Bodog Poker"

    HMODULE ntdll = LoadLibraryA("ntdll.dll");
    if (ntdll) {
        QueryProcessInfo = (NtQueryInformationProcessProc)GetProcAddress(ntdll, "NtQueryInformationProcess");
    }
    if (QueryProcessInfo == 0) {
        printf("Couldn't find NtQueryInformationProcess in winternl\n");
        exit(13);
    }

    char dllpath[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, dllpath, MAX_PATH);
    PathRemoveFileSpecA(dllpath);
    PathAppendA(dllpath, "BDGinj.dll"); // TODO: Move dll out.

    if (!PathFileExistsA(dllpath))
    {
        printf("File %s does not exist\n", dllpath);
        exit(13); // TODO: Proper error processing
    }

    static int attempt = 0;

    while (true)
    {
        printf("\rTrying to find Bodog process to attach: %d", attempt++);

        // Atempts to find the launcher process
        if (const int pid = startupV1(L"launcher='launcher'"); pid > 0)
        {
            injectDllIntoRemoteProcess(pid, dllpath);
            printf("Injected %s into pid: %d (Launcher)\n", dllpath, pid);
        }

        // Attempt to find the networking process
        if (const int pid = startupV1(L"network.mojom.NetworkService"); pid > 0)
        {
            injectDllIntoRemoteProcess(pid, dllpath);
            printf("Injected %s into pid: %d (Network)\n", dllpath, pid);

            break;
        }

        Sleep(10);
    }
} 
