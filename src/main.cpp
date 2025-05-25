#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include <dos/rdargs.h>
#include <workbench/startup.h>
#include <utility/tagitem.h>

#include "main.h"
#include "backend-stdout.h"
#include "backend-count.h"
#include "backend-8svx.h"
#include "backend-wave.h"
#include "backend-audio.h"

/* indexes of commandline arguments array */

#define MCARG_TEXT        0
#define MCARG_MODE        1
#define MCARG_DOT         2
#define MCARG_DASH        3
#define MCARG_SAMPRATE    4
#define MCARG_PITCH       5
#define MCARG_WPM         6
#define MCARG_TO          7
#define MCARG_ATTACK      8
#define MCARG_RELEASE     9
#define MCARG_SPAUSE     10
#define MCARG_CPAUSE     11
#define MCARG_WPAUSE     12
#define MCARG_REALWPM    13


extern Library *SysBase, *DOSBase;

Library *UtilityBase;
Library *MathIeeeSingBasBase;
Library *MathIeeeSingTransBase;
Library *IFFParseBase;


struct LibHandle
{
	char *lih_Name;
	ULONG lih_MinVer;
	Library **lih_Ptr;
};

struct LibHandle Libraries[] = {
//	{ "intuition.library", 39, &IntuitionBase },
	{ "utility.library", 39, &UtilityBase },
	{ "mathieeesingbas.library", 37, &MathIeeeSingBasBase },
	{ "mathieeesingtrans.library", 37, &MathIeeeSingTransBase },
	{ "iffparse.library", 39, &IFFParseBase },
	{ NULL, 0, NULL }
};

struct GenDesc
{
	char *type;
	long metrics;
};

#define GENERATOR_COUNT  0
#define GENERATOR_CON    1
#define GENERATOR_8SVX   2
#define GENERATOR_WAVE   3
#define GENERATOR_AUDIO  4

#define GENERATORS       5       

GenDesc Generators[GENERATORS] = {
	{ "COUNT", FALSE },
	{ "CON", FALSE },
	{ "8SVX", TRUE },
	{ "WAVE", TRUE },
	{ "AUDIO", FALSE }
};


BOOL OpenLibs()
{
	LibHandle *lih = Libraries;
	BOOL success = TRUE;

	while (lih->lih_Name)
	{
		Library *lb;

		lb = OpenLibrary(lih->lih_Name, lih->lih_MinVer);
		*lih->lih_Ptr = lb;
		if (!lb) success = FALSE;
		lih++;
	}

	return success;
}

/*----------------------------------------------------------------------------*/

void CloseLibs()
{
	LibHandle *lih = Libraries;

	while (lih->lih_Name)
	{
		if (*lih->lih_Ptr) CloseLibrary(*lih->lih_Ptr);
		*lih->lih_Ptr = NULL;
		lih++;
	}
}

/*----------------------------------------------------------------------------*/

static MorseGen* CreateGenerator(long mode, long *argvals, long *metrics)
{
	MorseGen *generator = NULL;

	switch (mode)
	{
		case GENERATOR_COUNT:
			generator = new CountMorseGen(TRUE, NULL);
		break;

		case GENERATOR_CON:
			generator = new StdOutMorseGen(
				(char*)argvals[MCARG_DOT],
				(char*)argvals[MCARG_DASH],
				(char*)argvals[MCARG_SPAUSE],
				(char*)argvals[MCARG_CPAUSE],
				(char*)argvals[MCARG_WPAUSE]);
		break;

		case GENERATOR_8SVX:
			generator = new SvxMorseGen(
				*(long*)argvals[MCARG_SAMPRATE],
				*(long*)argvals[MCARG_PITCH],
				*(long*)argvals[MCARG_WPM],
				*(long*)argvals[MCARG_REALWPM],
				*(long*)argvals[MCARG_ATTACK],
				*(long*)argvals[MCARG_RELEASE],
				(char*)argvals[MCARG_TO],
				metrics);
		break;

		case GENERATOR_WAVE:
			generator = new WaveMorseGen(
				*(long*)argvals[MCARG_SAMPRATE],
				*(long*)argvals[MCARG_PITCH],
				*(long*)argvals[MCARG_WPM],
				*(long*)argvals[MCARG_REALWPM],
				*(long*)argvals[MCARG_ATTACK],
				*(long*)argvals[MCARG_RELEASE],
				(char*)argvals[MCARG_TO],
				metrics);
		break;

		case GENERATOR_AUDIO:
			generator = new AudioDevMorseGen(
				*(long*)argvals[MCARG_SAMPRATE],
				*(long*)argvals[MCARG_PITCH],
				*(long*)argvals[MCARG_WPM],
				*(long*)argvals[MCARG_REALWPM],
				*(long*)argvals[MCARG_ATTACK],
				*(long*)argvals[MCARG_RELEASE]);
		break;
	}

	return generator;
}

/*---------------------------------------------------------------------------*/

void ParseCommandline()
{
	RDArgs *args;
	long metrics[5];

	long numdefs[6] = {
		8000,                    /* SAMPLERATE */
		500,                     /* PITCH */
		20,                      /* WPM */
		0,                       /* ATTACK */
		1,                       /* RELEASE */
		0,                       /* REALWPM */
	};

	long argvals[14] = {
		0,                       /* [00] TEXT */
		(int)"CON",              /* [01] MODE */
		(int)".",                /* [02] DOT */
		(int)"-",                /* [03] DASH */
	 	(int)&numdefs[0],        /* [04] SAMPLERATE */
	 	(int)&numdefs[1],        /* [05] PITCH */
	 	(int)&numdefs[2],        /* [06] WPM */
		0,                       /* [07] TO */
		(int)&numdefs[3],        /* [08] ATTACK */
		(int)&numdefs[4],        /* [09] RELEASE */
		(int)"",                 /* [10] SPAUSE */
		(int)" ",                /* [11] CPAUSE */
		(int)"   ",              /* [12] WPAUSE */
		(int)&numdefs[5]         /* [13] REALWPM */
	};

	if (args = ReadArgs("TEXT/A,MODE/K,DOT/K,DASH/K,SAMPLERATE=SR/K/N,PITCH/K/N,WPM/K/N,TO/K,"
	"ATTACK/K/N,RELEASE/K/N,SPAUSE/K,CPAUSE/K,WPAUSE/K,REALWPM/K/N", argvals, NULL))
	{
		long mode;

		for (mode = 0; mode < GENERATORS; mode++)
		{
			if (Stricmp((const char*)argvals[MCARG_MODE], Generators[mode].type) == 0) break;
		}

		if (mode < GENERATORS)
		{
			MorseGen *generator;
	
			if (Generators[mode].metrics)
			{
				CountMorseGen counter(FALSE, metrics);
				counter.MorseText((const char*)argvals[MCARG_TEXT]);
			}

			if (generator = CreateGenerator(mode, argvals, metrics))
			{
				if (generator->Initialized)
				{
					generator->MorseText((const char*)argvals[MCARG_TEXT]);
				}

				delete generator;
			}
		}
		else Printf("arguments check: no such mode '%s'\n", (long)argvals[MCARG_MODE]);

		FreeArgs(args);
	}
	else PrintFault(IoErr(), "argument parser");
}

/*---------------------------------------------------------------------------*/

LONG Main(WBStartup *wbmsg)
{
	LONG result = RETURN_FAIL;

	if (OpenLibs())
	{
		ParseCommandline();
	}

	CloseLibs();
	return result;
}
