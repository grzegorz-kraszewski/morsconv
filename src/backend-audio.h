#include "main.h"
#include "morsegenaudio.h"
#include <devices/audio.h>


enum AudioStatus { FAILED, UNUSED, FREE, PLAYING };
enum AudioChannel { LEFT, RIGHT };

#define AUDIO_BUFSIZE 16384

#define LEFT_CHANNELS_MASK    6
#define RIGHT_CHANNELS_MASK   9


// TODO: audio in both stereo channels
// TODO: CTRL+C handling

class AudioBuffer
{
	public:

	IOAudio request;
	signed char *audio;
	AudioChannel channel;
	AudioStatus status;
	LONG level;

	AudioBuffer(MsgPort *port);
	~AudioBuffer();
	void Initialize(IOAudio *master, LONG per);
	bool Full() { return level == AUDIO_BUFSIZE; }
	bool Available() { return (status == UNUSED) || (status == FREE); }
	void SetFree() { status = FREE; level = 0; }
};


class AudioDevMorseGen : public AudioMorseGen
{
	signed char *dotTone;
	signed char *dashTone;
	signed char *silence;
	IOAudio *request;       // opening device only, it is never sent
	AudioBuffer *bufferA;
	AudioBuffer *bufferB;
	MsgPort *port;
	LONG leftChannel;
	LONG rightChannel;
	LONG period;
	
	void WaitForBuffers();
	short PrepareBuffers();
	void PushAudio(LONG samples, signed char *source);
	AudioBuffer* GetFreeBuffer();

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
