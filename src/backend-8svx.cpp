#include <proto/exec.h>
#include <proto/dos.h>

#include "main.h"
#include "backend-8svx.h"
#include "backend-count.h"

#define ID_8SVX MAKE_ID('8','S','V','X')
#define ID_VHDR MAKE_ID('V','H','D','R')
#define ID_BODY MAKE_ID('B','O','D','Y')

struct Vhdr
{
	long OneShotHiSamples;
	long RepeatHiSamples;
	long SamplesPerHiCycle;
	unsigned short SamplingRate;
	unsigned char Octave;
	unsigned char Compression;
	long Volume;
};

/*--------------------------------------------------------------------------------------------*/

SvxMorseGen::SvxMorseGen(long srate, long pitch, long wpm, long rwpm, long attack, long release,
const char* path, long *metrics) : AudioMorseGen(srate, pitch, wpm, rwpm, attack, release,
metrics)
{
	OutputPath = path;
	File = NULL;
	Iff = NULL;
	IffOpened = FALSE;
	DotTone = NULL;
	FormPushed = FALSE;
	BodyPushed = FALSE;

	if (!Initialized) return;

	Initialized = FALSE;

	if ((SampleRate < 1000) || (SampleRate > 65535))
	{
		PutStr("agruments check: sampling rate out of range (1000 - 65535 Hz)\n");
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
    
short SvxMorseGen::InitStream()
{
	if (File = Open(OutputPath, MODE_NEWFILE))
	{
		if (Iff = AllocIFF())
		{
			long ifferr;
			Iff->iff_Stream = File;
			InitIFFasDOS(Iff);
	
			if ((ifferr = OpenIFF(Iff, IFFF_WRITE)) == 0)
			{
				IffOpened = TRUE;
				return TRUE;
			}
			else Printf("IFF writer: iffparse error %ld\n", ifferr);
		}
		else PutStr("IFF writer: out of memory\n");
	}
	else PrintFault(IoErr(), "IFF writer");

	return FALSE;
}

/*--------------------------------------------------------------------------------------------*/

short SvxMorseGen::PrepareBuffers()
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

/*--------------------------------------------------------------------------------------------*/

short SvxMorseGen::WriteHeader(long samples)
{
	struct Vhdr h;

	h.OneShotHiSamples = samples;
	h.RepeatHiSamples = 0;
	h.SamplesPerHiCycle = 0;
	h.SamplingRate = (unsigned short)SampleRate;
	h.Octave = 1;
	h.Compression = 0;
	h.Volume = 0x10000;

	if (PushChunk(Iff, ID_8SVX, ID_FORM, IFFSIZE_UNKNOWN) == 0)
	{
		FormPushed = TRUE;

		if (PushChunk(Iff, ID_8SVX, ID_VHDR, sizeof(Vhdr)) == 0)
		{
			if (WriteChunkBytes(Iff, &h, sizeof(Vhdr)) == sizeof(Vhdr))
			{
				if (PopChunk(Iff) == 0)
				{
					if (PushChunk(Iff, ID_8SVX, ID_BODY, samples) == 0)
					{
						BodyPushed = TRUE;
						return TRUE; 
					}
				}
			}
		}
	}

	return FALSE;
}

/*--------------------------------------------------------------------------------------------*/


SvxMorseGen::~SvxMorseGen()
{
	if (BodyPushed) PopChunk(Iff);
	if (FormPushed) PopChunk(Iff);
	if (DotTone) FreeVec (DotTone);
	if (IffOpened) CloseIFF(Iff);
	if (Iff) FreeIFF(Iff);
	if (File) Close(File);
}

 