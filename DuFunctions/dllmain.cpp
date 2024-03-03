// dllmain.cpp : Definiuje punkt wejścia dla aplikacji DLL.
#include "pch.h"
#include "DuFunctions.h"
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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
extern "C"
{
    DLLEXPORT int DuDisplay(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        int ret = vprintf(fmt, args);
        va_end(args);
        return ret;
    }
    DLLEXPORT int DuDisplayNumber(int32_t n)
    {
        return printf("\t%d\n", n);
    }
    DLLEXPORT void* DuAllocate(uint64_t size)
    {
        return malloc(size);
    }
    DLLEXPORT void* DuReallocate(uint64_t size, void* memory)
    {
        return realloc(memory, size);
    }
    DLLEXPORT void DuDeallocate(void* memory)
    {
        free(memory);
    }
}