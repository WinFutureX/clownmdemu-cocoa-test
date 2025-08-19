#ifndef AUDIO_H
#define AUDIO_H

#include <AudioUnit/AudioUnit.h>

#include "common/mixer.h"

#define SAMPLE_BUF_SIZE (MIXER_MAXIMUM_AUDIO_FRAMES_PER_FRAME * 5) + 1

typedef struct audio
{
	Mixer_State mixer;
	cc_bool pal;
	cc_bool has_mixer;
	cc_bool has_output;
	cc_bool paused;
	AudioUnit au;
	uint32_t samples_buf[SAMPLE_BUF_SIZE];
	unsigned int samples_head;
	unsigned int samples_tail;
} audio;

cc_s16l * mixer_allocate_fm(Mixer_State * mixer, size_t frames);
cc_s16l * mixer_allocate_psg(Mixer_State * mixer, size_t frames);
cc_s16l * mixer_allocate_pcm(Mixer_State * mixer, size_t frames);
cc_s16l * mixer_allocate_cdda(Mixer_State * mixer, size_t frames);

void audio_initialize(audio * a);
void audio_mixer_initialize(audio * a, cc_bool pal);
void audio_mixer_begin(audio * a);
void audio_mixer_end(audio * a);
void audio_shutdown(audio * a);

cc_bool audio_unit_initialize(audio * a, cc_bool pal);
void audio_unit_shutdown(audio * a);

cc_bool samples_put(audio * a, cc_s16l l, cc_s16l r);
cc_bool samples_get(audio * a, uint32_t * out);

#endif /* AUDIO_H */
