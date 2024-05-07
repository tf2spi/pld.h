// PLDLOG uses write by default so redefine it to do nothing
#define PLDLOG(err, buf, len) (void)err, (void)buf, (void)len
#include "../pld.h"

PLD("libc.so.6");
long write(int fd, const void *buf, unsigned long len)
{
	PFN(h, write);
	return h(fd, buf, len);
}

int main(void)
{
	write(1, "Hello World!\n", 13);
	return 0;
}
