#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include "main.h"
#include "backend-stdout.h"
#include "backend-8svx.h"
#include "backend-count.h"

ULONG MorErr; /* global error var, return code in upper 16 bits, error code in lower 16 bits */


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
	{ "CON",     CreateStdOutBackend },
	{ "8SVX",    CreateSvxBackend },
	{ "COUNT",   CreateCountBackend },
	{ NULL,      NULL }
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
	
	if (!mode->mm_Name) SetErr(RETURN_ERROR, ERROR_NOT_IMPLEMENTED);
	return generator;
}



/*----------------------------------------------------------------------------*/

LONG PrintMyFault()
{
	LONG err = MorErr & 0xFFFF;
	LONG res = MorErr >> 16;

	if (res > 0)
	{
		if (err > 0) PrintFault(err, "MorsConv");
		else Printf("MorsConv: unknown error %ld\n", err);
	}

	return res;
}

/*----------------------------------------------------------------------------*/

ULONG Main(void)
{
	LONG result;
	
	SetErr(RETURN_OK, 0);

	if (OpenLibs())
	{
		struct RDArgs *args;

		LONG numdefs[3] = {
			8000,                    /* SAMPLERATE */
			500,                     /* PITCH */
			20                       /* WPM */
		};

		LONG argvals[8] = {
			0,                       /* TEXT */
			(LONG)"CON",             /* MODE */
			(LONG)".",               /* DOT */
			(LONG)"-",               /* DASH */
		 	(LONG)&numdefs[0],       /* SAMPLERATE */
		 	(LONG)&numdefs[1],       /* PITCH */
		 	(LONG)&numdefs[2],       /* WPM */
		 	0                        /* TO */
		};

		if (args = ReadArgs("TEXT/A,MODE/K,DOT/K,DASH/K,SAMPLERATE=SR/K/N,PITCH/K/N,WPM/K/N,TO/K", argvals, NULL))
		{
			struct MorseGen *mg;

			if (mg = CreateGenerator(argvals))
			{
				if (MorseGenSetup(mg,
					MA_DotText, argvals[2],
					MA_DashText, argvals[3],
					MA_SamplingRate, *(LONG*)argvals[4],
					MA_TonePitch, *(LONG*)argvals[5],
					MA_WordsPerMinute, *(LONG*)argvals[6],
					MA_OutputFile, argvals[7],
					MA_CounterPrint, TRUE,
				TAG_END))
				{					
					MorseText(mg, (STRPTR)argvals[0]);
				}

				mg->mg_Cleanup(mg);
			}

			FreeArgs(args);
		}
		else SetErr(RETURN_ERROR, IoErr());

		result = PrintMyFault();
	}
	else result = RETURN_FAIL;

	CloseLibs();
	return result;
}
