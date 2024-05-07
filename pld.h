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

#ifndef PLDLOG
#define PLDLOG(message, name, err) PLD_Private_Log(message, name, err)
#endif

#ifdef _WIN32

#include <windows.h>
typedef DWORD PLD_ErrCode;
static void PLD_Private_Log(const char *, const char *, PLD_ErrCode);

static void PLD_Private_WriteString(const char *s)
{
	DWORD dummy;
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), s, PLD_Private_strlen(s), &dummy, NULL);
}

static void PLD_Private_FormatError(DWORD code, char *s, int len)
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
	if (hModule == NULL)
		hModule = LoadLibraryA(lpLibFileName);
	if (hModule == NULL)
		PLDLOG("Failed to load library!", lpLibFileName, GetLastError());
	return hModule;
}

static FARPROC PLD_Private_GetProcAddress_wrapper(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC lpProc = GetProcAddress(hModule, lpProcName);
	if (lpProc == NULL)
		PLDLOG("Failed to load procedure!", lpProcName, GetLastError());
	return lpProc;
}

#define PLD(name) \
	static HMODULE PLDH(void) { \
		static HMODULE hModule = NULL; \
		if (hModule == NULL) hModule =  PLD_Private_LoadLibrary_wrapper(name); \
		return hModule; \
	} \

#define PFN(name) ((PLDTYPEOF(&name))PLD_Private_GetProcAddress_wrapper(PLDH(), #name))

#else

#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>

typedef intptr_t PLD_ErrCode;
static void PLD_Private_Log(const char *, const char *, PLD_ErrCode);

static void PLD_Private_WriteString(const char *s)
{
	write(2, s, PLD_Private_strlen(s));
}

static void PLD_Private_FormatError(intptr_t code, char *s, int len)
{
	*s = 0;
	strncat(s, (const char *)code, len);
}

static void *PLD_Private_dlopen_wrapper(const char *name)
{
	if (name == NULL)
		return (void *)RTLD_NEXT;
	void *handle = dlopen(name, RTLD_NOLOAD);
	if (handle == NULL)
		handle = dlopen(name, RTLD_LAZY);
	if (handle == NULL)
		PLDLOG("Failed to load library!", name, (intptr_t)dlerror());
	return handle;
}

static void *PLD_Private_dlsym_wrapper(void *handle, const char *name)
{
	void *sym = dlsym(handle, name);
	if (sym == NULL)
		PLDLOG("Failed to load procedure!", name, (intptr_t)dlerror());
	return sym;
}

#define PLD(name) \
	static void *PLDH(void) { \
		static void *handle = NULL; \
		if (handle == NULL) handle = PLD_Private_dlopen_wrapper(name); \
		return handle; \
	} \

#define PFN(name) ((PLDTYPEOF(&name))PLD_Private_dlsym_wrapper(PLDH(), #name))

#endif

static void PLD_Private_Log(const char *message, const char *name, PLD_ErrCode err)
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

	// An extra newline because dlerror doesn't append one
#ifndef _WIN32
	PLD_Private_WriteString("\n");
#endif
}

#endif // COOL_PLD_H
