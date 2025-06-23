#ifndef EMULATOR_H
#define EMULATOR_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common/core/clownmdemu.h"
#include "common/cd-reader.h"

#include "audio.h"

#define MAX_FILE_SIZE 8388608

typedef struct emulator
{
	void * user_data;
	uint8_t * rom_buffer;
	int rom_size;
	uint32_t colors[VDP_TOTAL_COLOURS];
	uint32_t display[VDP_MAX_SCANLINE_WIDTH * VDP_MAX_SCANLINES];
	unsigned int width;
	unsigned int height;
	audio audio_output;
	cc_bool buttons[2][CLOWNMDEMU_BUTTON_MAX];
	ClownMDEmu_Configuration configuration;
	ClownMDEmu_Constant constant;
	ClownMDEmu_State state;
	ClownMDEmu_Callbacks callbacks;
	ClownMDEmu clownmdemu;
	cc_bool has_cartridge;
	cc_bool pal;
	cc_bool overseas;
	cc_bool log_enabled;
	CDReader_State cd_reader;
}
emulator;

cc_u8f emulator_callback_cartridge_read(void * const data, const cc_u32f addr);
void emulator_callback_cartridge_write(void * const data, const cc_u32f addr, const cc_u8f val);
void emulator_callback_color_update(void * const data, const cc_u16f idx, const cc_u16f color);
void emulator_callback_scanline_render(void * const data, const cc_u16f scanline, const cc_u8l * const pixels, const cc_u16f left_boundary, const cc_u16f right_boundary, const cc_u16f width, const cc_u16f height);
cc_bool emulator_callback_input_request(void * const data, const cc_u8f player, const ClownMDEmu_Button button);
void emulator_callback_fm_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t frames, void (generate_fm_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_frames)));
void emulator_callback_psg_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t samples, void (generate_psg_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_samples)));
void emulator_callback_pcm_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t frames, void (generate_pcm_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_frames)));
void emulator_callback_cdda_generate(void * const data, const struct ClownMDEmu * clownmdemu, size_t frames, void (generate_cdda_audio(const struct ClownMDEmu * clownmdemu, cc_s16l * sample_buffer, size_t total_frames)));
void emulator_callback_cd_seek(void * const data, const cc_u32f idx);
void emulator_callback_cd_sector_read(void * const data, cc_u16l * const buf);
cc_bool emulator_callback_cd_seek_track(void * const data, const cc_u16f idx, const ClownMDEmu_CDDAMode mode);
size_t emulator_callback_cd_audio_read(void * const data, cc_s16l * const buf, const size_t frames);
cc_bool emulator_callback_save_file_open_read(void * const data, const char * const filename);
cc_s16f emulator_callback_save_file_read(void * const data);
cc_bool emulator_callback_save_file_open_write(void * const data, const char * const filename);
void emulator_callback_save_file_write(void * const data, const cc_u8f byte);
void emulator_callback_save_file_close(void * const data);
cc_bool emulator_callback_save_file_remove(void * const data, const char * const filename);
cc_bool emulator_callback_save_file_size_obtain(void * const data, const char * const filename, size_t * const size);
void emulator_callback_log(void * const data, const char * fmt, va_list args);
void emulator_callback_log_cd(void * const data, const char * const msg);
void emulator_warn(const char * fmt, ...);

void emulator_initialize(emulator * emu, void * data);
void emulator_cartridge_insert(emulator * emu, uint8_t * rom, int size);
void emulator_soft_reset(emulator * emu);
void emulator_hard_reset(emulator * emu);
void emulator_update(emulator * emu);
void emulator_shutdown(emulator * emu);
#endif
