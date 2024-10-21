#include <proto/exec.h>
#include <proto/dos.h>

#include "main.h"
#include "backend-audio.h"

//----------------------------------------------------------------------------------------------

AudioDevMorseGen::AudioDevMorseGen(long srate, long pitch, long wpm, long rwpm, long attack,
 long release, long *metrics) : AudioMorseGen(srate, pitch, wpm, rwpm, attack, release,
 metrics)
{
	DotTone = NULL;

	if (!Initialized) return;

	Initialized = FALSE;

	if ((SampleRate < 1000) || (SampleRate > 65535))
	{
		PutStr("agruments check: sampling rate out of range (1000 - 65535 Hz)\n");
		return;
	}

	if (!PrepareBuffers()) return;

	Initialized = TRUE;
}

//----------------------------------------------------------------------------------------------
    
short AudioDevMorseGen::PrepareBuffers()
{
	LONG attack_samples, release_samples, samples_per_dash;

	samples_per_dash = SamplesPerDot * 3;
	attack_samples = SampleRate * AttackTime / 1000;
	release_samples = SampleRate * ReleaseTime / 1000;

	if (DotTone = (signed char*)AllocVec(SamplesPerDot + samples_per_dash + SamplesPerWPause,
	MEMF_ANY | MEMF_CLEAR))
	{
		DashTone = DotTone + SamplesPerDot;
		Silence = DashTone + samples_per_dash;
		GenerateTone8(DotTone, SamplesPerDot, SampleRate, TonePitch);
		ApplyEnvelope8(DotTone, SamplesPerDot, attack_samples, release_samples);
		GenerateTone8(DashTone, samples_per_dash, SampleRate, TonePitch);
		ApplyEnvelope8(DashTone, samples_per_dash, attack_samples, release_samples);
		return TRUE;
	}

	PutStr("audio generator: out of memory\n");
	return FALSE;
}

//----------------------------------------------------------------------------------------------

AudioDevMorseGen::~AudioDevMorseGen()
{
}

 