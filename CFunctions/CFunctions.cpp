// CFunctions.cpp : Definiuje funkcje biblioteki statycznej.
//

#include "pch.h"
#include "DuFunctions.h"
#include <cstdlib>
#include <malloc.h>
#include <cstdio>
#include <cstdarg>
extern "C"
{
	void* DuAllocate(uint64_t size)
	{
		return malloc(size);
	}

	void* DuReallocate(void* memory, uint64_t size)
	{
		return realloc(memory, size);
	}

	void DuDeallocate(void* memory)
	{
		return free(memory);
	}

	int DuDisplay(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		int ret = vprintf(fmt, args);
		va_end(args);
		return ret;
	}
}