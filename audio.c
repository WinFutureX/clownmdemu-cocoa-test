#include <stdint.h>
#define MIXER_IMPLEMENTATION
#include "audio.h"
#include "frontend_log.h"

// 3732 frames for 60hz, 4433 for 50hz
void mixer_callback(void * data, const cc_s16l * samples, size_t frames)
{
	// in pal mode, this is called several times with frames set to 0 for some reason???
	// we'll ignore such cases so the audio queue callback won't be fed an empty buffer
	// thus we'll get working audio output under pal mode just like ntsc
	if (frames > 0 || frames <= MIXER_MAXIMUM_AUDIO_FRAMES_PER_FRAME)
	{
		audio * a = (audio *) data;
		for (int i = 0; i < frames; i++)
		{
			cc_s16l l = *samples++;
			cc_s16l r = *samples++;
			while(!samples_put(a, l, r));
		}
	}
}

cc_s16l * mixer_allocate_fm(Mixer_State * mixer, size_t frames)
{
	return Mixer_AllocateFMSamples(mixer, frames);
}

cc_s16l * mixer_allocate_psg(Mixer_State * mixer, size_t frames)
{
	return Mixer_AllocatePSGSamples(mixer, frames);
}

cc_s16l * mixer_allocate_pcm(Mixer_State * mixer, size_t frames)
{
	return Mixer_AllocatePCMSamples(mixer, frames);
}

cc_s16l * mixer_allocate_cdda(Mixer_State * mixer, size_t frames)
{
	return Mixer_AllocateCDDASamples(mixer, frames);
}

void audio_initialize(audio * a)
{
	a->has_mixer = cc_false;
	a->has_output = cc_false;
}

void audio_mixer_initialize(audio * a, cc_bool pal)
{
	if (a->has_output == cc_true) audio_unit_shutdown(a);
	if (a->has_mixer == cc_true) Mixer_Deinitialise(&a->mixer);
	a->pal = pal;
	a->has_mixer = Mixer_Initialise(&a->mixer, a->pal) == cc_true ? cc_true : cc_false;
	a->has_output = audio_unit_initialize(a, pal);
	a->paused = cc_false;
}

void audio_mixer_begin(audio * a)
{
	if (a->has_mixer == cc_true) Mixer_Begin(&a->mixer);
}

void audio_mixer_end(audio * a)
{
	if (a->has_mixer == cc_true) Mixer_End(&a->mixer, mixer_callback, a);
}

void audio_shutdown(audio * a)
{
	if (a->has_output == cc_true) audio_unit_shutdown(a);
	Mixer_Deinitialise(&a->mixer);
	a->has_mixer = cc_false;
}

OSStatus audio_unit_callback(void * data, AudioUnitRenderActionFlags * flags, const AudioTimeStamp * time_stamp, UInt32 bus, UInt32 frames, AudioBufferList * buf_list)
{
	audio * a = (audio *) data;
	SInt16 * output = (SInt16 *) buf_list->mBuffers[0].mData;
	for (int i = 0; i < frames; i++)
	{
		uint32_t comb;
		UInt32 out = i * 2;
		if (samples_get(a, &comb) == cc_true)
		{
			SInt16 l = (comb & 0xFFFF0000) >> 16;
			SInt16 r = comb;
			output[out] = l;
			output[out + 1] = r;
		}
		else
		{
			output[out] = 0;
			output[out + 1] = 0;
		}
	}
	return noErr;
}

cc_bool audio_unit_initialize(audio * a, cc_bool pal)
{
	OSErr err;
	AudioComponentDescription acd;
	acd.componentType = kAudioUnitType_Output;
	acd.componentSubType = kAudioUnitSubType_DefaultOutput;
	acd.componentManufacturer = kAudioUnitManufacturer_Apple;
	AudioComponent c = AudioComponentFindNext(NULL, &acd);
	if (!c)
	{
		frontend_err("AudioComponentFindNext failed\n");
		return cc_false;
	}
	err = AudioComponentInstanceNew(c, &a->au);
	if (err)
	{
		frontend_err("AudioComponentInstanceNew failed: %d\n", err);
		return cc_false;
	}
	AURenderCallbackStruct aurcs;
	aurcs.inputProc = audio_unit_callback;
	aurcs.inputProcRefCon = a;
	err = AudioUnitSetProperty(a->au, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &aurcs, sizeof(aurcs));
	if (err)
	{
		frontend_err("AudioUnitSetProperty failed for kAudioUnitProperty_SetRenderCallback: %d\n", err);
		return cc_false;
	}
	AudioStreamBasicDescription asbd;
	asbd.mFormatID = kAudioFormatLinearPCM;
	asbd.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	//asbd.mSampleRate = pal == cc_true ? 221650.0f : 223920.0f;
	asbd.mSampleRate = MIXER_OUTPUT_SAMPLE_RATE;
	asbd.mBitsPerChannel = 16;
	asbd.mChannelsPerFrame = 2;
	asbd.mFramesPerPacket = 1;
	asbd.mBytesPerFrame = 4;
	asbd.mBytesPerPacket = 4;
	err = AudioUnitSetProperty(a->au, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, sizeof(asbd));
	if (err)
	{
		frontend_err("AudioUnitSetProperty failed for kAudioUnitProperty_StreamFormat: %d\n", err);
		return cc_false;
	}
	err = AudioUnitInitialize(a->au);
	if (err)
	{
		frontend_err("AudioUnitInitialize failed: %d\n", err);
		return cc_false;
	}
	AudioOutputUnitStart(a->au);
	return cc_true;
}

void audio_unit_shutdown(audio * a)
{
	AudioOutputUnitStop(a->au);
	AudioUnitUninitialize(a->au);
	AudioComponentInstanceDispose(a->au);
}

cc_bool samples_put(audio * a, cc_s16l l, cc_s16l r)
{
	uint32_t tmp = (uint32_t) (l << 16) | (uint32_t) r;
	if (a->samples_head == (a->samples_tail - 1 + SAMPLE_BUF_SIZE) % SAMPLE_BUF_SIZE)
	{
		//frontend_err("samples_put: full\n");
		return cc_false;
	}
	a->samples_buf[a->samples_head] = tmp;
	a->samples_head = (a->samples_head + 1) % SAMPLE_BUF_SIZE;
	return cc_true;
}

cc_bool samples_get(audio * a, uint32_t * out)
{
	if (a->samples_head == a->samples_tail)
	{
		//frontend_err("samples_get: empty\n");
		return cc_false;
	}
	*out = a->samples_buf[a->samples_tail];
	a->samples_tail = (a->samples_tail + 1) % SAMPLE_BUF_SIZE;
	return cc_true;
}
