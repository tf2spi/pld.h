#include "../pld.h"

#define PLDH k32
PLD("KERNEL32.DLL");
static DWORD MyWriteFile(void)
{
	DWORD dummy;
	PFN(h, WriteFile);
	return h(GetStdHandle(STD_OUTPUT_HANDLE), "Hello World!\n", 13, &dummy, NULL);
}
#undef PLDH
#define PLDH u32
PLD("USER32.DLL");

int main(void) {
	MyWriteFile();
	PFN(MessageBoxA)(NULL, "Hello!", "Hello!", MB_OK);
	return 0;
}
