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

// Private dlopen function has platform-specific behavior when name == NULL
// * Windows: Return GetModuleHandleA(NULL)
// * POSIX: Return RTLD_NEXT
//
// Private dlsym function has platform-specific behavior when handle == NULL
// * Windows: Use GetModuleHandleA(NULL) instead
// * POSIX: Use RTLD_NEXT instead
#ifdef _WIN32

#include <windows.h>
typedef FARPROC PLD_ProcAddress;
typedef HMODULE PLD_LibHandle;
typedef DWORD PLD_ErrCode;
static void PLD_Private_Log(const char *, const char *, PLD_ErrCode);

static PLD_ErrCode PLD_Private_ErrCode(void)
{
	return GetLastError();
}

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

static PLD_LibHandle PLD_Private_dlopen(const char *name)
{
	PLD_LibHandle handle = GetModuleHandleA(name);
	if (handle == NULL)
		handle = LoadLibraryA(name);
	return handle;
}

static PLD_ProcAddress PLD_Private_dlsym(PLD_LibHandle handle, const char *name)
{
	if (handle == NULL)
		handle = GetModuleHandleA(NULL);
	return GetProcAddress(handle, name);
}

#else

#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>

typedef void *PLD_ProcAddress;
typedef void *PLD_LibHandle;
typedef intptr_t PLD_ErrCode;
static void PLD_Private_Log(const char *, const char *, PLD_ErrCode);

static PLD_ErrCode PLD_Private_ErrCode(void)
{
	return (intptr_t)dlerror();
}

static void PLD_Private_WriteString(const char *s)
{
	write(2, s, PLD_Private_strlen(s));
}

static void PLD_Private_FormatError(intptr_t code, char *s, int len)
{
	*s = 0;
	strncat(s, (const char *)code, len);
}

static PLD_LibHandle PLD_Private_dlopen(const char *name)
{
	if (name == NULL)
		return (PLD_LibHandle)RTLD_NEXT;
	void *handle = dlopen(name, RTLD_NOLOAD);
	if (handle == NULL)
		handle = dlopen(name, RTLD_LAZY);
	return handle;
}

static PLD_ProcAddress PLD_Private_dlsym(PLD_LibHandle handle, const char *name)
{
	if (handle == NULL)
		handle = (PLD_LibHandle)RTLD_NEXT;
	return dlsym(handle, name);
}


#endif

static PLD_ProcAddress PLD_Private_dlsym_wrapper(PLD_LibHandle handle, const char *name)
{
	if (name == NULL) {
		PLDLOG("NULL name provided for procedure!", "???", 0);
		return NULL;
	}
	if (handle == NULL)
		handle = PLD_Private_dlopen(NULL);
	PLD_ProcAddress sym = PLD_Private_dlsym(handle, name);
	if (sym == NULL)
		PLDLOG("Failed to load procedure!", name, PLD_Private_ErrCode());
	return sym;
}

static PLD_LibHandle PLD_Private_dlopen_wrapper(const char *name)
{
	// On Linux, RTLD_NEXT is NULL, so check if the
	// name is NULL before concluding it's an error
	PLD_LibHandle handle = PLD_Private_dlopen(name);
	if (handle == NULL && name != NULL)
		PLDLOG("Failed to load library!", name, PLD_Private_ErrCode());
	return handle;
}


#define PLD(name) \
	PLD_LibHandle PLDH(void) { \
		static PLD_LibHandle handle = NULL; \
		if (handle == NULL) handle = PLD_Private_dlopen_wrapper(name); \
		return handle; \
	} \

// For importing by ordinal on Windows or just using
// another name in a library for whatever reason
#define PFNALT(name, alt) ((PLDTYPEOF(&name))PLD_Private_dlsym_wrapper(PLDH(), alt))
#define PFN(name) PFNALT(name, #name)

// When code is 0, output nothing
static void PLD_Private_FormatError_wrapper(intptr_t code, char *s, int len) {
	// Add an extra newline only on Windows to mimic FormatMessageA
	if (code == 0) {
#ifdef _WIN32
		s[0] = '\r';
		s[1] = '\n';
		s[2] = 0;
#else
		*s = 0;
#endif
		return;
	}
	PLD_Private_FormatError(code, s, len);
}


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
	PLD_Private_FormatError_wrapper(err, errtext, sizeof(errtext));
	PLD_Private_WriteString(errtext);

	// An extra newline because dlerror doesn't append one
#ifndef _WIN32
	PLD_Private_WriteString("\n");
#endif
}

#endif // COOL_PLD_H
