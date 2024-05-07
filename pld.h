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

static void PLD_Private_FormatError(int code, char *s, int len)
{
	DWORD_PTR arg = (DWORD_PTR)s;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		s,
		len,
		(va_list *)&arg);
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
	PLD_Private_WriteString(" ");
	PLD_Private_WriteString(name);
#ifdef _WIN32
	PLD_Private_WriteString("\r\n");
#else
	PLD_Private_WriteString("\n");
#endif
	char errtext[256];
	PLD_Private_FormatError(err, errtext, sizeof(errtext));
	PLD_Private_WriteString(errtext);
}

#ifndef PLDLOG
#define PLDLOG(message, name, err) PLD_Private_Log(message, name, err)
#endif

#endif // COOL_PLD_H
