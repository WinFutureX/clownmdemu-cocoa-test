#include "emulator.h"
#include "frontend_log.h"
#include "frontend_bridge.h"

cc_u8f emulator_callback_cartridge_read(void * const data, const cc_u32f addr)
{
	emulator * e = (emulator *) data;
	return addr < e->rom_size ? e->rom_buffer[addr] : 0;
}

void emulator_callback_cartridge_write(void * const data, const cc_u32f addr, const cc_u8f val)
{
	// unimplemented
	// any attempt to write to rom will call the bridge function instead
	// so we can test the c-objc frontend bridging
	// plus the official frontends don't really use this callback anyway
	emulator * e = (emulator *) data;
	frontend_bridge_cartridge_write(e->user_data, addr, val);
}

void emulator_callback_color_update(void * const data, const cc_u16f idx, const cc_u16f color)
{
	emulator * e = (emulator *) data;
	// split from XBGR4444
	const cc_u32f r = color & 0xF;
	const cc_u32f g = color >> 4 & 0xF;
	const cc_u32f b = color >> 8 & 0xF;
	// recombine into BGRA8888
	e->colors[idx] = 0xFF | (r << 8) | (r << 12) | (g << 16) | (g << 20) | (b << 24) | (b << 28);
}

void emulator_callback_scanline_render(void * const data, const cc_u16f scanline, const cc_u8l * const pixels, const cc_u16f left_boundary, const cc_u16f right_boundary, const cc_u16f width, const cc_u16f height)
{
	emulator * e = (emulator *) data;
	e->width = width;
	e->height = height;
	const uint8_t * input = pixels + left_boundary;
	uint32_t * output = &e->display[scanline * width + left_boundary];
	for (int i = left_boundary; i < right_boundary; ++i)
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
	generate_fm_audio(clownmdemu, mixer_allocate_fm(&e->audio_output.mixer, frames), frames);
}

void emulator_callback_psg_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t samples, void (generate_psg_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_samples)))
{
	emulator * e = (emulator *) data;
	generate_psg_audio(clownmdemu, mixer_allocate_psg(&e->audio_output.mixer, samples), samples);
}

void emulator_callback_pcm_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t frames, void (generate_pcm_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_frames)))
{
	emulator * e = (emulator *) data;
	generate_pcm_audio(clownmdemu, mixer_allocate_pcm(&e->audio_output.mixer, frames), frames);
}

void emulator_callback_cdda_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t frames, void (generate_cdda_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_frames)))
{
	emulator * e = (emulator *) data;
	generate_cdda_audio(clownmdemu, mixer_allocate_cdda(&e->audio_output.mixer, frames), frames);
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
	return -1;
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

void emulator_initialize(emulator * emu, void * data)
{
	emu->user_data = data; // required to call objc frontend methods
	// emulator core
	ClownMDEmu_Parameters_Initialise(&emu->clownmdemu, &emu->configuration, &emu->constant, &emu->state, &emu->callbacks);
	emu->callbacks.user_data = emu;
	emu->callbacks.cartridge_read = emulator_callback_cartridge_read;
	emu->callbacks.cartridge_written = emulator_callback_cartridge_write;
	emu->callbacks.colour_updated = emulator_callback_color_update;
	emu->callbacks.scanline_rendered = emulator_callback_scanline_render;
	emu->callbacks.input_requested = emulator_callback_input_request;
	emu->callbacks.fm_audio_to_be_generated = emulator_callback_fm_generate;
	emu->callbacks.psg_audio_to_be_generated = emulator_callback_psg_generate;
	emu->callbacks.pcm_audio_to_be_generated = emulator_callback_pcm_generate;
	emu->callbacks.cdda_audio_to_be_generated = emulator_callback_cdda_generate;
	emu->callbacks.cd_seeked = emulator_callback_cd_seek;
	emu->callbacks.cd_sector_read = emulator_callback_cd_sector_read;
	emu->callbacks.cd_track_seeked = emulator_callback_cd_seek_track;
	emu->callbacks.cd_audio_read = emulator_callback_cd_audio_read;
	emu->callbacks.save_file_opened_for_reading = emulator_callback_save_file_open_read;
	emu->callbacks.save_file_read = emulator_callback_save_file_read;
	emu->callbacks.save_file_opened_for_writing = emulator_callback_save_file_open_write;
	emu->callbacks.save_file_written = emulator_callback_save_file_write;
	emu->callbacks.save_file_closed = emulator_callback_save_file_close;
	emu->callbacks.save_file_removed = emulator_callback_save_file_remove;
	emu->callbacks.save_file_size_obtained = emulator_callback_save_file_size_obtain;
	emu->configuration.general.region = CLOWNMDEMU_REGION_OVERSEAS; // to be set by cartridge loader
	emu->configuration.general.tv_standard = CLOWNMDEMU_TV_STANDARD_NTSC; // to be set by cartridge loader
	emu->configuration.vdp.sprites_disabled       = cc_false;
	emu->configuration.vdp.window_disabled        = cc_false;
	emu->configuration.vdp.planes_disabled[0]     = cc_false;
	emu->configuration.vdp.planes_disabled[1]     = cc_false;
	emu->configuration.fm.fm_channels_disabled[0] = cc_false;
	emu->configuration.fm.fm_channels_disabled[1] = cc_false;
	emu->configuration.fm.fm_channels_disabled[2] = cc_false;
	emu->configuration.fm.fm_channels_disabled[3] = cc_false;
	emu->configuration.fm.fm_channels_disabled[4] = cc_false;
	emu->configuration.fm.fm_channels_disabled[5] = cc_false;
	emu->configuration.fm.dac_channel_disabled    = cc_false;
	emu->configuration.psg.tone_disabled[0]       = cc_false;
	emu->configuration.psg.tone_disabled[1]       = cc_false;
	emu->configuration.psg.tone_disabled[2]       = cc_false;
	emu->configuration.psg.noise_disabled         = cc_false;
	emu->configuration.pcm.channels_disabled[0]   = cc_false;
	emu->configuration.pcm.channels_disabled[1]   = cc_false;
	emu->configuration.pcm.channels_disabled[2]   = cc_false;
	emu->configuration.pcm.channels_disabled[3]   = cc_false;
	emu->configuration.pcm.channels_disabled[4]   = cc_false;
	emu->configuration.pcm.channels_disabled[5]   = cc_false;
	emu->configuration.pcm.channels_disabled[6]   = cc_false;
	emu->configuration.pcm.channels_disabled[7]   = cc_false;
	ClownMDEmu_SetLogCallback(emulator_callback_log, emu);
	ClownMDEmu_Constant_Initialise(&emu->constant);
	ClownMDEmu_State_Initialise(&emu->state);
	// audio engine
	audio_initialize(&emu->audio_output);
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
	audio_mixer_initialize(&emu->audio_output, emu->pal);
}

void emulator_soft_reset(emulator * emu)
{
	ClownMDEmu_Reset(&emu->clownmdemu, cc_false, emu->rom_size);
}

void emulator_hard_reset(emulator * emu)
{
	ClownMDEmu_State_Initialise(&emu->state);
	ClownMDEmu_Parameters_Initialise(&emu->clownmdemu, &emu->configuration, &emu->constant, &emu->state, &emu->callbacks);
	emulator_soft_reset(emu);
}

void emulator_update(emulator * emu)
{
	audio_mixer_begin(&emu->audio_output);
	ClownMDEmu_Iterate(&emu->clownmdemu);
	audio_mixer_end(&emu->audio_output);
}

void emulator_shutdown(emulator * emu)
{
	audio_shutdown(&emu->audio_output);
}
