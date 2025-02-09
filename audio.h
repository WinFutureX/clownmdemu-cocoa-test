#ifndef AUDIO_H
#define AUDIO_H

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>

#define MIXER_FORMAT cc_s16l
#include "clownmdemu-frontend-common/mixer.h"

typedef struct audio
{
	Mixer_Constant constant;
	Mixer_State state;
	Mixer mixer;
	cc_bool shutdown;
	cc_bool done;
	cc_bool pal;
	cc_bool has_mixer;
	cc_bool has_queue;
	cc_bool paused;
	AudioQueueRef queue;
	MIXER_FORMAT samples[1920];
	unsigned int bytes;
}
audio;

MIXER_FORMAT * mixer_allocate_fm(Mixer * mixer, size_t frames);
MIXER_FORMAT * mixer_allocate_psg(Mixer * mixer, size_t frames);
MIXER_FORMAT * mixer_allocate_pcm(Mixer * mixer, size_t frames);
MIXER_FORMAT * mixer_allocate_cdda(Mixer * mixer, size_t frames);

audio * audio_alloc();
cc_bool audio_queue_initialize(audio * a, cc_bool pal);
void audio_initialize(audio * a, cc_bool pal);
void audio_begin(audio * a);
void audio_end(audio * a);
void audio_queue_shutdown(audio * a);
void audio_shutdown(audio * a);

#endif
