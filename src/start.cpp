/*-------------------------------------*/
/* Minimal C++ startup for AmigaOS 3.x */
/* RastPort, 2024                      */
/*-------------------------------------*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <workbench/startup.h>

Library *SysBase;
Library *DOSBase;
APTR TaskPool = NULL;

extern ULONG Main(WBStartup *wbmsg);


__saveds ULONG Start(void)
{
	Process *myproc = NULL;
	WBStartup *wbmsg = NULL;
	BOOL have_shell = FALSE;
	ULONG result = RETURN_OK;

	SysBase = *(Library**)4L;
	myproc = (Process*)FindTask(NULL);

	if (myproc->pr_CLI) have_shell = TRUE;

	if (!have_shell)
	{
		WaitPort(&myproc->pr_MsgPort);
		wbmsg = (WBStartup*)GetMsg(&myproc->pr_MsgPort);
	}

	result = RETURN_FAIL;

	if (DOSBase = OpenLibrary("dos.library", 39))
	{
		if (TaskPool = CreatePool(MEMF_ANY, 4096, 2048))
		{
			result = Main(wbmsg);
			DeletePool(TaskPool);
		}

		CloseLibrary(DOSBase);
	}

	if (wbmsg)
	{
		Forbid();
		ReplyMsg(&wbmsg->sm_Message);
	}

	return (ULONG)result;
}


__attribute__((section(".text"))) UBYTE VString[] = "$VER: MorsConv 2.3 (03.04.2024)\r\n";



APTR operator new(ULONG size) throw()
{
	ULONG *m;

	size += 4;

	if (m = (ULONG*)AllocPooled(TaskPool, size))
	{
		*m = size;
		return m + 1;
	}
	else return NULL;
}


APTR operator new[](ULONG size)
{
	return operator new(size);
}


void operator delete(APTR memory)
{
	ULONG *m = (ULONG*)memory - 1;

	FreePooled(TaskPool, m, *m);
}


void operator delete[](APTR memory)
{
	operator delete(memory);
}
