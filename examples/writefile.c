#include "../pld.h"

PLD(KERNEL32)
int main(void) {
	DWORD dummy;
	PFN(WriteFile)(GetStdHandle(STD_OUTPUT_HANDLE), "Hello World!\n", 13, &dummy, NULL);
	return 0;
}
