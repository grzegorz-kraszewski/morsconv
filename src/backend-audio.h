#include "main.h"
#include "morsegenaudio.h"
#include <devices/audio.h>

#define AUDIO_UNUSED    4
#define AUDIO_PLAY_R    2
#define AUDIO_PLAY_L    1
#define AUDIO_PLAY      3
#define AUDIO_FREE      0
#define AUDIO_FAILED    8

#define AUDIO_BUFSIZE 8192

#define LEFT_CHANNELS_MASK    6
#define RIGHT_CHANNELS_MASK   9


// TODO: audio in both stereo channels
// TODO: CTRL+C handling

class AudioBuffer
{
	public:

	IOAudio requestL;
	IOAudio requestR;
	signed char *audio;
	ULONG status;
	LONG level;

	AudioBuffer(MsgPort *port);
	~AudioBuffer();
	void Initialize(IOAudio *master, LONG per);
	bool Full() { return (level == AUDIO_BUFSIZE); }
	bool Available() { return ((status & AUDIO_PLAY) == 0); }
};


class AudioDevMorseGen : public AudioMorseGen
{
	signed char *dotTone;
	signed char *dashTone;
	signed char *silence;
	IOAudio *request;       // opening device, stop, start
	AudioBuffer *bufferA;
	AudioBuffer *bufferB;
	MsgPort *port;
	LONG period;
	LONG started;

	void WaitForBuffers();
	short PrepareBuffers();
	void PushAudio(LONG samples, signed char *source);
	AudioBuffer* GetFreeBuffer();
	void StopChannels();
	void StartChannels();
	void SendBuffer(AudioBuffer *buffer);

	public:

	AudioDevMorseGen(long srate, long pitch, long wpm, long rwpm, long attack, long release);
	virtual void IntraSymbolPause() { PushAudio(SamplesPerDot, silence); }
	virtual void InterSymbolPause() { PushAudio(SamplesPerCPause, silence); }
	virtual void InterWordPause() { PushAudio(SamplesPerWPause, silence); }
	virtual void ShortTone() { PushAudio(SamplesPerDot, dotTone); }
	virtual void LongTone() { PushAudio(SamplesPerDot * 3, dashTone); }
	virtual void FlushBuffers();
	virtual ~AudioDevMorseGen();
};
