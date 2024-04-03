#include <proto/iffparse.h>

#include "main.h"
#include "morsegenaudio.h"

class SvxMorseGen : public AudioMorseGen
{
	const char *OutputPath;
	IFFHandle *Iff;
	BPTR File;
	signed char *DotTone;
	signed char *DashTone;
	signed char *Silence;
	short IffOpened;
	short FormPushed;
	short BodyPushed;
	
	short InitStream();
	short PrepareBuffers();
	short WriteHeader(long samples);

	public:

	SvxMorseGen(long srate, long pitch, long wpm, long rwpm, long attack, long release, const char* path,
		long *metrics);
	void IntraSymbolPause() { WriteChunkBytes(Iff, Silence, SamplesPerDot); }
	void InterSymbolPause() { WriteChunkBytes(Iff, Silence, SamplesPerCPause); }
	void InterWordPause() { WriteChunkBytes(Iff, Silence, SamplesPerWPause); } 
	void ShortTone() { WriteChunkBytes(Iff, DotTone, SamplesPerDot); }
	void LongTone() { WriteChunkBytes(Iff, DashTone, SamplesPerDot * 3); } 
	~SvxMorseGen();
};
