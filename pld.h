#ifndef COOL_PLD_H
#define COOL_PLD_H

// Always define GNU sources
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#if defined(__cplusplus)
#define PLDTYPEOF(x) decltype(x)
#elif defined(_MSC_VER)
#define PLDTYPEOF(x) __typeof__(x)
#else
#define PLDTYPEOF(x) typeof(x)
#endif

// Allow users to provide their own logging
// This may also help to reduce headers and
// imports to allow for more easy preloads.
#ifndef PLDLOG
#define PLD_PRIVATE_LOG_INCLUDED
static int PLD_Private_strlen(const char *s)
{
	int len = 0;
	while (s[len++])
		;
	return len - 1;
}

#ifdef _WIN32
#include <windows.h>
static void PLD_Private_WriteString(const char *s)
{
	DWORD dummy;
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), s, PLD_Private_strlen(s), &dummy, NULL);
}
static void PLD_Private_FormatError(void *code, char *s, int len, const char *name)
{
	DWORD_PTR arg = (DWORD_PTR)name;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		(DWORD_PTR)code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		s,
		len,
		(va_list *)&arg);
}
#else // _WIN32
#include <dlfcn.h>
#include <unistd.h>
#include <stddef.h>
static void PLD_Private_WriteString(const char *s)
{
	write(2, s, PLD_Private_strlen(s));
}
static void PLD_Private_FormatError(void *code, char *s, int len, const char *name)
{
	(void)name;
	if (len == 0)
		return;
	const char *dlerr = (const char *)code;
	while (--len && (*s++ = *dlerr++))
		;
	*s = 0;
}
#endif // _WIN32

// When code is 0, output nothing
static void PLD_Private_FormatError_wrapper(void *code, char *s, int len, const char *name) {
	// Add an extra newline only on Windows to mimic FormatMessageA
	if (code == 0) {
#ifdef _WIN32
		s[0] = '\r';
		s[1] = '\n';
		s[2] = 0;
#else
		*s = 0;
#endif // _WIN32
		return;
	}
	PLD_Private_FormatError(code, s, len, name);
}


static void PLD_Private_Log(const char *message, const char *name, void *err)
{
	PLD_Private_WriteString(message);
	PLD_Private_WriteString(" ");
	PLD_Private_WriteString(name);
#ifdef _WIN32
	PLD_Private_WriteString("\r\n");
#else
	PLD_Private_WriteString("\n");
#endif // _WIN32
	char errtext[256];
	PLD_Private_FormatError_wrapper(err, errtext, sizeof(errtext), name);
	PLD_Private_WriteString(errtext);

	// An extra newline because dlerror doesn't append one
#ifndef _WIN32
	PLD_Private_WriteString("\n");
#endif
}

#define PLDLOG(message, name, err) PLD_Private_Log(message, name, err)
#endif // PLDLOG

// Private dlopen function has platform-specific behavior when name == NULL
// * Windows: Return GetModuleHandleA(NULL)
// * POSIX: Return RTLD_NEXT
//
// Private dlsym function has platform-specific behavior when handle == NULL
// * Windows: Use GetModuleHandleA(NULL) instead
// * POSIX: Use RTLD_NEXT instead
#ifdef _WIN32

// Optimization to not include Windows twice
// when defining our own PLDLOG
#ifndef PLD_PRIVATE_LOG_INCLUDED
#include <windows.h>
#endif

typedef HMODULE PLD_LibHandle;
typedef FARPROC PLD_ProcAddress;

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

static void *PLD_Private_ErrCode(void)
{
	return (void *)(DWORD_PTR)GetLastError();
}

#else // _WIN32

#ifndef PLD_PRIVATE_LOG_INCLUDED
#include <dlfcn.h>
#include <stddef.h>
#endif

typedef void *PLD_LibHandle;
typedef void *PLD_ProcAddress;

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

static void *PLD_Private_ErrCode(void)
{
	return dlerror();
}

#endif // _WIN32

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
	static PLD_LibHandle PLDH(void) { \
		static PLD_LibHandle handle = NULL; \
		if (handle == NULL) handle = PLD_Private_dlopen_wrapper(name); \
		return handle; \
	} \

// For importing by ordinal on Windows or just using
// another name in a library for whatever reason
#define PFNALTFIND(name, alt) ((PLDTYPEOF(&name))PLD_Private_dlsym_wrapper(PLDH(), alt))
#define PFNFIND(name) PFNALTFIND(name, #name)

// Cache functions that are loaded
#define PFNALT(h, name, alt)  \
	static PLDTYPEOF(&name) h = NULL; \
	if (h == NULL) h = PFNALTFIND(name, alt)
#define PFN(h, name) PFNALT(h, name, #name)

#endif // COOL_PLD_H
