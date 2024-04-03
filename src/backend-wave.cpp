#include <proto/exec.h>
#include <proto/dos.h>
#include <libraries/iffparse.h>

#include "main.h"
#include "backend-wave.h"

struct WaveHeader
{
	long Riff;
	long RiffLen;
	long Wave;
	long Fmt;
	long FmtLen;
	short Format;
	short Channels;
	long SampleRate;
	long ByteRate;
	short BlockAlign;
	short BitsPerSample;
	long Data;
	long DataLen;
};

#define ID_RIFF MAKE_ID('R','I','F','F')
#define ID_WAVE MAKE_ID('W','A','V','E')
#define ID_FMT  MAKE_ID('f','m','t',' ')
#define ID_DATA MAKE_ID('d','a','t','a')

/*--------------------------------------------------------------------------------------------*/

WaveMorseGen::WaveMorseGen(long srate, long pitch, long wpm, long rwpm, long attack, long release,
const char* path, long *metrics) : AudioMorseGen(srate, pitch, wpm, rwpm, attack, release,
metrics)
{
	OutputPath = path;
	File = NULL;
	DotTone = NULL;

	if (!Initialized) return;

	Initialized = FALSE;

	if ((SampleRate < 1000) || (SampleRate > 192000))
	{
		PutStr("agruments check: sampling rate out of range (1000 - 192000 Hz)\n");
		return;
	}

	if (!OutputPath)
	{
		PutStr("arguments check: no output file specified (missing 'TO' argument)\n");
		return;
	}

	if (!PrepareBuffers()) return;
	if (!InitStream()) return;
	if (!WriteHeader(CalculateAudioSize())) return;

	Initialized = TRUE;
}

/*--------------------------------------------------------------------------------------------*/
    
short WaveMorseGen::InitStream()
{
	if (File = Open(OutputPath, MODE_NEWFILE)) return TRUE;
	PrintFault(IoErr(), "WAVE writer");
	return FALSE;
}

/*--------------------------------------------------------------------------------------------*/

short WaveMorseGen::PrepareBuffers()
{
	long attack_samples, release_samples, samples_per_dash;

	samples_per_dash = SamplesPerDot * 3;
	attack_samples = SampleRate * AttackTime / 1000;
	release_samples = SampleRate * ReleaseTime / 1000;

	if (DotTone = (short*)AllocVec((SamplesPerDot + samples_per_dash +
	SamplesPerWPause) * sizeof(short), MEMF_ANY | MEMF_CLEAR))
	{
		DashTone = DotTone + SamplesPerDot;
		Silence = DashTone + samples_per_dash;
		GenerateTone16(DotTone, SamplesPerDot, SampleRate, TonePitch);
		ApplyEnvelope16(DotTone, SamplesPerDot, attack_samples, release_samples);
		ByteSwap16(DotTone, SamplesPerDot);
		GenerateTone16(DashTone, samples_per_dash, SampleRate, TonePitch);
		ApplyEnvelope16(DashTone, samples_per_dash, attack_samples, release_samples);
		ByteSwap16(DashTone, samples_per_dash);
		return TRUE;
	}

	PutStr("audio generator: out of memory\n");
	return FALSE;
}

/*--------------------------------------------------------------------------------------------*/

short WaveMorseGen::WriteHeader(long samples)
{
	WaveHeader wh;
	long bytes = samples << 1;

	wh.Wave = ID_WAVE;
	wh.Data = ID_DATA;
	wh.DataLen = bswap32(bytes);
	wh.Fmt = ID_FMT;
	wh.FmtLen = 0x10000000; 
	wh.Riff = ID_RIFF;
	wh.RiffLen = bswap32(bytes + 36);
	wh.Format = 0x0100;
	wh.Channels = 0x0100;
	wh.SampleRate = bswap32(SampleRate);
	wh.ByteRate = bswap32(SampleRate << 1);
	wh.BlockAlign = 0x0200;
	wh.BitsPerSample = 0x1000;

	return FWrite(File, &wh, sizeof(WaveHeader), 1);
}

/*--------------------------------------------------------------------------------------------*/


WaveMorseGen::~WaveMorseGen()
{
	if (DotTone) FreeVec(DotTone);
	if (File) Close(File);
}

 