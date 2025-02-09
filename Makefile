CC=clang
CFLAGS=-Iclownmdemu-frontend-common -Iclownmdemu-frontend-common/clownresampler -Iclownmdemu-frontend-common/core -Iclownmdemu-frontend-common/core/clowncommon -Iclownmdemu-frontend-common/core/clown68000/common/clowncommon -g3 -Og

EMU_CD_OBJ=clownmdemu-frontend-common/cd-reader.o \
	clownmdemu-frontend-common/clowncd/audio.o \
	clownmdemu-frontend-common/clowncd/clowncd.o \
	clownmdemu-frontend-common/clowncd/cue.o \
	clownmdemu-frontend-common/clowncd/error.o \
	clownmdemu-frontend-common/clowncd/file-io.o \
	clownmdemu-frontend-common/clowncd/utilities.o \
	clownmdemu-frontend-common/clowncd/audio/flac.o \
	clownmdemu-frontend-common/clowncd/audio/mp3.o \
	clownmdemu-frontend-common/clowncd/audio/vorbis.o \
	clownmdemu-frontend-common/clowncd/audio/wav.o

EMU_SND_OBJ=clownmdemu-frontend-common/clownresampler/clownresampler.o

EMU_CORE_OBJ=clownmdemu-frontend-common/core/bus-common.o \
	clownmdemu-frontend-common/core/log.o \
	clownmdemu-frontend-common/core/bus-z80.o \
	clownmdemu-frontend-common/core/controller.o \
	clownmdemu-frontend-common/core/fm-phase.o \
	clownmdemu-frontend-common/core/clown68000/interpreter/clown68000.o \
	clownmdemu-frontend-common/core/clown68000/common/opcode.o \
	clownmdemu-frontend-common/core/pcm.o \
	clownmdemu-frontend-common/core/z80.o \
	clownmdemu-frontend-common/core/clownmdemu.o \
	clownmdemu-frontend-common/core/vdp.o \
	clownmdemu-frontend-common/core/bus-main-m68k.o \
	clownmdemu-frontend-common/core/fm-operator.o \
	clownmdemu-frontend-common/core/psg.o \
	clownmdemu-frontend-common/core/fm.o \
	clownmdemu-frontend-common/core/fm-channel.o \
	clownmdemu-frontend-common/core/fm-lfo.o \
	clownmdemu-frontend-common/core/fm-operator.o \
	clownmdemu-frontend-common/core/fm-phase.o \
	clownmdemu-frontend-common/core/bus-sub-m68k.o \
	clownmdemu-frontend-common/core/io-port.o \
	clownmdemu-frontend-common/core/cdc.o \
	clownmdemu-frontend-common/core/cdda.o

FRONTEND_OBJ =  frontend.o \
		frontend_log.o \
		frontend_view.o \
		emulator.o \
		audio.o \
		main.o

%.o: %.c %.m
	$(CC) $(CFLAGS) -c $< -o $@

clownmdemu: $(EMU_CD_OBJ) $(EMU_SND_OBJ) $(EMU_CORE_OBJ) $(FRONTEND_OBJ)
	$(CC) $(CFLAGS) -framework Cocoa -framework QuartzCore -framework OpenGL -framework AudioUnit -framework CoreAudio -framework AudioToolBox $^ -o $@

clean:
	$(RM) *.o clownmdemu-frontend-common/{,clowncd/{,audio/},clownresampler/,core/{,clown68000/{common,interpreter}/}}*.o clownmdemu
