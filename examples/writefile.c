#include "../pld.h"

PLD("KERNEL32.DLL")
int main(void) {
	PFN(h, WriteFile);
	DWORD dummy;
	h(GetStdHandle(STD_OUTPUT_HANDLE), "Hello World!\n", 13, &dummy, NULL);
	return 0;
}
