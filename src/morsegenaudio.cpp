#include <proto/utility.h>
#include <proto/dos.h>
#include <math.h>

#include "main.h"
#include "morsegenaudio.h"

#define TWO_PI 6.28318531f

char* AudioMorseGen::modname_argcheck = "arguments check";

AudioMorseGen::AudioMorseGen(long srate, long pitch, long wpm, long rwpm, long attack,
long release, long *metrics) : MorseGen()
{
	SampleRate = srate;
	TonePitch = pitch;
	Wpm = wpm;
	RealWpm = rwpm;
	Metrics = metrics;
	AttackTime = attack;
	ReleaseTime = release;

	if (RealWpm == 0) RealWpm = Wpm;

	if ((TonePitch < 100) || (TonePitch > 8000))
	{
		Printf("%s: tone pitch out of range (100 - 8000 Hz)\n", modname_argcheck);
		return;
	}

	if ((TonePitch << 2) > SampleRate)
	{
		Printf("%s: tone pitch must not be higher than 1/4 of sampling rate\n", modname_argcheck);
		return;
	}

	if ((Wpm < 5) || (Wpm > 100))
	{
		Printf("%s: WPM out of range (5 - 100)\n", modname_argcheck);
		return;
	}

	if ((RealWpm < 5) || (RealWpm > 100))
	{
		Printf("%s: real WPM out of range (5 - 100)\n", modname_argcheck);
		return;
	}

	if (RealWpm > Wpm)
	{
		Printf("%s: real WPM must not be higher than symbol WPM\n", modname_argcheck);
		return;
	}

	if ((AttackTime < 0) || (AttackTime > 50))
	{
		Printf("%s: attack time out of range (0 - 50 ms)\n", modname_argcheck);
		return;
	}

	if ((ReleaseTime < 0) || (ReleaseTime > 50))
	{
		Printf("%s: release time out of range (0 - 50 ms)\n", modname_argcheck);
		return;
	}

	SamplesPerDot = (SampleRate * 6) / (Wpm * 5);
	CalculateFarnsworthPauses();
	Initialized = TRUE; 

}

/*--------------------------------------------------------------------------------------------*/

long AudioMorseGen::CalculateAudioSize()
{
	long samples;
	long samples_per_dash = SamplesPerDot * 3;

	samples = Metrics[COUNTER_DOTS] * SamplesPerDot;
	samples += Metrics[COUNTER_DASHES] * samples_per_dash;
	samples += Metrics[COUNTER_SYMBOL_PAUSES] * SamplesPerDot;
	samples += Metrics[COUNTER_CHAR_PAUSES] * SamplesPerCPause;
	samples += Metrics[COUNTER_WORD_PAUSES] * SamplesPerWPause;
	return samples;
}

/*--------------------------------------------------------------------------------------------*/
/* Note that MorsConv knows Morse code metrics in advance, so it could fit RealWpm perfectly. */
/* However it calculates Farnsworth timing based on PARIS word, the same as ARRL paper.       */
/*--------------------------------------------------------------------------------------------*/

void AudioMorseGen::CalculateFarnsworthPauses()
{
	long alpha, beta, cpauseplus, wpauseplus;

	alpha = SampleRate * 60 * (Wpm - RealWpm);	
	beta = mul16(mul16(Wpm, RealWpm), 19);
	cpauseplus = alpha * 3 / beta;
	wpauseplus = alpha * 7 / beta;
	SamplesPerCPause = SamplesPerDot * 3 + cpauseplus;
	SamplesPerWPause = SamplesPerDot * 7 + wpauseplus;
}

/*--------------------------------------------------------------------------------------------*/

void AudioMorseGen::GenerateTone8(signed char *buffer, long count, long samprate, long pitch)
{
	long i;
	float step;

	step = TWO_PI / samprate * pitch;
 
	for (i = 0; i < count; i++)
	{
		float x;

		x = sinf(i * step) * 127.0f;
		*buffer++ = (signed char)x;
	}
}

/*--------------------------------------------------------------------------------------------*/

void AudioMorseGen::GenerateTone16(short *buffer, long count, long samprate, long pitch)
{
	long i;
	float step;

	step = TWO_PI / samprate * pitch;
 
	for (i = 0; i < count; i++)
	{
		float x;

		x = sinf(i * step) * 32767.0f;
		*buffer++ = (short)x;
	}
}

/*--------------------------------------------------------------------------------------------*/

void AudioMorseGen::ApplyEnvelope8(signed char *buffer, long count, short attack, short release)
{
	short i;
	signed char *p, s;

	/*----------------------------------------------*/
	/* limit attack and release zones to audio size */
	/*----------------------------------------------*/

	if (attack > count) attack = count;
	if (release > count) release = count;

	/*--------------*/
	/* apply attack */
	/*--------------*/

	for (p = buffer, i = 0; i < attack; i++)
	{
		s = *p;
		*p++ = div16(mul16(s, i), attack);
	}

	/*---------------*/
	/* apply release */
	/*---------------*/

	for (p = buffer + count - release, i = release - 1; i >= 0; i--)
	{
		s = *p;
		*p++ = div16(mul16(s, i), release);
	}
}

/*--------------------------------------------------------------------------------------------*/

void AudioMorseGen::ApplyEnvelope16(short *buffer, long count, short attack, short release)
{
	short i;
	short *p, s;

	/*----------------------------------------------*/
	/* limit attack and release zones to audio size */
	/*----------------------------------------------*/

	if (attack > count) attack = count;
	if (release > count) release = count;

	/*--------------*/
	/* apply attack */
	/*--------------*/

	for (p = buffer, i = 0; i < attack; i++)
	{
		s = *p;
		*p++ = div16(mul16(s, i), attack);
	}

	/*---------------*/
	/* apply release */
	/*---------------*/

	for (p = buffer + count - release, i = release - 1; i >= 0; i--)
	{
		s = *p;
		*p++ = div16(mul16(s, i), release);
	}
}


/*--------------------------------------------------------------------------------------------*/

void AudioMorseGen::ByteSwap16(short *buffer, long count)
{
	unsigned short x, *p = (unsigned short*)buffer;
	long i;

	for (i = 0; i < count; i++)
	{
		x = *p;
		*p++ = bswap16(x);
	}	
}

/*--------------------------------------------------------------------------------------------*/


AudioMorseGen::~AudioMorseGen() {}

