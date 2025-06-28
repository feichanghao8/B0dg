#include "Utils.hpp"

int _GetModuleBase(HANDLE processHandle, char* sModuleName)
{
    HMODULE* hModules = NULL;
    char szBuf[50];
    DWORD cModules;
    DWORD dwBase = -1;

    EnumProcessModules(processHandle, hModules, 0, &cModules);
    hModules = new HMODULE[cModules / sizeof(HMODULE)];

    if (EnumProcessModules(processHandle, hModules, cModules / sizeof(HMODULE), &cModules)) {
        for (size_t i = 0; i < cModules / sizeof(HMODULE); i++) {
            if (GetModuleBaseNameA(processHandle, hModules[i], szBuf, sizeof(szBuf))) {
                //if (sModuleName.compare(szBuf) == 0) {
                if (strcmp(sModuleName, szBuf) == 0) {
                    dwBase = (DWORD)hModules[i];
                    break;
                }
            }
        }
    }

    delete[] hModules;
    return dwBase;
}