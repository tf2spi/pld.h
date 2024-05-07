#ifndef COOL_PLD_H
#define COOL_PLD_H

#ifdef _MSC_VER
#define PLDTYPEOF(x) __typeof__(x)
#else
#define PLDTYPEOF(x) typeof(x)
#endif

#ifdef _WIN32
#include <windows.h>
static HMODULE PLD_LoadLibrary_wrapper(LPCSTR lpLibFileName)
{
	HMODULE hModule = GetModuleHandleA(lpLibFileName);
	if (hModule != NULL)
		hModule = LoadLibraryA(lpLibFileName);
	return hModule;
}
static FARPROC PLD_GetProcAddress_wrapper(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC lpProc = GetProcAddress(hModule, lpProcName);
	return lpProc;
}

#define PLD(name) \
	static HMODULE PLDH(void) { \
		static HMODULE hModule = NULL; \
		if (hModule == NULL) hModule =  PLD_LoadLibrary_wrapper(#name); \
		return hModule; \
	} \

#define PFN(name) ((PLDTYPEOF(name))PLD_GetProcAddress_wrapper(PLDH(), #name))

#else
#error POSIX Unimplemented!
#endif

#endif // COOL_PLD_H
