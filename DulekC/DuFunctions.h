#ifdef _WIN32
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLIMPORT
#endif

extern "C"
{
	DLLIMPORT int DuDisplay(const char*, ...);
	DLLIMPORT int DuDisplayNumber(int32_t);
	DLLIMPORT uint8_t* DuAllocate(uint64_t);
	DLLIMPORT uint8_t* DuReallocate(uint64_t, uint8_t*);
	DLLIMPORT void DuDeallocate(void*);
}