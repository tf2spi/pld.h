#include "../pld.h"
#include <stdio.h>
PLD("libc.so.6")
int main()
{
	PFN(o, stderr);
	fprintf(*o, "Hello World!\n");
	return 0;
}
