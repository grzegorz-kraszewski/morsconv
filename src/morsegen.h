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
};

LONG MorseText(struct MorseGen*, STRPTR);
BOOL MorseGenSetup(struct MorseGen*, Tag tag0, ...);

#endif   /* TELEGRAPHIST_MORSEGEN_H */