#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include "main.h"
#include "backend-stdout.h"
#include "backend-8svx.h"

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

struct MorsMode
{
	STRPTR mm_Name;
	struct MorseGen*(*mm_Creator)(void);
};

const struct MorsMode Modes[] = 
{
	{ "CON",  CreateStdOutBackend },
	{ "8SVX", CreateSvxBackend },
	{ NULL, NULL }
};

static struct MorseGen* CreateGenerator(LONG *argvals)
{
	STRPTR modename = (STRPTR)argvals[1];
	const struct MorsMode *mode;
	struct MorseGen* generator = NULL;
	
	for (mode = Modes; mode->mm_Name; mode++)
	{
		if (Stricmp(mode->mm_Name, modename) == 0)
		{
			generator = mode->mm_Creator();
			break;
		}
	}
	
	/* Set error if mode hasn't been found. */
	
	if (!mode->mm_Name) SetIoErr(ERROR_NOT_IMPLEMENTED);
	return generator;
}

/*----------------------------------------------------------------------------*/


ULONG Main(void)
{
	ULONG result = 0;

	if (OpenLibs())
	{
		struct RDArgs *args;
		LONG argvals[4] = {
		 0,                       /* TEXT */
		 (LONG)"CON",             /* MODE */
		 (LONG)".",               /* DOT */
		 (LONG)"-"                /* DASH */
		};
		
		if (args = ReadArgs("TEXT/A,MODE/K,DOT/K,DASH/K", argvals, NULL))
		{
			struct MorseGen *mg;

			if (mg = CreateGenerator(argvals))
			{
				if (MorseGenSetup(mg,
					MA_DotText, argvals[2],
					MA_DashText, argvals[3],
				TAG_END))
				{					
					MorseText(mg, (STRPTR)argvals[0]);
				}
			
				mg->mg_Cleanup(mg);
			}
			else result = RETURN_FAIL;

			FreeArgs(args);
		}
		else result = RETURN_ERROR;
	
		if (result != 0)
		{
			LONG syserror = IoErr();
			if (syserror) PrintFault(syserror, "MorsConv");
		}		
	}
	else result = RETURN_FAIL;

	CloseLibs();
	return result;
}
