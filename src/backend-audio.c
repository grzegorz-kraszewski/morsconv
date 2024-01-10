#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include <devices/audio.h>

#include "main.h"
#include "morsegen.h"

struct AudioMorseGen
{
	struct MorseGen mg;
	struct MsgPort *amg_Port;
	struct IOAudio *amg_Req0;
	struct IOAudio *amg_Req1;
};	
	
	
/****i* backend-audio/AudioSetup *********************************************
*
* NAME
*   AudioSetup -- Prepares Morse audio backend to work.
*
* SYNOPSIS
*   success = AudioSetup(morsegen, taglist)
*   BOOL AudioSetup(struct MorseGen*);
*
* FUNCTION
*   Control tags:
*
* INPUTS
*   morsegen - control structure.
*   taglist - parameters.
*
* RESULT
*
******************************************************************************
*/

static void ParseTags(struct AudioMorseGen *mg, struct TagItem *taglist)
{
	struct TagItem *tag, *tagptr = taglist;

	while (tag = NextTagItem(&tagptr))
	{
		switch (tag->ti_Tag)
		{
			// case MA_CounterStorage:   mg->cmg_CounterStorage = (LONG*)tag->ti_Data;    break;
		}
	}
}

/*---------------------------------------------------------------------------*/

static BOOL OpenAudioDevice(struct AudioMorseGen *amg)
{
	Printf("audio: req0 at $%08lx, req1 at $%08lx.\n", (LONG)amg->amg_Req0, (LONG)amg->amg_Req1);
	SetErr(RETURN_ERROR, ERROR_NOT_IMPLEMENTED);
	return FALSE;
}

/*---------------------------------------------------------------------------*/

static BOOL GetAudioRequests(struct AudioMorseGen *amg)
{
	Printf("audio: msgport at $%08lx.\n", (LONG)amg->amg_Port);
	
	if (amg->amg_Req0 = (struct IOAudio*)CreateIORequest(amg->amg_Port, sizeof(struct IOAudio)))
	{
		if (amg->amg_Req1 = (struct IOAudio*)CreateIORequest(amg->amg_Port, sizeof(struct IOAudio)))
		{
			return OpenAudioDevice(amg);
		}
	}
	
	SetErr(RETURN_ERROR, ERROR_NO_FREE_STORE);
	return FALSE;
}

/*---------------------------------------------------------------------------*/

static BOOL GetAudioPort(struct AudioMorseGen *amg)
{
	if (amg->amg_Port = CreateMsgPort()) return GetAudioRequests(amg);
	SetErr(RETURN_ERROR, ERROR_NO_FREE_STORE);
	return FALSE;
}


static BOOL AudioSetup(struct MorseGen *mg, struct TagItem *taglist)
{
	struct AudioMorseGen *amg = (struct AudioMorseGen*)mg;

	ParseTags(amg, taglist);
	return GetAudioPort(amg);
}


/****i* backend-audio/AudioCleanup *******************************************
*
* NAME
*   AudioCleanup -- Frees audio backend resources and disposes it.
*
* SYNOPSIS
*   AudioCleanup(morsegen)
*   void AudioCleanup(struct MorseGen*);
*
* FUNCTION
*   - Frees audio message port.
*   - Frees AudioMorseGen structure.
*
* INPUTS
*   morsegen - control structure. Don't call with NULL.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void AudioCleanup(struct MorseGen *mg)
{
	struct AudioMorseGen *amg = (struct AudioMorseGen*)mg;

	if (amg->amg_Req1) DeleteIORequest(amg->amg_Req1);
	if (amg->amg_Req0) DeleteIORequest(amg->amg_Req0);
	if (amg->amg_Port) DeleteMsgPort(amg->amg_Port);
	FreeMem(mg, sizeof(struct AudioMorseGen));
	return;
}


/****i* backend-audio/AudioIntraSymbolPause **********************************
*
* NAME
*   AudioIntraSymbolPause -- Generates Morse intra-symbol pause for
*     audio backend.
*
* SYNOPSIS
*   AudioIntraSymbolPause(morsegen)
*   void AudioIntraSymbolPause(struct MorseGen*);
*
* FUNCTION
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void AudioIntraSymbolPause(struct MorseGen *mg)
{
	struct AudioMorseGen *amg = (struct AudioMorseGen*)mg;

	return;
}


/****i* backend-audio/AudioInterSymbolPause **********************************
*
* NAME
*   AudioInterSymbolPause -- Generates Morse inter-symbol pause for
*     audio backend.
*
* SYNOPSIS
*   AudioInterSymbolPause(morsegen)
*   void AudioInterSymbolPause(struct MorseGen*);
*
* FUNCTION
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void AudioInterSymbolPause(struct MorseGen *mg)
{
	struct AudioMorseGen *amg = (struct AudioMorseGen*)mg;

	return;
}


/****i* backend-audio/AudioInterWordPause ************************************
*
* NAME
*   AudioInterWordPause -- Generates Morse inter-word pause for audio backend.
*
* SYNOPSIS
*   AudioInterWordPause(morsegen)
*   void AudioInterWordPause(struct MorseGen*);
*
* FUNCTION
*
* INPUTS
*   morsegen - control structure.
*
*  RESULT
*    None.
*
******************************************************************************
*/

static void AudioInterWordPause(struct MorseGen *mg)
{
	struct AudioMorseGen *amg = (struct AudioMorseGen*)mg;

	return;
}


/****i* backend-audio/AudioShortTone *****************************************
*
* NAME
*   AudioShortTone -- Generates Morse short tone ("dot") for audio backend.
*
* SYNOPSIS
*   AudioShortTone(morsegen)
*   void AudioShortTone(struct MorseGen*);
*
* FUNCTION
*
* INPUTS
*   morsegen - control structure.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void AudioShortTone(struct MorseGen *mg)
{
	struct AudioMorseGen *amg = (struct AudioMorseGen*)mg;

}


/****i* backend-audio/AudioLongTone ******************************************
*
* NAME
*   AudioLongTone -- Generates Morse long tone ("dash") for audio backend.
*
* SYNOPSIS
*   AudioLongTone(morsegen)
*   void AudioLongTone(struct MorseGen*);
*
* FUNCTION
*
* INPUTS
*   morsegen - control structure.
*
* RESULT
*   None.
*
******************************************************************************
*/

static void AudioLongTone(struct MorseGen *mg)
{
	struct AudioMorseGen *amg = (struct AudioMorseGen*)mg;

	return;
}


/****i* backend-audio/CreateAudioBackend ****************************************
*
* NAME
*   CreateAudioBackend -- Allocates and initializes MorseGen for audio backend.
*
* SYNOPSIS
*   morsegen = CreateAudioBackend()
*   struct MorseGen* CreateAudioBackend(void);
*
* FUNCTION
*   Allocates MorseGen structure and fills it with audio-backend callbacks.
*
* INPUTS
*   None.
*
* RESULT
*   Pointer to initialized MorseGen structure or NULL if memory allocation
*   failed.
*
* SEE ALSO
*   AudioCleanup, AudioSetup
*
******************************************************************************
*/

struct MorseGen* CreateAudioBackend(void)
{
	struct AudioMorseGen *amg;
	
	amg = (struct AudioMorseGen*)AllocMem(sizeof(struct AudioMorseGen), MEMF_ANY | MEMF_CLEAR);

	if (amg)
	{
		amg->mg.mg_Setup = AudioSetup;
		amg->mg.mg_Cleanup = AudioCleanup;
		amg->mg.mg_IntraSymbolPause = AudioIntraSymbolPause;
		amg->mg.mg_InterSymbolPause = AudioInterSymbolPause;
		amg->mg.mg_InterWordPause = AudioInterWordPause;
		amg->mg.mg_ShortTone = AudioShortTone;
		amg->mg.mg_LongTone = AudioLongTone;
	}

	return &amg->mg;
}
