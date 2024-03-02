#pragma once

#define WIN32_LEAN_AND_MEAN             // Wyklucz rzadko używane rzeczy z nagłówków systemu Windows
#include <cinttypes>
extern "C"
{
	void* DuAllocate(uint64_t size);

	void* DuReallocate(void* memory, uint64_t size);

	void DuDeallocate(void* memory);


	int DuDisplay(const char* fmt, ...);
}