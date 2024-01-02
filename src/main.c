#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include "main.h"
#include "backend-stdout.h"

struct Library
	*IntuitionBase,
	*GfxBase,
	*UtilityBase,
	*MathIeeeSingBasBase,
	*MathIeeeSingTransBase;


ULONG OpenMyWindow(struct Gui *g);
ULONG TheLoop(struct Gui *g);
ULONG GetScreen(void);


struct LibHandle
{
	STRPTR lih_Name;
	ULONG lih_MinVer;
	struct Library **lih_Ptr;
};


struct LibHandle Libraries[] = {
	{ "intuition.library", 39, &IntuitionBase },
	{ "utility.library", 39, &UtilityBase },
	{ "graphics.library", 39, &GfxBase },
	{ "mathieeesingbas.library", 37, &MathIeeeSingBasBase },
	{ "mathieeesingtrans.library", 37, &MathIeeeSingTransBase },
	{ NULL, 0, NULL }
};

/*----------------------------------------------------------------------------*/

BOOL OpenLibs(void)
{
	struct LibHandle *lih = Libraries;
	BOOL success = TRUE;
	
	while (lih->lih_Name)
	{
		struct Library *lb;
		
		lb = OpenLibrary(lih->lih_Name, lih->lih_MinVer);
		*lih->lih_Ptr = lb;
		if (!lb) success = FALSE;
		lih++;
	}
	
	return success; 
}

/*----------------------------------------------------------------------------*/

void CloseLibs(void)
{
	struct LibHandle *lih = Libraries;
	
	while (lih->lih_Name)
	{
		if (*lih->lih_Ptr) CloseLibrary(*lih->lih_Ptr);
		*lih->lih_Ptr = NULL;
		lih++;
	}
}

/*----------------------------------------------------------------------------*/

ULONG Main(void)
{
	ULONG result = 0;

	if (OpenLibs())
	{
		struct RDArgs *args;
		LONG argvals[3] = { 0, (LONG)".", (LONG)"-" };
		
		if (args = ReadArgs("TEXT/A,DOT/K,DASH/K", argvals, NULL))
		{
			struct MorseGen *mg;

			if (mg = CreateStdOutBackend())
			{
				if (MorseGenSetup(mg,
					MA_DotText, argvals[1],
					MA_DashText, argvals[2],
				TAG_END))
				{					
					MorseText(mg, (STRPTR)argvals[0]);
				}
			
				mg->mg_Cleanup(mg);
			}
			else
			{
				SetIoErr(ERROR_NO_FREE_STORE);
				result = RETURN_FAIL;
			}

			FreeArgs(args);
		}
		else result = RETURN_ERROR;
	
		if (result != 0)
		{
			LONG syserror = IoErr();
			if (syserror) PrintFault(syserror, "Telegraphist");
		}		
	}
	else result = RETURN_FAIL;

	CloseLibs();
	return result;
}
