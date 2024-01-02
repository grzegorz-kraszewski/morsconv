#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/iffparse.h>

#include <exec/memory.h>

#include "main.h"
#include "morsegen.h"

struct SvxMorseGen
{
	struct MorseGen mg;
	LONG smg_SamplingRate;
	LONG smg_TonePitch;
	LONG smg_WordsPerMinute;
	LONG smg_SamplesPerDot;
	BYTE *smg_DashSilence;
	BYTE *smg_DashTone;
	STRPTR smg_OutputPath;
	BPTR smg_OutputFile;
	struct IFFHandle* smg_OutputIFF;
};	
	
	
/****i* backend-8svx/SvxSetup ************************************************
*
* NAME
*   SvxSetup -- Prepares Morse 8SVX backend to work.
*
* SYNOPSIS
*   success = SvxSetup(morsegen, taglist)
*   BOOL SvxSetup(struct MorseGen*);
*
* FUNCTION
*   Parses the taglist of parameters, overwriting default values. Checks if
*   values of these parameters are within allowed ranges. Calculates number of
*   audio samples per dot symbol. Allocates two audio buffers, one with
*   silence, one with tone.
*
*   Control tags:
*     MA_SamplingRate - sampling frequency of generated audio file in Hz.
*       Default value is 8000 Hz. Allowed range: 1000 to 65535 Hz.
*     MA_TonePitch - tone pitch of Morse symbols. Default value is 500 Hz.
*       Allowed range is 100 to 8000 Hz with additional requirement, that
*       pitch cannot be higher than half of the sampling rate. 
*     MA_WordsPerMinute - Morse code speed based on PARIS word. Defaults to
*       20 wpm. Allowed range: 5 to 100 wpm. 
*
* INPUTS
*   morsegen - control structure.
*   taglist - parameters.
*
* RESULT
*   May be FALSE in following conditions:
*   - parameter(s)out of range
*   - pitch is higher than half of sampling rate, so sampling theorem is not
*     fulfilled
*   - failed buffer allocation
*
* NOTES
*   Upper sampling rate limit of 65535 Hz is IFF 8SVX format limitation.
*
******************************************************************************
*/

static void ParseTags(struct SvxMorseGen *mg, struct TagItem *taglist)
{
	struct TagItem *tag, *tagptr = taglist;

	while (tag = NextTagItem(&tagptr))
	{
		switch (tag->ti_Tag)
		{
			case MA_SamplingRate:    mg->smg_SamplingRate = tag->ti_Data;         break;
			case MA_TonePitch:       mg->smg_TonePitch = tag->ti_Data;            break;
			case MA_WordsPerMinute:  mg->smg_WordsPerMinute = tag->ti_Data;       break;
			case MA_OutputFile:      mg->smg_OutputPath = (STRPTR)tag->ti_Data;   break;
		}
	}
}


static BOOL RangeChecks(struct SvxMorseGen *mg)
{
	BOOL success = TRUE;
	
	if (mg->smg_SamplingRate < 1000) success = FALSE;
	if (mg->smg_SamplingRate > 65535) success = FALSE;
	if (mg->smg_TonePitch < 100) success = FALSE;
	if (mg->smg_TonePitch > 8000) success = FALSE;
	if ((mg->smg_TonePitch << 1) > mg->smg_SamplingRate) success = FALSE;
	if (mg->smg_WordsPerMinute < 5) success = FALSE;
	if (mg->smg_WordsPerMinute > 100) success = FALSE;
	if (!success) SetIoErr(ERROR_BAD_NUMBER);
	return success;
}


static BOOL SvxSetup(struct MorseGen *mg, struct TagItem *taglist)
{
	struct SvxMorseGen *smg = (struct SvxMorseGen*)mg;

	ParseTags(smg, taglist);
	return RangeChecks(smg);

	#if 0	
	
	smg->smg_SamplesPerDot = SDivMod32(SMult32(smg->smg_SamplingRate, 6),
		SMult32(smg->smg_WordsPerMinute, 5));
	Printf("Samples per dot = %ld.\n", smg->smg_SamplesPerDot);
	buflen = SMult32(smg->smg_SamplesPerDot, 3);

	if (!smg->smg_OutputPath)
	{
		SetIOErr(ERROR_REQUIRED_ARG_MISSING);
		return FALSE;
	}

	if (smg->smg_OutputIFF = AllocIFF())
	{
		if (smg->smg_OutputFile = Open(smg->smg_OutputPath, MODE_NEWFILE)
		{
		
		if (smg->smg_DashSilence = AllocVec(buflen, MEMF_ANY | MEMF_CLEAR))
		{
			if (smg->smg_DashTone = AllocVec(buflen, MEMF_ANY))
			{
				GenerateTone(smg->smg_DashTone, buflen, smg->smg_SamplingRate, smg->smg_TonePitch);
				return TRUE;
			}
		}
	}
	
	#endif
}


/****i* backend-8svx/SvxCleanup **********************************************
*
* NAME
*   SvxCleanup -- Frees 8SVX backend resources and disposes it.
*
* SYNOPSIS
*   SvxCleanup(morsegen)
*   void SvxCleanup(struct MorseGen*);
*
* FUNCTION
*   - frees IFF handle,
*   - closes the output file,
*   - frees audio buffers,
*   - frees MorseGen structure.
*
* INPUTS
*   morsegen - control structure. Don't call with NULL.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void SvxCleanup(struct MorseGen *mg)
{
	struct SvxMorseGen *smg = (struct SvxMorseGen*)mg;

	FreeMem(mg, sizeof(struct SvxMorseGen));
}



/****i* backend-8svx/SvxIntraSymbolPause *************************************
*
* NAME
*   SvxIntraSymbolPause -- Generates Morse intra-symbol pause for
*     8SVX backend.
*
* SYNOPSIS
*   SvxIntraSymbolPause(morsegen)
*   void SvxIntraSymbolPause(struct MorseGen*);
*
* FUNCTION
*   Writes silence of dot length to 8SVX file.
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void SvxIntraSymbolPause(struct MorseGen *mg)
{
	struct SvxMorseGen *smg = (struct SvxMorseGen*)mg;
	return;
}


/****i* backend-8svx/SvxInterSymbolPause *************************************
*
* NAME
*   SvxInterSymbolPause -- Generates Morse inter-symbol pause for
*     8SVX backend.
*
* SYNOPSIS
*   SvxInterSymbolPause(morsegen)
*   void SvxInterSymbolPause(struct MorseGen*);
*
* FUNCTION
*   Writes silence of 3 * dot length to 8SVX file.
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void SvxInterSymbolPause(struct MorseGen *mg)
{
	struct SvxMorseGen *smg = (struct SvxMorseGen*)mg;
}


/****i* backend-8svx/SvxInterWordPause ***************************************
*
* NAME
*   SvxInterWordPause -- Generates Morse inter-word pause for 8SVX backend.
*
* SYNOPSIS
*   SvxInterWordPause(morsegen)
*   void SvxInterWordPause(struct MorseGen*);
*
* FUNCTION
*   Writes silence of 7 * dot length to 8SVX file.
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void SvxInterWordPause(struct MorseGen *mg)
{
	struct SvxMorseGen *smg = (struct SvxMorseGen*)mg;
}


/****i* backend-8svx/SvxShortTone ********************************************
*
* NAME
*   SvxShortTone -- Generates Morse short tone ("dot") for 8SVX backend.
*
* SYNOPSIS
*   SvxShortTone(morsegen)
*   void SvxShortTone(struct MorseGen*);
*
* FUNCTION
*   Writes sinusoidal tone of dot length to 8SVX file.
*
* INPUTS
*   morsegen - control structure.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void SvxShortTone(struct MorseGen *mg)
{
	struct SvxMorseGen *smg = (struct SvxMorseGen*)mg;
}


/****i* backend-8svx/SvxLongTone *********************************************
*
* NAME
*   SvxLongTone -- Generates Morse long tone ("dash") for 8SVX backend.
*
* SYNOPSIS
*   SvxLongTone(morsegen)
*   void SvxLongTone(struct MorseGen*);
*
* FUNCTION
*   Writes sinusoidal tone of dash length to 8SVX file.
*
* INPUTS
*   morsegen - control structure.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void SvxLongTone(struct MorseGen *mg)
{
	struct SvxMorseGen *smg = (struct SvxMorseGen*)mg;
}



/****i* backend-8svx/CreateSvxBackend ****************************************
*
* NAME
*   CreateSvxBackend -- Allocates and initializes MorseGen for 8SVX backend.
*
* SYNOPSIS
*   morsegen = CreateSvxBackend()
*   struct MorseGen* CreateSvxBackend(void);
*
* FUNCTION
*   Allocates MorseGen structure and fills it with stdout-backend callbacks.
*   Sets default values of audio attributes. Sets audio buffer pointers to
*   NULL.
*
* INPUTS
*   None.
*
* RESULT
*   Pointer to initialized MorseGen structure or NULL if memory allocation
*   failed.
*
* SEE ALSO
*   SvxCleanup, SvxSetup
*
******************************************************************************
*/

struct MorseGen* CreateSvxBackend(void)
{
	struct SvxMorseGen *smg;
	
	smg = (struct SvxMorseGen*)AllocMem(sizeof(struct SvxMorseGen), MEMF_ANY | MEMF_CLEAR);
	
	if (smg)
	{
		smg->mg.mg_Setup = SvxSetup;
		smg->mg.mg_Cleanup = SvxCleanup;
		smg->mg.mg_IntraSymbolPause = SvxIntraSymbolPause;
		smg->mg.mg_InterSymbolPause = SvxInterSymbolPause;
		smg->mg.mg_InterWordPause = SvxInterWordPause;
		smg->mg.mg_ShortTone = SvxShortTone;
		smg->mg.mg_LongTone = SvxLongTone;
		smg->smg_SamplingRate = 8000;
		smg->smg_TonePitch = 500;
		smg->smg_WordsPerMinute = 20;
	}
	
	return &smg->mg;
}
