#include "../pld.h"
#include <stdio.h>
PLD("libc.so.6");
int main(void)
{
	PFN(h, printf);
	h("Hello World! %d\n", 2);
	return 0;
}
