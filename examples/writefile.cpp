#include "../pld.h"

PLD("KERNEL32.DLL")
int main(void) {
	DWORD dummy;
	PFNFIND(WriteFile)(GetStdHandle(STD_OUTPUT_HANDLE), "Hello World!\n", 13, &dummy, NULL);
	return 0;
}
