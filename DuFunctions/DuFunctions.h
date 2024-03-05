
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
	DLLEXPORT uint8_t* DuAllocate(uint64_t);
	DLLEXPORT uint8_t* DuReallocate(uint64_t, uint8_t*);
	DLLEXPORT void DuDeallocate(uint8_t*);
}