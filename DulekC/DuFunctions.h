#ifdef _WIN32
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLIMPORT
#endif

extern "C"
{
	DLLIMPORT int DuDisplay(const char*, ...);
	DLLIMPORT int DuDisplayNumber(int32_t);
	DLLIMPORT void* DuAllocate(uint64_t);
	DLLIMPORT void* DuReallocate(uint64_t, void*);
	DLLIMPORT void DuDeallocate(void*);
}