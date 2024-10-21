#include "main.h"
#include "morsegenaudio.h"

class AudioDevMorseGen : public AudioMorseGen
{
	signed char *DotTone;
	signed char *DashTone;
	signed char *Silence;
	
	short InitStream();
	short PrepareBuffers();

	public:

	AudioDevMorseGen(long srate, long pitch, long wpm, long rwpm, long attack, long release,
	 long *metrics);
	void IntraSymbolPause() {}
	void InterSymbolPause() {}
	void InterWordPause() {} 
	void ShortTone() {}
	void LongTone() {} 
	~AudioDevMorseGen();
};
