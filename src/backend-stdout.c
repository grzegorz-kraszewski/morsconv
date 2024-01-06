#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include "main.h"
#include "morsegen.h"

struct StdOutMorseGen
{
	struct MorseGen mg;
	STRPTR somg_ShortString;
	STRPTR somg_LongString;
	STRPTR somg_SymbolPauseString;
	STRPTR somg_CharPauseString;
	STRPTR somg_WordPauseString;
};


/****i* backend-stdout/StdOutSetup *******************************************
*
* NAME
*   StdOutSetup -- Prepares Morse stdout-backend to work.
*
* SYNOPSIS
*   success = StdOutSetup(morsegen, taglist)
*   BOOL StdOutSetup(struct MorseGen*);
*
* FUNCTION
*   For stdout-backend setup function sets parameters of generated outputs
*   from the taglist. Following tags are recognized.
*     MA_DotText - text emmited for a dot. Default value is ".".
*     MA_DashText - text emmited for a dash. Default value is "-".
*
* INPUTS
*   morsegen - control structure.
*   taglist - parameters.
*
* RESULT
*   Always TRUE.
*
******************************************************************************
*/

/*-----------------------------------------------------------------
 Replaces escape sequences with characters.
 %n -> $0A
 %t -> $09
 %% -> %
 Unescaped string is always shorter, so it may be done in-place.
 Unknown escape sequences are discarded completely.
-----------------------------------------------------------------*/

static void Unescape(STRPTR s)
{
	STRPTR d = s;
	UBYTE c;

	while (c = *s++)
	{
		if (c == '%')
		{
			switch (*s++)
			{
				case 'n':   *d++ = 0x0A;   break;
				case 't':   *d++ = 0x09;   break;
				case '%':   *d++ = '%';    break;
			}
		}
		else *d++ = c;
	}

	*d = 0x00;
}


static BOOL StdOutSetup(struct MorseGen *mg, struct TagItem *taglist)
{
	struct StdOutMorseGen *somg = (struct StdOutMorseGen*)mg;
	struct TagItem *tag, *tagptr = taglist;

	while (tag = NextTagItem(&tagptr))
	{
		switch (tag->ti_Tag)
		{
			case MA_DotText:       somg->somg_ShortString = (STRPTR)tag->ti_Data;        break;
			case MA_DashText:      somg->somg_LongString = (STRPTR)tag->ti_Data;         break;
			case MA_SymbolPause:   somg->somg_SymbolPauseString = (STRPTR)tag->ti_Data;  break;
			case MA_CharPause:     somg->somg_CharPauseString = (STRPTR)tag->ti_Data;    break;
			case MA_WordPause:     somg->somg_WordPauseString = (STRPTR)tag->ti_Data;    break;
		}
	}

	Unescape(somg->somg_ShortString);
	Unescape(somg->somg_LongString);
	Unescape(somg->somg_SymbolPauseString);
	Unescape(somg->somg_CharPauseString);
	Unescape(somg->somg_WordPauseString);
	return TRUE;
}



/****i* backend-stdout/StdOutCleanup *****************************************
*
* NAME
*   StdOutCleanup -- Frees stdout-backend resources and disposes it.
*
* SYNOPSIS
*   StdOutCleanup(morsegen)
*   void StdOutCleanup(struct MorseGen*);
*
* FUNCTION
*   StdOutCleanup sends a newline, then just deallocates MorseGen structure.
*
* INPUTS
*   morsegen - control structure. Don't call with NULL.
*
* RESULT
*   None.
*
*******************************************************************************
*/

static void StdOutCleanup(struct MorseGen *mg)
{
	WriteChars("\n", 1);
	FreeMem(mg, sizeof(struct StdOutMorseGen));
}



/****i* backend-stdout/StdOutIntraSymbolPause ********************************
*
* NAME
*   StdOutIntraSymbolPause -- Generates Morse intra-symbol pause for
*     stdout-backend.
*
* SYNOPSIS
*   StdOutIntraSymbolPause(morsegen)
*   void StdOutIntraSymbolPause(struct MorseGen*);
*
* FUNCTION
*   Intra-symbol pause is ignored in this backend. Textual representation of
*   Morse code provides intra-symbol pauses as separation between characters.
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void StdOutIntraSymbolPause(struct MorseGen *mg)
{
	struct StdOutMorseGen *somg = (struct StdOutMorseGen*)mg;

	PutStr(somg->somg_SymbolPauseString);
	return;
}


/****i* backend-stdout/StdOutInterSymbolPause ********************************
*
* NAME
*   StdOutInterSymbolPause -- Generates Morse inter-symbol pause for
*     stdout-backend.
*
* SYNOPSIS
*   StdOutInterSymbolPause(morsegen)
*   void StdOutInterSymbolPause(struct MorseGen*);
*
* FUNCTION
*   Inter-symbol pause is represented by this backend as a space. It is sent
*   to the standard output by WriteChars().
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void StdOutInterSymbolPause(struct MorseGen *mg)
{
	struct StdOutMorseGen *somg = (struct StdOutMorseGen*)mg;

	PutStr(somg->somg_CharPauseString);	
	return;
}


/****i* backend-stdout/StdOutInterWordPause **********************************
*
* NAME
*   StdOutInterWordPause -- Generates Morse inter-word pause for
*     stdout-backend.
*
* SYNOPSIS
*   StdOutInterWordPause(morsegen)
*   void StdOutInterWordPause(struct MorseGen*);
*
* FUNCTION
*   Inter-word pause is represented by this backend as three spaces. They
*   are sent to the standard output by WriteChars().
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void StdOutInterWordPause(struct MorseGen *mg)
{
	struct StdOutMorseGen *somg = (struct StdOutMorseGen*)mg;

	PutStr(somg->somg_WordPauseString);
	return;
}


/****i* backend-stdout/StdOutShortTone ***************************************
*
* NAME
*   StdOutShortTone -- Generates Morse short tone ("dit") for stdout-backend.
*
* SYNOPSIS
*   StdOutShortTone(morsegen)
*   void StdOutShortTone(struct MorseGen*);
*
* FUNCTION
*   StdOut backend emits a specified character (default character is '.') for
*   a short tone. Character is sent to stdout using WriteChars().
*
* INPUTS
*   morsegen - control structure.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void StdOutShortTone(struct MorseGen *mg)
{
	struct StdOutMorseGen *somg = (struct StdOutMorseGen*)mg;
	PutStr(somg->somg_ShortString);
}


/****i* backend-stdout/StdOutLongTone ****************************************
*
* NAME
*   StdOutLongTone -- Generates Morse long tone ("dat") for stdout-backend.
*
* SYNOPSIS
*   StdOutLongTone(morsegen)
*   void StdOutLongTone(struct MorseGen*);
*
* FUNCTION
*   StdOut backend emits a string (default is "-") for a long tone. String is
*   sent to stdout using PutStr().
*
* INPUTS
*   morsegen - control structure.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void StdOutLongTone(struct MorseGen *mg)
{
	struct StdOutMorseGen *somg = (struct StdOutMorseGen*)mg;
	PutStr(somg->somg_LongString);
}



/****i* backend-stdout/CreateStdOutBackend ***********************************
*
* NAME
*   CreateStdOutBackend -- Allocates and initializes MorseGen for stdout
*   backend.
*
* SYNOPSIS
*   morsegen = CreateStdOutBackend()
*   struct MorseGen* CreateStdOutBackend(void);
*
* FUNCTION
*   Allocates MorseGen structure and fills it with stdout-backend callbacks.
*
* INPUTS
*   None.
*
* RESULT
*   Pointer to initialized MorseGen structure or NULL if memory allocation
*   failed.
*
* SEE ALSO
*   StdOutCleanup, StdOutSetup, StdOutPause, StdOutTone
*
******************************************************************************
*/

struct MorseGen* CreateStdOutBackend(void)
{
	struct StdOutMorseGen *somg;
	
	somg = (struct StdOutMorseGen*)AllocMem(sizeof(struct StdOutMorseGen), MEMF_ANY);
	
	if (somg)
	{
		somg->mg.mg_Setup = StdOutSetup;
		somg->mg.mg_Cleanup = StdOutCleanup;
		somg->mg.mg_IntraSymbolPause = StdOutIntraSymbolPause;
		somg->mg.mg_InterSymbolPause = StdOutInterSymbolPause;
		somg->mg.mg_InterWordPause = StdOutInterWordPause;
		somg->mg.mg_ShortTone = StdOutShortTone;
		somg->mg.mg_LongTone = StdOutLongTone;
		somg->mg.mg_NeedsMetrics = FALSE;
		somg->somg_ShortString = ".";
		somg->somg_LongString = "-";
		somg->somg_SymbolPauseString = "";
		somg->somg_CharPauseString = " ";
		somg->somg_WordPauseString = "   ";
	}
	
	return &somg->mg;
}
