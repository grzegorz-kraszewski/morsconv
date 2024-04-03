#include <proto/dos.h>

extern struct Library *DOSBase;

void __pure_virtual(void)
{
	PutStr("pure virtual\n");
}
