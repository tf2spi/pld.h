#ifndef COOL_PLD_H
#define COOL_PLD_H

#ifdef _MSC_VER
#define PLDTYPEOF(x) __typeof__(x)
#else
#define PLDTYPEOF(x) typeof(x)
#endif

static int PLD_Private_strlen(const char *s)
{
	int len = 0;
	while (s[len++])
		;
	return len - 1;
}

static void PLD_Private_Log(const char *, const char *, int);

#ifdef _WIN32

#include <windows.h>

static void PLD_Private_WriteString(const char *s)
{
	DWORD dummy;
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), s, PLD_Private_strlen(s), &dummy, NULL);
}

static HMODULE PLD_Private_LoadLibrary_wrapper(LPCSTR lpLibFileName)
{
	HMODULE hModule = GetModuleHandleA(lpLibFileName);
	if (hModule != NULL)
		hModule = LoadLibraryA(lpLibFileName);
	if (hModule == NULL)
		PLD_Private_Log("Failed to load library!", lpLibFileName, GetLastError());
	return hModule;
}
static FARPROC PLD_Private_GetProcAddress_wrapper(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC lpProc = GetProcAddress(hModule, lpProcName);
	if (lpProc == NULL)
		PLD_Private_Log("Failed to load procedure!", lpProcName, GetLastError());
	return lpProc;
}

#define PLD(name) \
	static HMODULE PLDH(void) { \
		static HMODULE hModule = NULL; \
		if (hModule == NULL) hModule =  PLD_Private_LoadLibrary_wrapper(#name); \
		return hModule; \
	} \

#define PFN(name) ((PLDTYPEOF(name))PLD_Private_GetProcAddress_wrapper(PLDH(), #name))

#else
#error POSIX Unimplemented!
#endif

static void PLD_Private_Log(const char *message, const char *name, int err)
{
	PLD_Private_WriteString(message);
	PLD_Private_WriteString(" (");
	PLD_Private_WriteString(name);
	PLD_Private_WriteString(", err = ");

	char itoastr[8 + 1];
	char *iter = &itoastr[sizeof(itoastr) - 1];
	*iter-- = 0;
	for (int i = 0; i < sizeof(itoastr) - 1; i++) {
		int digit = err & 0xf;
		if (digit >= 9)
			digit = 'A' + (digit - 10);
		else
			digit = '0' + digit;
		*iter-- = digit;
		err >>= 4;
	}
	PLD_Private_WriteString(++iter);
	PLD_Private_WriteString(")\n");
}

#ifndef PLDLOG
#define PLDLOG(message, name, err) PLD_Private_Log(message, name, err)
#endif

#endif // COOL_PLD_H
