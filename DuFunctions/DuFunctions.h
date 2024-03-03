
#include "pch.h"
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif
#include <cstdint>

extern "C"
{
	DLLEXPORT int DuDisplay(const char*, ...);
	DLLEXPORT int DuDisplayNumber(int32_t);
	DLLEXPORT void* DuAllocate(uint64_t);
	DLLEXPORT void* DuReallocate(uint64_t, void*);
	DLLEXPORT void DuDeallocate(void*);
}