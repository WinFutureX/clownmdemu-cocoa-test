#ifndef AUDIO_H
#define AUDIO_H

#include <AudioToolbox/AudioToolbox.h>

#include "common/mixer.h"

typedef struct audio
{
	Mixer_State mixer;
	cc_bool shutdown;
	cc_bool done;
	cc_bool pal;
	cc_bool has_mixer;
	cc_bool has_queue;
	cc_bool paused;
	AudioQueueRef queue;
	cc_s16l samples[MIXER_MAXIMUM_AUDIO_FRAMES_PER_FRAME * MIXER_CHANNEL_COUNT];
	unsigned int bytes;
}
audio;

cc_s16l * mixer_allocate_fm(Mixer_State * mixer, size_t frames);
cc_s16l * mixer_allocate_psg(Mixer_State * mixer, size_t frames);
cc_s16l * mixer_allocate_pcm(Mixer_State * mixer, size_t frames);
cc_s16l * mixer_allocate_cdda(Mixer_State * mixer, size_t frames);

void audio_initialize(audio * a);
cc_bool audio_queue_initialize(audio * a, cc_bool pal);
void audio_mixer_initialize(audio * a, cc_bool pal);
void audio_mixer_begin(audio * a);
void audio_mixer_end(audio * a);
void audio_queue_shutdown(audio * a);
void audio_shutdown(audio * a);

#endif
