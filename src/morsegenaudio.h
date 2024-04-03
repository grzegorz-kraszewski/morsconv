#ifndef MORSCONV_MORSEGENAUDIO_H
#define MORSCONV_MORSEGENAUDIO_H

#include <libraries/iffparse.h>
#include <utility/tagitem.h>

#include "main.h"
#include "morsegen.h"

class AudioMorseGen : public MorseGen
{
	static char* modname_argcheck; 

	protected:

	long SampleRate;
	long SamplesPerDot;
	long SamplesPerCPause;
	long SamplesPerWPause;
	long *Metrics;
	short TonePitch;
	short Wpm;
	short RealWpm;
	short AttackTime;
	short ReleaseTime;
	
	long CalculateAudioSize();
	void CalculateFarnsworthPauses();
	void GenerateTone8(signed char *buffer, long count, long samprate, long pitch);
	void ApplyEnvelope8(signed char *buffer, long count, short attack, short release);
	void GenerateTone16(short *buffer, long count, long samprate, long pitch);
	void ApplyEnvelope16(short *buffer, long count, short attack, short release);
	void ByteSwap16(short *buffer, long count);

	public:

	AudioMorseGen(long srate, long pitch, long wpm, long rwpm, long attack, long release, long *metrics);
	~AudioMorseGen();
};

#endif  /* MORSCONV_MORSEGENAUDIO_H */
