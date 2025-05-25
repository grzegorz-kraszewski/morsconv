#include <proto/dos.h>

#include "main.h"
#include "morsegenaudio.h"

class WaveMorseGen : public AudioMorseGen
{
	const char *OutputPath;
	BPTR File;
	short *DotTone;    /* byteswapped */
	short *DashTone;   /* byteswapped */
	short *Silence;    /* byteswapped ;-) */

	short InitStream();
	short PrepareBuffers();
	short WriteHeader(long samples);

	public:

	WaveMorseGen(long srate, long pitch, long wpm, long rwpm, long attack, long release, const char* path,
		long *metrics);
	virtual void IntraSymbolPause() { FWrite(File, Silence, SamplesPerDot << 1, 1); }
	virtual void InterSymbolPause() { FWrite(File, Silence, SamplesPerCPause << 1, 1); }
	virtual void InterWordPause() { FWrite(File, Silence, SamplesPerWPause << 1, 1); }
	virtual void ShortTone() { FWrite(File, DotTone, SamplesPerDot << 1, 1); }
	virtual void LongTone() { FWrite(File, DashTone, SamplesPerDot * 6, 1); }
	~WaveMorseGen();
};
