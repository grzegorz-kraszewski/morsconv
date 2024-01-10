#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include "main.h"
#include "morsegen.h"

struct AudioMorseGen
{
	struct MorseGen mg;
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


static BOOL AudioSetup(struct MorseGen *mg, struct TagItem *taglist)
{
	struct AudioMorseGen *amg = (struct AudioMorseGen*)mg;

	ParseTags(amg, taglist);
	return TRUE;
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

	FreeMem(mg, sizeof(struct AudioMorseGen));
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
	
	return &amg->mg;
}
