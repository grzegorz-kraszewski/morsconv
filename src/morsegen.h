#ifndef TELEGRAPHIST_MORSEGEN_H
#define TELEGRAPHIST_MORSEGEN_H

#include <utility/tagitem.h>


/****i* morsegen/MorseGen ****************************************************
*
* NAME
*   MorseGen -- Structure controlling Morse generator 
*
* SYNOPSIS
*   struct MorseGen
*   {
*      BOOL(*mg_Setup)(struct MorseGen*, struct TagItem*);
*      void(*mg_Cleanup)(struct MorseGen*);
*      void(*mg_IntraSymbolPause)(struct MorseGen*);
*      void(*mg_InterSymbolPause)(struct MorseGen*);
*      void(*mg_InterWordPause)(struct MorseGen*);
*      void(*mg_ShortTone)(struct MorseGen*);
*      void(*mg_LongTone)(struct MorseGen*);
*      BOOL mg_NeedsMetrics;
*   };
*
* FUNCTION
*   The structure is the master control structure of a Morse generator. The
*   application creates it and fills with data. Meaning of structure fields:
*   - mg_Setup, points to generator setup function. Should not be called
*     before each text, it is assumed to be called once in application init.
*   - mg_Cleanup, points to generator cleanup function, usually called once
*     at application exit.
*   - mg_IntraSymbolPause, is a callback called by generator for emitting
*     pauses inside a symbol.
*   - mg_InterSymbolPause, is a callback called by generator for emitting
*     pauses between symbols in a word.
*   - mg_InterWordPause, is a callback called by generator for emitting
*     pauses between words (spaces).
*   - mg_ShortTone, is a callback called by generator for emitting "dit"
*     tone.
*   - mg_LongTone, is a callback called by generator for emitting "dat" tone.
*   - mg_NeedsMetrics, boolean flag set to TRUE if given backend has to
*     precalculate its output size based on text to be converted later.
*
******************************************************************************
*/

struct MorseGen
{
	BOOL(*mg_Setup)(struct MorseGen*, struct TagItem*);
	void(*mg_Cleanup)(struct MorseGen*);
	void(*mg_IntraSymbolPause)(struct MorseGen*);
	void(*mg_InterSymbolPause)(struct MorseGen*);
	void(*mg_InterWordPause)(struct MorseGen*);
	void(*mg_ShortTone)(struct MorseGen*);
	void(*mg_LongTone)(struct MorseGen*);
	BOOL mg_NeedsMetrics;
};

LONG MorseText(struct MorseGen*, STRPTR);
BOOL MorseGenSetup(struct MorseGen*, Tag tag0, ...);

/*-----------------------------------*/
/* control tags for Morse generators */
/*-----------------------------------*/

#define MA_DotText         0x80000001   /* CON */
#define MA_DashText        0x80000002   /* CON */
#define MA_SamplingRate    0x80000003   /* 8SVX */
#define MA_TonePitch       0x80000004   /* 8SVX */
#define MA_WordsPerMinute  0x80000005   /* 8SVX */
#define MA_OutputFile      0x80000006   /* 8SVX */
#define MA_CounterStorage  0x80000007   /* COUNT */
#define MA_CounterPrint    0x80000008   /* COUNT */
#define MA_MorseMetrics    0x80000009   /* 8SVX */
#define MA_EnvAttack       0x8000000A   /* 8SVX */
#define MA_EnvRelease      0x8000000B   /* 8SVX */
#define MA_SymbolPause     0x8000000C   /* CON */
#define MA_CharPause       0x8000000D   /* CON */
#define MA_WordPause       0x8000000E   /* CON */

/* indexes for the counter array (count module) */

#define COUNTER_DOTS              0
#define COUNTER_DASHES            1
#define COUNTER_SYMBOL_PAUSES     2
#define COUNTER_CHAR_PAUSES       3
#define COUNTER_WORD_PAUSES       4


#endif   /* TELEGRAPHIST_MORSEGEN_H */
