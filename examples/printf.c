#include "../pld.h"
#include <stdio.h>
PLD("libc.so.6");
int main(void)
{
	PFN(printf)("Hello World! %d\n", 2);
	return 0;
}
