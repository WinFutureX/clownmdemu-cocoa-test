#include "emulator.h"
#include "frontend_log.h"

cc_u8f emulator_callback_cartridge_read(void * const data, const cc_u32f addr)
{
	emulator * e = (emulator *) data;
	return addr <= e->rom_size ? e->rom_buffer[addr] : 0;
}

void emulator_callback_cartridge_write(void * const data, const cc_u32f addr, const cc_u8f val)
{
	// unimplemented
}

void emulator_callback_color_update(void * const data, const cc_u16f idx, const cc_u16f color)
{
	emulator * e = (emulator *) data;
	// split from XBGR4444
	const cc_u32f r = color & 0xF;
	const cc_u32f g = color >> 4 & 0xF;
	const cc_u32f b = color >> 8 & 0xF;
	// recompose into BGRA8888
	e->colors[idx] = 0xFF | (r << 8) | (r << 12) | (g << 16) | (g << 20) | (b << 24) | (b << 28);
}

void emulator_callback_scanline_render(void * const data, const cc_u16f scanline, const cc_u8l * const pixels, const cc_u16f width, const cc_u16f height)
{
	emulator * e = (emulator *) data;
	e->width = width;
	e->height = height;
	const uint8_t * input = pixels;
	uint32_t * output = &e->display[scanline * width];
	for (int i = 0; i < width; ++i)
	{
		*output++ = e->colors[*input++];
	}
}

cc_bool emulator_callback_input_request(void * const data, const cc_u8f player, const ClownMDEmu_Button button)
{
	emulator * e = (emulator *) data;
	return e->buttons[player][button];
}

void emulator_callback_fm_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t frames, void (generate_fm_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_frames)))
{
	emulator * e = (emulator *) data;
	generate_fm_audio(clownmdemu, mixer_allocate_fm(&e->audio_output->mixer, frames), frames);
}

void emulator_callback_psg_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t samples, void (generate_psg_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_samples)))
{
	emulator * e = (emulator *) data;
	generate_psg_audio(clownmdemu, mixer_allocate_psg(&e->audio_output->mixer, samples), samples);
}

void emulator_callback_pcm_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t frames, void (generate_pcm_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_frames)))
{
	emulator * e = (emulator *) data;
	generate_pcm_audio(clownmdemu, mixer_allocate_pcm(&e->audio_output->mixer, frames), frames);
}

void emulator_callback_cdda_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t frames, void (generate_cdda_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_frames)))
{
	emulator * e = (emulator *) data;
	generate_cdda_audio(clownmdemu, mixer_allocate_cdda(&e->audio_output->mixer, frames), frames);
}

void emulator_callback_cd_seek(void * const data, const cc_u32f idx)
{
	// unimplemented
}

void emulator_callback_cd_sector_read(void * const data, cc_u16l * buf)
{
	// unimplemented
}

cc_bool emulator_callback_cd_seek_track(void * const data, const cc_u16f idx, const ClownMDEmu_CDDAMode mode)
{
	// unimplemented
	return cc_false;
}

size_t emulator_callback_cd_audio_read(void * const data, cc_s16l * const buf, const size_t frames)
{
	// unimplemented
	return 0;
}

cc_bool emulator_callback_save_file_open_read(void * const data, const char * const filename)
{
	// unimplemented
	return cc_false;
}

cc_s16f emulator_callback_save_file_read(void * const data)
{
	// unimplemented
	return 0;
}

cc_bool emulator_callback_save_file_open_write(void * const data, const char * const filename)
{
	// unimplemented
	return cc_false;
}

void emulator_callback_save_file_write(void * const data, const cc_u8f byte)
{
	// unimplemented
}

void emulator_callback_save_file_close(void * const data)
{
	// unimplemented
}

cc_bool emulator_callback_save_file_remove(void * const data, const char * const filename)
{
	// unimplemented
	return cc_false;
}

cc_bool emulator_callback_save_file_size_obtain(void * const data, const char * const filename, size_t * const size)
{
	// unimplemented
	return cc_false;
}

void emulator_callback_log(void * const data, const char * fmt, va_list args)
{
	emulator * e = (emulator *) data;
	if (e->log_enabled == cc_true)
	{
		char buf[256];
		vsprintf(buf, fmt, args);
		printf("[emulator] %s\n", buf);
	}
}

emulator * emulator_alloc()
{
	emulator * ret = calloc(1, sizeof(emulator));
	if (ret)
	{
		// emulator core
		ClownMDEmu_Parameters_Initialise(&ret->clownmdemu, &ret->configuration, &ret->constant, &ret->state, &ret->callbacks);
		ret->callbacks.user_data = ret;
		ret->callbacks.cartridge_read = emulator_callback_cartridge_read;
		ret->callbacks.cartridge_written = emulator_callback_cartridge_write;
		ret->callbacks.colour_updated = emulator_callback_color_update;
		ret->callbacks.scanline_rendered = emulator_callback_scanline_render;
		ret->callbacks.input_requested = emulator_callback_input_request;
		ret->callbacks.fm_audio_to_be_generated = emulator_callback_fm_generate;
		ret->callbacks.psg_audio_to_be_generated = emulator_callback_psg_generate;
		ret->callbacks.pcm_audio_to_be_generated = emulator_callback_pcm_generate;
		ret->callbacks.cdda_audio_to_be_generated = emulator_callback_cdda_generate;
		ret->callbacks.cd_seeked = emulator_callback_cd_seek;
		ret->callbacks.cd_sector_read = emulator_callback_cd_sector_read;
		ret->callbacks.cd_track_seeked = emulator_callback_cd_seek_track;
		ret->callbacks.cd_audio_read = emulator_callback_cd_audio_read;
		ret->callbacks.save_file_opened_for_reading = emulator_callback_save_file_open_read;
		ret->callbacks.save_file_read = emulator_callback_save_file_read;
		ret->callbacks.save_file_opened_for_writing = emulator_callback_save_file_open_write;
		ret->callbacks.save_file_written = emulator_callback_save_file_write;
		ret->callbacks.save_file_closed = emulator_callback_save_file_close;
		ret->callbacks.save_file_removed = emulator_callback_save_file_remove;
		ret->callbacks.save_file_size_obtained = emulator_callback_save_file_size_obtain;
		ret->configuration.general.region = CLOWNMDEMU_REGION_OVERSEAS; // to be set by cartridge loader
		ret->configuration.general.tv_standard = CLOWNMDEMU_TV_STANDARD_NTSC; // to be set by cartridge loader
		ret->configuration.vdp.sprites_disabled       = cc_false;
		ret->configuration.vdp.window_disabled        = cc_false;
		ret->configuration.vdp.planes_disabled[0]     = cc_false;
		ret->configuration.vdp.planes_disabled[1]     = cc_false;
		ret->configuration.fm.fm_channels_disabled[0] = cc_false;
		ret->configuration.fm.fm_channels_disabled[1] = cc_false;
		ret->configuration.fm.fm_channels_disabled[2] = cc_false;
		ret->configuration.fm.fm_channels_disabled[3] = cc_false;
		ret->configuration.fm.fm_channels_disabled[4] = cc_false;
		ret->configuration.fm.fm_channels_disabled[5] = cc_false;
		ret->configuration.fm.dac_channel_disabled    = cc_false;
		ret->configuration.psg.tone_disabled[0]       = cc_false;
		ret->configuration.psg.tone_disabled[1]       = cc_false;
		ret->configuration.psg.tone_disabled[2]       = cc_false;
		ret->configuration.psg.noise_disabled         = cc_false;
		ret->configuration.pcm.channels_disabled[0]   = cc_false;
		ret->configuration.pcm.channels_disabled[1]   = cc_false;
		ret->configuration.pcm.channels_disabled[2]   = cc_false;
		ret->configuration.pcm.channels_disabled[3]   = cc_false;
		ret->configuration.pcm.channels_disabled[4]   = cc_false;
		ret->configuration.pcm.channels_disabled[5]   = cc_false;
		ret->configuration.pcm.channels_disabled[6]   = cc_false;
		ret->configuration.pcm.channels_disabled[7]   = cc_false;
		ClownMDEmu_SetLogCallback(emulator_callback_log, ret);
		ClownMDEmu_Constant_Initialise(&ret->constant);
		ClownMDEmu_State_Initialise(&ret->state);
		// audio engine
		ret->audio_output = audio_alloc();
		if (!ret->audio_output)
		{
			frontend_log("audio_alloc() failed\n");
			free(ret);
			ret = NULL;
		}
	}
	return ret;
}

void emulator_cartridge_insert(emulator * emu, uint8_t * rom, int size)
{
	emu->rom_buffer = rom;
	emu->rom_size = size;
	emu->has_cartridge = cc_true;
	emu->pal = emu->rom_buffer[0x1F0] == 'E' ? cc_true : cc_false;
	emu->overseas = emu->rom_buffer[0x1F0] == 'J' ? cc_false : cc_true;
	emu->configuration.general.region = emu->overseas == cc_true ? CLOWNMDEMU_REGION_OVERSEAS : CLOWNMDEMU_REGION_DOMESTIC;
	emu->configuration.general.tv_standard = emu->pal == cc_true ? CLOWNMDEMU_TV_STANDARD_PAL : CLOWNMDEMU_TV_STANDARD_NTSC;
	emulator_hard_reset(emu);
	audio_initialize(emu->audio_output, emu->pal);
}

void emulator_soft_reset(emulator * emu)
{
	ClownMDEmu_Reset(&emu->clownmdemu, cc_false);
}

void emulator_hard_reset(emulator * emu)
{
	ClownMDEmu_State_Initialise(&emu->state);
	ClownMDEmu_Parameters_Initialise(&emu->clownmdemu, &emu->configuration, &emu->constant, &emu->state, &emu->callbacks);
	emulator_soft_reset(emu);
}

void emulator_update(emulator * emu)
{
	audio_begin(emu->audio_output);
	ClownMDEmu_Iterate(&emu->clownmdemu);
	audio_end(emu->audio_output);
}

void emulator_shutdown(emulator * emu)
{
	audio_shutdown(emu->audio_output);
	free(emu);
}
