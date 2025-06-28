#pragma once
#include <Windows.h>
#include <psapi.h>


int _GetModuleBase(HANDLE processHandle, char* sModuleName);