#include <proto/exec.h>
#include <proto/dos.h>

#include "main.h"
#include "backend-audio.h"


//=============================================================================================
// AudioBuffer::AudioBuffer()
//=============================================================================================

AudioBuffer::AudioBuffer(MsgPort *port) : status(AUDIO_FAILED), audio(NULL), level(0)
{
	if (audio = (signed char*)AllocMem(AUDIO_BUFSIZE, MEMF_CHIP))
	{
		status = AUDIO_UNUSED;
	}
	else PutStr("audio generator: out of memory\n");
}

//=============================================================================================
// AudioBuffer::~AudioBuffer()
//=============================================================================================

AudioBuffer::~AudioBuffer()
{
	if (status & AUDIO_PLAY_L)
	{
		AbortIO((IORequest*)&requestL);
		WaitIO((IORequest*)&requestL);
	}

	if (status & AUDIO_PLAY_R)
	{
		AbortIO((IORequest*)&requestR);
		WaitIO((IORequest*)&requestR);
	}

	if (audio) FreeMem(audio, AUDIO_BUFSIZE);
}

//=============================================================================================
// AudioBuffer::Initialize()
//=============================================================================================

void AudioBuffer::Initialize(IOAudio *master, LONG per)
{
	CopyMem(master, &requestL, sizeof(IOAudio));
	requestL.ioa_Volume = 64;
	requestL.ioa_Period = per;
	requestL.ioa_Cycles = 1;
	requestL.ioa_Data = (UBYTE*)audio;
	requestL.ioa_Length = AUDIO_BUFSIZE;
	requestL.ioa_Request.io_Flags = ADIOF_PERVOL;
	requestL.ioa_Request.io_Command = CMD_WRITE;
	CopyMem(&requestL, &requestR, sizeof(IOAudio));

	requestL.ioa_Request.io_Unit = (Unit*)((ULONG)requestL.ioa_Request.io_Unit &
		LEFT_CHANNELS_MASK);
	requestR.ioa_Request.io_Unit = (Unit*)((ULONG)requestR.ioa_Request.io_Unit &
		RIGHT_CHANNELS_MASK);
}

//=============================================================================================
// BeginIO()
//=============================================================================================

static void BeginIO(IOAudio *req)
{
	register IORequest *ioRequest asm("a1") = &req->ioa_Request;
	register Library *deviceBase asm("a6") = &req->ioa_Request.io_Device->dd_Library;

	asm("MOVEA.L %1,a1; JSR %0@(-30);" : : "a"(deviceBase), "a"(ioRequest) : "a1");
}

//=============================================================================================
// AudioDevMorseGen::AudioDevMorseGen()
//=============================================================================================

AudioDevMorseGen::AudioDevMorseGen(long srate, long pitch, long wpm, long rwpm, long attack,
 long release) : AudioMorseGen(srate, pitch, wpm, rwpm, attack, release, NULL), port(NULL),
 request(NULL), bufferA(NULL), bufferB(NULL)
{
	UBYTE channelPatterns[4] = { 3, 5, 10, 12 };   // one left + one right
	LONG masterClock = 3546895;                    // PAL

	dotTone = NULL;

	if (!Initialized) return;

	Initialized = FALSE;

	if ((SampleRate < 1000) || (SampleRate > 65535))
	{
		PutStr("agruments check: sampling rate out of range (1000 - 65535 Hz)\n");
		return;
	}

	if (!PrepareBuffers()) return;

	if (port = CreateMsgPort())
	{
		if (request = (IOAudio*)CreateIORequest(port, sizeof(IOAudio)))
		{
			request->ioa_AllocKey = 0;
			request->ioa_Request.io_Message.mn_Node.ln_Pri = 120;   // audio priority
			request->ioa_Data = channelPatterns;
			request->ioa_Length = sizeof(channelPatterns);

			if ((bufferA = new AudioBuffer(port)) && (bufferA->status == AUDIO_UNUSED))
			{
				if ((bufferB = new AudioBuffer(port)) && (bufferB->status == AUDIO_UNUSED))
				{
					if (OpenDevice("audio.device", 0, (IORequest*)request, 0) == 0)
					{
						if (((ExecBase*)SysBase)->VBlankFrequency == 60) masterClock =  3579545;   // NTSC
						period = divu16(masterClock, SampleRate);
						bufferA->Initialize(request, period);
						bufferB->Initialize(request, period);
						StopChannels();
						Initialized = TRUE;
					}
					else PutStr("audio generator: can't open audio.device.\n");
				}
				else PutStr("audio generator: out of memory\n");
			}
			else PutStr("audio generator: out of memory\n");
		}
		else PutStr("audio generator: out of memory\n");
	}
	else PutStr("audio generator: message port creation failed\n");
}

//=============================================================================================
// AudioDevMorseGen::~AudioDevMorseGen()
//=============================================================================================

AudioDevMorseGen::~AudioDevMorseGen()
{
	if (bufferA) delete bufferA;
	if (bufferB) delete bufferB;
	if (Initialized) CloseDevice((IORequest*)request);
	if (request) DeleteIORequest(request);
	if (port) DeleteMsgPort(port);
}

//=============================================================================================
// AudioDevMorseGen::StopChannels()
//---------------------------------------------------------------------------------------------
// Preparation for synchronous start of stereo channels. It uses the master IOAudio in
// synchronous manner.
//=============================================================================================

void AudioDevMorseGen::StopChannels()
{
	request->ioa_Request.io_Command = CMD_STOP;
	request->ioa_Request.io_Flags = IOF_QUICK;
	BeginIO(request);
	started = FALSE;
}

//=============================================================================================
// AudioDevMorseGen::StartChannels()
//---------------------------------------------------------------------------------------------
// Preparation for synchronous start of stereo channels. It uses the master IOAudio in
// synchronous manner.
//=============================================================================================

void AudioDevMorseGen::StartChannels()
{
	request->ioa_Request.io_Command = CMD_START;
	request->ioa_Request.io_Flags = IOF_QUICK;
	BeginIO(request);
	started = TRUE;
}

//=============================================================================================
// AudioDevMorseGen::PrepareBuffers()
//=============================================================================================

short AudioDevMorseGen::PrepareBuffers()
{
	LONG attack_samples, release_samples, samples_per_dash;

	samples_per_dash = SamplesPerDot * 3;
	attack_samples = SampleRate * AttackTime / 1000;
	release_samples = SampleRate * ReleaseTime / 1000;

	if (dotTone = (signed char*)AllocVec(SamplesPerDot + samples_per_dash + SamplesPerWPause,
	MEMF_ANY | MEMF_CLEAR))
	{
		dashTone = dotTone + SamplesPerDot;
		silence = dashTone + samples_per_dash;
		GenerateTone8(dotTone, SamplesPerDot, SampleRate, TonePitch);
		ApplyEnvelope8(dotTone, SamplesPerDot, attack_samples, release_samples);
		GenerateTone8(dashTone, samples_per_dash, SampleRate, TonePitch);
		ApplyEnvelope8(dashTone, samples_per_dash, attack_samples, release_samples);
		return TRUE;
	}

	PutStr("audio generator: out of memory.\n");
	return FALSE;
}

//=============================================================================================
// SendBuffer()
//=============================================================================================

void AudioDevMorseGen::SendBuffer(AudioBuffer *buffer)
{
	buffer->requestL.ioa_Length = buffer->level;
	buffer->requestR.ioa_Length = buffer->level;
	BeginIO(&buffer->requestL);
	BeginIO(&buffer->requestR);
	if (!started) StartChannels();
	buffer->status = AUDIO_PLAY;
	buffer->level = 0;
}

//=============================================================================================
// AudioDevMorseGen::PushAudio()
//=============================================================================================

void AudioDevMorseGen::PushAudio(LONG samples, signed char *source)
{
	LONG samplesToCopy, freeSpace, sourcePosition = 0;
	AudioBuffer *buffer;

	while (sourcePosition < samples)
	{
		buffer = GetFreeBuffer();
		freeSpace = AUDIO_BUFSIZE - buffer->level;
		samplesToCopy = samples - sourcePosition;
		if (samplesToCopy > freeSpace) samplesToCopy = freeSpace;
		CopyMem(source + sourcePosition, buffer->audio + buffer->level, samplesToCopy);
		buffer->level += samplesToCopy;
		sourcePosition += samplesToCopy;
		if (buffer->Full()) SendBuffer(buffer);
	}
}

//=============================================================================================
// AudioDevMorseGen::WaitForBuffers()
//=============================================================================================

void AudioDevMorseGen::WaitForBuffers()
{
	IOAudio *returnedReq;
	ULONG signals, portmask = 1 << port->mp_SigBit;

	signals = Wait(portmask | SIGBREAKF_CTRL_C);

	if (signals & portmask)
	{
		while (returnedReq = (IOAudio*)GetMsg(port))
		{
			if (returnedReq == &bufferA->requestL) bufferA->status &= ~AUDIO_PLAY_L;
			else if (returnedReq == &bufferA->requestR) bufferA->status &= ~AUDIO_PLAY_R;
			else if (returnedReq == &bufferB->requestL) bufferB->status &= ~AUDIO_PLAY_L;
			else if (returnedReq == &bufferB->requestR) bufferB->status &= ~AUDIO_PLAY_R;
		}
	}

	if (signals & SIGBREAKF_CTRL_C)
	{
		PutStr("Break! Waiting for audio to complete...\n");
		Stop = TRUE;
	}
}

//=============================================================================================
// AudioDevMorseGen::GetFreeBuffer()
//=============================================================================================

AudioBuffer* AudioDevMorseGen::GetFreeBuffer()
{
	AudioBuffer *buffer = NULL;

	while (!buffer)
	{
		if (bufferA->Available()) buffer = bufferA;
		else if (bufferB->Available()) buffer = bufferB;
		else WaitForBuffers();
	}

	return buffer;
}


//=============================================================================================
// AudioDevMorseGen::FlushBuffers()
//=============================================================================================

void AudioDevMorseGen::FlushBuffers()
{
	AudioBuffer *buffer;

	//---------------------------------------
	// send out partially filled last buffer 
	//---------------------------------------

	if ((bufferA->Available()) && (bufferA->level > 0)) SendBuffer(bufferA);
	else if ((bufferB->Available()) && (bufferB->level > 0)) SendBuffer(bufferB);

	//---------------------------------------
	// wait for any playing buffer to finish
	//---------------------------------------

	while (!(bufferA->Available() && bufferB->Available()))
	{
		WaitForBuffers();
	}
}
