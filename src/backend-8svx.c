#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/iffparse.h>
#include <proto/mathieeesingbas.h>
#include <proto/mathieeesingtrans.h>

#include <exec/memory.h>

#include "main.h"
#include "morsegen.h"
#include "backend-count.h"


struct SvxMorseGen
{
	struct MorseGen mg;
	LONG *smg_Metrics;
	LONG smg_SamplingRate;
	LONG smg_TonePitch;
	LONG smg_WordsPerMinute;
	LONG smg_RealWordsPerMinute;     /* Farnsworth timing */
	LONG smg_SamplesPerDot;
	LONG smg_SamplesPerCharPause;    /* Farnsworth timing */
	LONG smg_SamplesPerWordPause;    /* Farnsworth timing */
	LONG smg_AttackTime;             /* miliseconds */
	LONG smg_ReleaseTime;            /* miliseconds */
	BYTE *smg_DotTone;
	BYTE *smg_DashTone;
	BYTE *smg_Silence;               /* long enough for word pause, including Farnsworth */
	STRPTR smg_OutputPath;
	BPTR smg_OutputFile;
	struct IFFHandle* smg_OutputIFF;
	BOOL smg_IFFOpened;
	BOOL smg_FormPushed;
	BOOL smg_BodyPushed;
};


#define ID_8SVX MAKE_ID('8','S','V','X')
#define ID_VHDR MAKE_ID('V','H','D','R')
#define ID_BODY MAKE_ID('B','O','D','Y')

struct Vhdr
{
	ULONG OneShotHiSamples;
	ULONG RepeatHiSamples;
	ULONG SamplesPerHiCycle;
	UWORD SamplingRate;
	UBYTE Octave;
	UBYTE Compression;
	ULONG Volume;
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
			case MA_SamplingRate:          mg->smg_SamplingRate = tag->ti_Data;         break;
			case MA_TonePitch:             mg->smg_TonePitch = tag->ti_Data;            break;
			case MA_WordsPerMinute:        mg->smg_WordsPerMinute = tag->ti_Data;       break;
			case MA_OutputFile:            mg->smg_OutputPath = (STRPTR)tag->ti_Data;   break;
			case MA_MorseMetrics:          mg->smg_Metrics = (LONG*)tag->ti_Data;       break;
			case MA_EnvAttack:             mg->smg_AttackTime = tag->ti_Data;           break;
			case MA_EnvRelease:            mg->smg_ReleaseTime = tag->ti_Data;          break;
			case MA_RealWordsPerMinute:    mg->smg_RealWordsPerMinute = tag->ti_Data;   break;
		}
	}
}


static BOOL CreateBody(struct SvxMorseGen *mg, LONG samples)
{
	if (PushChunk(mg->smg_OutputIFF, ID_8SVX, ID_BODY, samples) == 0)
	{
		mg->smg_BodyPushed = TRUE;
		return TRUE;
	}

	return FALSE;
}


static BOOL WriteHeader(struct SvxMorseGen *mg, LONG samples)
{
	struct Vhdr h;

	h.OneShotHiSamples = samples;
	h.RepeatHiSamples = 0;
	h.SamplesPerHiCycle = 0;
	h.SamplingRate = (UWORD)mg->smg_SamplingRate;
	h.Octave = 1;
	h.Compression = 0;
	h.Volume = 0x10000;

	if (PushChunk(mg->smg_OutputIFF, ID_8SVX, ID_FORM, IFFSIZE_UNKNOWN) == 0)
	{
		mg->smg_FormPushed = TRUE;

		if (PushChunk(mg->smg_OutputIFF, ID_8SVX, ID_VHDR, sizeof(struct Vhdr)) == 0)
		{
			if (WriteChunkBytes(mg->smg_OutputIFF, &h, sizeof(struct Vhdr)) == sizeof(struct Vhdr))
			{
				if (PopChunk(mg->smg_OutputIFF) == 0) return CreateBody(mg, samples);
			}
		}
	}

	return FALSE;
}


static BOOL CalculateAudioSize(struct SvxMorseGen *mg)
{
	LONG samples = 0, samples_per_dash;

	samples_per_dash = (mg->smg_SamplesPerDot << 1) + mg->smg_SamplesPerDot;
	samples += SMult32(mg->smg_Metrics[COUNTER_DOTS], mg->smg_SamplesPerDot);
	samples += SMult32(mg->smg_Metrics[COUNTER_DASHES], samples_per_dash);
	samples += SMult32(mg->smg_Metrics[COUNTER_SYMBOL_PAUSES], mg->smg_SamplesPerDot);
	samples += SMult32(mg->smg_Metrics[COUNTER_CHAR_PAUSES], mg->smg_SamplesPerCharPause);
	samples += SMult32(mg->smg_Metrics[COUNTER_WORD_PAUSES], mg->smg_SamplesPerWordPause);
	Printf("%ld audio samples total.\n", samples);
	return WriteHeader(mg, samples);
}


static BOOL OpenOutputIFF(struct SvxMorseGen *mg)
{
	if (mg->smg_OutputIFF = AllocIFF())
	{
		mg->smg_OutputIFF->iff_Stream = mg->smg_OutputFile;
		InitIFFasDOS(mg->smg_OutputIFF);

		if (OpenIFF(mg->smg_OutputIFF, IFFF_WRITE) == 0)
		{
			mg->smg_IFFOpened = TRUE;
			return CalculateAudioSize(mg);
		}
		else SetErr(RETURN_ERROR, 400);
	}
	else SetErr(RETURN_ERROR, ERROR_NO_FREE_STORE);

	return FALSE;
}


static BOOL OpenOutputFile(struct SvxMorseGen *mg)
{
	if (mg->smg_OutputPath)
	{
		if (mg->smg_OutputFile = Open(mg->smg_OutputPath, MODE_NEWFILE))
		{
			Printf("File $%08lx opened.\n", (LONG)mg->smg_OutputFile);
			return OpenOutputIFF(mg);
		}
		else SetErr(RETURN_ERROR, IoErr());
	}
	else SetErr(RETURN_ERROR, ERROR_REQUIRED_ARG_MISSING);

	return FALSE;
}


void GenerateTone(BYTE *buffer, LONG count, LONG samprate, LONG pitch)
{
	LONG i;
	FLOAT step, fpitch;

	step = IEEESPFlt(samprate);
	step = IEEESPDiv(6.28318531f, step);
	fpitch = IEEESPFlt(pitch);
	step = IEEESPMul(step, fpitch);

	for (i = 0; i < count; i++)
	{
		FLOAT x;

		x = IEEESPFlt(i);
		x = IEEESPMul(x, step);
		x = IEEESPSin(x);
		x = IEEESPMul(x, 127.0f);
		*buffer++ = (BYTE)IEEESPFix(x);
	}
}


void ApplyEnvelope(BYTE *buffer, LONG count, LONG attack, LONG release)
{
	LONG i;
	BYTE *p, s;

	for (p = buffer, i = 0; i < attack; i++)
	{
		s = *p;
		*p++ = div16(mul16(s, i), attack);
	}

	for (p = buffer + count - release, i = release - 1; i >= 0; i--)
	{
		s = *p;
		*p++ = div16(mul16(s, i), release);
	}
}

/*--------------------------------------------------------------------------------------------*/
/* Function is prepared to be moved out of this file, as will be used with more backends.     */
/*--------------------------------------------------------------------------------------------*/
/* Note that MorsConv knows Morse code metrics in advance, so it could fit RealWPM perfectly. */
/* However it calculates Farnsworth timing based on PARIS word, the same as ARRL paper.       */
/* It calculates two values, so they are returned via pointers.                               */
/*--------------------------------------------------------------------------------------------*/

void CalculateFarnsworthPauses(LONG samplesperdot, WORD charwpm, WORD realwpm, LONG samplerate,
LONG *charpause, LONG *wordpause)
{
	LONG alpha, beta, cpauseplus, wpauseplus;
	
	alpha = SMult32(SMult32(60, samplerate), charwpm - realwpm);	
	beta = mul16(charwpm, realwpm);   /* max 10000 */
	beta = SMult32(beta, 19);
	cpauseplus = SDivMod32(SMult32(3, alpha), beta);
	wpauseplus = SDivMod32(SMult32(7, alpha), beta);
	*charpause = SMult32(samplesperdot, 3) + cpauseplus;
	*wordpause = SMult32(samplesperdot, 7) + wpauseplus;
}


static BOOL PrepareBuffers(struct SvxMorseGen *mg)
{
	LONG attack_samples, release_samples, samples_per_dash;

	mg->smg_SamplesPerDot = SDivMod32(SMult32(mg->smg_SamplingRate, 6), SMult32(mg->smg_WordsPerMinute, 5));
	samples_per_dash = (mg->smg_SamplesPerDot << 1) + mg->smg_SamplesPerDot;
	CalculateFarnsworthPauses(mg->smg_SamplesPerDot, mg->smg_WordsPerMinute, mg->smg_RealWordsPerMinute,
	mg->smg_SamplingRate, &mg->smg_SamplesPerCharPause, &mg->smg_SamplesPerWordPause);
	attack_samples = SDivMod32(SMult32(mg->smg_SamplingRate, mg->smg_AttackTime), 1000);
	release_samples = SDivMod32(SMult32(mg->smg_SamplingRate, mg->smg_ReleaseTime), 1000);

	if (mg->smg_DotTone = AllocVec(mg->smg_SamplesPerDot + samples_per_dash + mg->smg_SamplesPerWordPause,
	MEMF_ANY | MEMF_CLEAR))
	{
		mg->smg_DashTone = mg->smg_DotTone + mg->smg_SamplesPerDot;
		mg->smg_Silence = mg->smg_DashTone + samples_per_dash;
		GenerateTone(mg->smg_DotTone, mg->smg_SamplesPerDot, mg->smg_SamplingRate, mg->smg_TonePitch);
		ApplyEnvelope(mg->smg_DotTone, mg->smg_SamplesPerDot, attack_samples, release_samples);
		GenerateTone(mg->smg_DashTone, samples_per_dash, mg->smg_SamplingRate, mg->smg_TonePitch);
		ApplyEnvelope(mg->smg_DashTone, samples_per_dash, attack_samples, release_samples);
		return OpenOutputFile(mg);
	}

	SetErr(RETURN_ERROR, ERROR_NO_FREE_STORE);
	return FALSE;
}


static BOOL RangeChecks(struct SvxMorseGen *mg)
{
	BOOL success = TRUE;

	/* if RealWPM has been not specified in taglist, it defaults to WPM */

	if (mg->smg_RealWordsPerMinute == 0) mg->smg_RealWordsPerMinute = mg->smg_WordsPerMinute;

	if (mg->smg_SamplingRate < 1000) success = FALSE;
	if (mg->smg_SamplingRate > 65535) success = FALSE;
	if (mg->smg_TonePitch < 100) success = FALSE;
	if (mg->smg_TonePitch > 8000) success = FALSE;
	if ((mg->smg_TonePitch << 2) > mg->smg_SamplingRate) success = FALSE;
	if (mg->smg_WordsPerMinute < 5) success = FALSE;
	if (mg->smg_WordsPerMinute > 100) success = FALSE;
	if (mg->smg_RealWordsPerMinute < 5) success = FALSE;
	if (mg->smg_RealWordsPerMinute > 100) success = FALSE;
	if (mg->smg_RealWordsPerMinute > mg->smg_WordsPerMinute) success = FALSE;
	if (mg->smg_AttackTime < 0) success = FALSE;
	if (mg->smg_AttackTime > 50) success = FALSE;
	if (mg->smg_ReleaseTime < 0) success = FALSE;
	if (mg->smg_ReleaseTime > 50) success = FALSE;
	if (success) return PrepareBuffers(mg);

	SetErr(RETURN_ERROR, ERROR_BAD_NUMBER);
	return FALSE;
}


static BOOL SvxSetup(struct MorseGen *mg, struct TagItem *taglist)
{
	struct SvxMorseGen *smg = (struct SvxMorseGen*)mg;

	ParseTags(smg, taglist);

	if (smg->smg_Metrics)
	{
		return RangeChecks(smg);
	}
	else SetErr(RETURN_ERROR, ERROR_REQUIRED_ARG_MISSING);
	
	return FALSE;
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

	if (smg->smg_BodyPushed) PopChunk(smg->smg_OutputIFF);  /* BODY */
	if (smg->smg_FormPushed) PopChunk(smg->smg_OutputIFF);  /* FORM */
	if (smg->smg_IFFOpened) CloseIFF(smg->smg_OutputIFF);
	if (smg->smg_OutputIFF) FreeIFF(smg->smg_OutputIFF);
	if (smg->smg_OutputFile) Close(smg->smg_OutputFile);
	if (smg->smg_DotTone) FreeVec(smg->smg_DotTone);
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

	WriteChunkBytes(smg->smg_OutputIFF, smg->smg_Silence, smg->smg_SamplesPerDot);
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
*   Writes silence of inter-symbol lenght to 8SVX file.
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

	WriteChunkBytes(smg->smg_OutputIFF, smg->smg_Silence, smg->smg_SamplesPerCharPause);
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
*   Writes silence of inter-word pause length to 8SVX file.
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

	WriteChunkBytes(smg->smg_OutputIFF, smg->smg_Silence, smg->smg_SamplesPerWordPause);
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

	WriteChunkBytes(smg->smg_OutputIFF, smg->smg_DotTone, smg->smg_SamplesPerDot);
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
	LONG samples_per_dash;
	
	samples_per_dash = (smg->smg_SamplesPerDot << 1) + smg->smg_SamplesPerDot;
	WriteChunkBytes(smg->smg_OutputIFF, smg->smg_DashTone, samples_per_dash);
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
		smg->mg.mg_NeedsMetrics = TRUE;
		smg->smg_SamplingRate = 8000;
		smg->smg_TonePitch = 500;
		smg->smg_WordsPerMinute = 20;
		smg->smg_ReleaseTime = 1;
	}

	return &smg->mg;
}
