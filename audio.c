#include <stdint.h>
#define MIXER_IMPLEMENTATION
#include "audio.h"
#include "frontend_log.h"

void audio_queue_callback(void * data, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
	audio * a = (audio *) data;
	if (a->shutdown == cc_true) return;
	MIXER_FORMAT * source = &a->samples[0];
	uint8_t * target = buffer->mAudioData;
	if (a->paused == cc_true || a->done == cc_true)
	{
		memset(target, 0, buffer->mAudioDataBytesCapacity);
		buffer->mAudioDataByteSize = buffer->mAudioDataBytesCapacity;
	}
	else
	{
		memcpy(target, source, a->bytes);
		buffer->mAudioDataByteSize = a->bytes;
		a->done = cc_true;
	}
	OSStatus err = AudioQueueEnqueueBuffer(queue, buffer, 0, 0);
	if (err) frontend_err("audio_queue_callback: AudioQueueEnqueueBuffer returned %d\n", err);
}

void mixer_callback(void * data, const MIXER_FORMAT * samples, size_t frames)
{
	// in pal mode, this is called several times with frames set to 0 for some reason???
	// we'll ignore such cases so the audio queue callback won't be fed an empty buffer
	// thus we'll get working audio output under pal mode just like ntsc
	if (frames > 0)
	{
		audio * a = (audio *) data;
		MIXER_FORMAT * output = &a->samples[0];
		for (int i = 0; i < frames; i++)
		{
			*output++ = *samples++;
			*output++ = *samples++;
		}
		a->bytes = sizeof(MIXER_FORMAT) * frames * 2;
		a->done = cc_false;
	}
}

MIXER_FORMAT * mixer_allocate_fm(Mixer * mixer, size_t frames)
{
	return Mixer_AllocateFMSamples(mixer, frames);
}

MIXER_FORMAT * mixer_allocate_psg(Mixer * mixer, size_t frames)
{
	return Mixer_AllocatePSGSamples(mixer, frames);
}

MIXER_FORMAT * mixer_allocate_pcm(Mixer * mixer, size_t frames)
{
	return Mixer_AllocatePCMSamples(mixer, frames);
}

MIXER_FORMAT * mixer_allocate_cdda(Mixer * mixer, size_t frames)
{
	return Mixer_AllocateCDDASamples(mixer, frames);
}

#define NUM_AUDIO_QUEUE_BUFFERS 4

void audio_initialize(audio * a)
{
	// required to initialize mixer properly
	a->mixer = (Mixer) {&a->constant, &a->state};
	Mixer_Constant_Initialise(&a->constant);
	a->has_mixer = cc_false;
	a->has_queue = cc_false;
}

cc_bool audio_queue_initialize(audio * a, cc_bool pal)
{
	cc_bool ret = cc_false;
	AudioStreamBasicDescription stream_desc;
	stream_desc.mSampleRate = 48000.0f;
	stream_desc.mFormatID = kAudioFormatLinearPCM;
	stream_desc.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
	stream_desc.mBitsPerChannel = 16;
	stream_desc.mChannelsPerFrame = 2;
	stream_desc.mFramesPerPacket = 1;
	stream_desc.mBytesPerFrame = 4;
	stream_desc.mBytesPerPacket = 4;
	a->queue = 0;
	OSStatus err = AudioQueueNewOutput(&stream_desc, &audio_queue_callback, a, 0, 0, 0, &a->queue);
	if (err) frontend_err("audio_queue_initialize: AudioQueueNewOutput failed: %d\n", err);
	int buffer_size = stream_desc.mBytesPerFrame * (pal == cc_true ? 960 : 800);
	AudioQueueBufferRef audio_queue_buffers[NUM_AUDIO_QUEUE_BUFFERS];
	for (int i = 0; i < NUM_AUDIO_QUEUE_BUFFERS; i++)
	{
		err = AudioQueueAllocateBuffer(a->queue, buffer_size, &audio_queue_buffers[i]);
		if (err) frontend_err("audio_queue_initialize: AudioQueueAllocateBuffer failed for buffer %d: %d\n", i, err);
		memset(audio_queue_buffers[i]->mAudioData, 0, audio_queue_buffers[i]->mAudioDataBytesCapacity);
		audio_queue_buffers[i]->mAudioDataByteSize = audio_queue_buffers[i]->mAudioDataBytesCapacity;
		err = AudioQueueEnqueueBuffer(a->queue, audio_queue_buffers[i], 0, NULL);
		if (err) frontend_err("audio_queue_initialize: AudioQueueEnqueueBuffer failed for buffer %d: %d\n", i, err);
	}
	err = AudioQueueSetParameter(a->queue, kAudioQueueParam_Volume, 1.0);
	if (err) frontend_err("audio_queue_initialize: AudioQueueSetParameter failed for parameter kAudioQueueParam_Volume: %d\n", err);
	err = AudioQueueStart(a->queue, 0);
	if (err) frontend_err("audio_queue_initialize: AudioQueueStart failed: %d\n", err);
	if (!err) ret = cc_true;
	return ret;
}

void audio_mixer_initialize(audio * a, cc_bool pal)
{
	if (a->has_queue == cc_true) audio_queue_shutdown(a);
	if (a->has_mixer == cc_true) Mixer_State_Deinitialise(&a->state);
	a->pal = pal;
	a->has_mixer = Mixer_State_Initialise(&a->state, 48000, a->pal) == cc_true ? cc_true : cc_false;
	a->has_queue = audio_queue_initialize(a, pal);
	a->shutdown = cc_false;
}

void audio_mixer_begin(audio * a)
{
	if (a->has_mixer == cc_true) Mixer_Begin(&a->mixer);
}

void audio_mixer_end(audio * a)
{
	if (a->has_mixer == cc_true) Mixer_End(&a->mixer, 1, 1, mixer_callback, a);
}

void audio_queue_shutdown(audio * a)
{
	a->shutdown = cc_true;
	OSStatus err = AudioQueueStop(a->queue, TRUE);
	if (err) frontend_err("audio_queue_shutdown: AudioQueueStop failed: %d\n", err);
	err = AudioQueueDispose(a->queue, FALSE);
	if (err) frontend_err("audio_queue_shutdown: AudioQueueDispose failed: %d\n", err);
	a->has_queue = cc_false;
}

void audio_shutdown(audio * a)
{
	if (a->has_queue == cc_true) audio_queue_shutdown(a);
	Mixer_State_Deinitialise(&a->state);
	a->has_mixer = cc_false;
}
