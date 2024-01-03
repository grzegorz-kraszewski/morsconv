#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include "main.h"
#include "morsegen.h"

struct CountMorseGen
{
	struct MorseGen mg;
	LONG cmg_Dots;
	LONG cmg_Dashes;
	LONG cmg_SymbolPauses;
	LONG cmg_CharPauses;
	LONG cmg_WordPauses;
	LONG *cmg_CounterStorage;
	BOOL cmg_Print;
};	
	
	
/****i* backend-count/CountSetup *********************************************
*
* NAME
*   CountSetup -- Prepares Morse count backend to work.
*
* SYNOPSIS
*   success = CountSetup(morsegen, taglist)
*   BOOL CountSetup(struct MorseGen*);
*
* FUNCTION
*   Control tags:
*     MA_CounterStorage - a pointer to array of 5 LONGs, where values of
*       counters are stored during cleanup. Default is NULL (counters are not
*       stored anywhere). Order of counters is: dots, dashes, symbol pauses,
*       character pauses, word pauses. One can use predefined constants
*       COUNTER_xxx as indexes to the array.
*     MA_CounterPrint - boolean. If TRUE, values of counters are printed
*       to standard output separated by spaces with newline at end. Order of
*       counters is the same as described in MA_CounterStorage. Defaults to
*       FALSE.
*
* INPUTS
*   morsegen - control structure.
*   taglist - parameters.
*
* RESULT
*
******************************************************************************
*/

static void ParseTags(struct CountMorseGen *mg, struct TagItem *taglist)
{
	struct TagItem *tag, *tagptr = taglist;

	while (tag = NextTagItem(&tagptr))
	{
		switch (tag->ti_Tag)
		{
			case MA_CounterStorage:   mg->cmg_CounterStorage = (LONG*)tag->ti_Data;    break;
			case MA_CounterPrint:     mg->cmg_Print = (BOOL)tag->ti_Data;              break;
		}
	}
}


static BOOL CountSetup(struct MorseGen *mg, struct TagItem *taglist)
{
	struct CountMorseGen *cmg = (struct CountMorseGen*)mg;

	ParseTags(cmg, taglist);
	return TRUE;
}


/****i* backend-count/CountCleanup *******************************************
*
* NAME
*   CountCleanup -- Frees count backend resources and disposes it.
*
* SYNOPSIS
*   CountCleanup(morsegen)
*   void CountCleanup(struct MorseGen*);
*
* FUNCTION
*   - Copies counters to storage array if MA_CounterStorage has been set.
*   - Prints values of counters to stdout if MA_CounterPrint has been set.
*   - Frees MorseGen structure.
*
* INPUTS
*   morsegen - control structure. Don't call with NULL.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void CountCleanup(struct MorseGen *mg)
{
	struct CountMorseGen *cmg = (struct CountMorseGen*)mg;

	if (cmg->cmg_CounterStorage)
	{
		cmg->cmg_CounterStorage[COUNTER_DOTS] = cmg->cmg_Dots;
		cmg->cmg_CounterStorage[COUNTER_DASHES] = cmg->cmg_Dashes;
		cmg->cmg_CounterStorage[COUNTER_SYMBOL_PAUSES] = cmg->cmg_SymbolPauses;
		cmg->cmg_CounterStorage[COUNTER_CHAR_PAUSES] = cmg->cmg_CharPauses;
		cmg->cmg_CounterStorage[COUNTER_WORD_PAUSES] = cmg->cmg_WordPauses;
	}
	
	if (cmg->cmg_Print)
	{
		Printf("%ld %ld %ld %ld %ld\n", cmg->cmg_Dots, cmg->cmg_Dashes, cmg->cmg_SymbolPauses,
			cmg->cmg_CharPauses, cmg->cmg_WordPauses);
	}

	FreeMem(mg, sizeof(struct CountMorseGen));
}



/****i* backend-count/CountIntraSymbolPause **********************************
*
* NAME
*   CountIntraSymbolPause -- Generates Morse intra-symbol pause for
*     count backend.
*
* SYNOPSIS
*   CountIntraSymbolPause(morsegen)
*   void CountIntraSymbolPause(struct MorseGen*);
*
* FUNCTION
*   Increments counter of symbol pauses.
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void CountIntraSymbolPause(struct MorseGen *mg)
{
	struct CountMorseGen *cmg = (struct CountMorseGen*)mg;

	cmg->cmg_SymbolPauses++;
	return;
}


/****i* backend-count/CountInterSymbolPause **********************************
*
* NAME
*   CountInterSymbolPause -- Generates Morse inter-symbol pause for
*     count backend.
*
* SYNOPSIS
*   CountInterSymbolPause(morsegen)
*   void CountInterSymbolPause(struct MorseGen*);
*
* FUNCTION
*   Increments counter of character pauses.
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void CountInterSymbolPause(struct MorseGen *mg)
{
	struct CountMorseGen *cmg = (struct CountMorseGen*)mg;

	cmg->cmg_CharPauses++;
	return;
}


/****i* backend-count/CountInterWordPause ************************************
*
* NAME
*   CountInterWordPause -- Generates Morse inter-word pause for count backend.
*
* SYNOPSIS
*   CountInterWordPause(morsegen)
*   void CountInterWordPause(struct MorseGen*);
*
* FUNCTION
*   Increments counter of word pauses.
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void CountInterWordPause(struct MorseGen *mg)
{
	struct CountMorseGen *cmg = (struct CountMorseGen*)mg;

	cmg->cmg_WordPauses++;
	return;
}


/****i* backend-count/CountShortTone *****************************************
*
* NAME
*   CountShortTone -- Generates Morse short tone ("dot") for count backend.
*
* SYNOPSIS
*   CountShortTone(morsegen)
*   void CountShortTone(struct MorseGen*);
*
* FUNCTION
*   Increments counter of dots.
*
* INPUTS
*   morsegen - control structure.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void CountShortTone(struct MorseGen *mg)
{
	struct CountMorseGen *cmg = (struct CountMorseGen*)mg;

	cmg->cmg_Dots++;
}


/****i* backend-count/CountLongTone ******************************************
*
* NAME
*   CountLongTone -- Generates Morse long tone ("dash") for count backend.
*
* SYNOPSIS
*   CountLongTone(morsegen)
*   void CountLongTone(struct MorseGen*);
*
* FUNCTION
*   Increments counter of dashes.
*
* INPUTS
*   morsegen - control structure.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void CountLongTone(struct MorseGen *mg)
{
	struct CountMorseGen *cmg = (struct CountMorseGen*)mg;

	cmg->cmg_Dashes++;
	return;
}


/****i* backend-count/CreateCountBackend ****************************************
*
* NAME
*   CreateCountBackend -- Allocates and initializes MorseGen for count backend.
*
* SYNOPSIS
*   morsegen = CreateCountBackend()
*   struct MorseGen* CreateCountBackend(void);
*
* FUNCTION
*   Allocates MorseGen structure and fills it with count-backend callbacks.
*   Counters are cleared automatically due to MEMF_CLEAR flag for allocation.
*
* INPUTS
*   None.
*
* RESULT
*   Pointer to initialized MorseGen structure or NULL if memory allocation
*   failed.
*
* SEE ALSO
*   CountCleanup, CountSetup
*
******************************************************************************
*/

struct MorseGen* CreateCountBackend(void)
{
	struct CountMorseGen *cmg;
	
	cmg = (struct CountMorseGen*)AllocMem(sizeof(struct CountMorseGen), MEMF_ANY | MEMF_CLEAR);
	
	if (cmg)
	{
		cmg->mg.mg_Setup = CountSetup;
		cmg->mg.mg_Cleanup = CountCleanup;
		cmg->mg.mg_IntraSymbolPause = CountIntraSymbolPause;
		cmg->mg.mg_InterSymbolPause = CountInterSymbolPause;
		cmg->mg.mg_InterWordPause = CountInterWordPause;
		cmg->mg.mg_ShortTone = CountShortTone;
		cmg->mg.mg_LongTone = CountLongTone;
	}
	
	return &cmg->mg;
}
