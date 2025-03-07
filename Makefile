OBJDIR = obj
OPT := -g3 -Og
CFLAGS = -std=c99 $(OPT)

EMU_CD_OBJS = cd-reader.o \
	$(addprefix clowncd/, audio.o \
	clowncd.o \
	cue.o \
	error.o \
	file-io.o \
	utilities.o \
	$(addprefix audio/, flac.o \
	mp3.o \
	vorbis.o \
	wav.o))

EMU_SND_OBJS = $(addprefix clownresampler/, clownresampler.o)

EMU_CORE_OBJS = $(addprefix core/, bus-common.o \
	log.o \
	bus-z80.o \
	controller.o \
	fm-phase.o \
	pcm.o \
	z80.o \
	clownmdemu.o \
	vdp.o \
	bus-main-m68k.o \
	fm-operator.o \
	psg.o \
	fm.o \
	fm-channel.o \
	fm-lfo.o \
	fm-operator.o \
	fm-phase.o \
	bus-sub-m68k.o \
	io-port.o \
	cdc.o \
	cdda.o \
	low-pass-filter.o \
	$(addprefix clown68000/interpreter/, clown68000.o) \
	$(addprefix clown68000/common/, opcode.o))

FRONTEND_OBJS = frontend.o \
	frontend_log.o \
	frontend_view.o \
	frontend_bridge.o \
	emulator.o \
	audio.o \
	main.o

ALL_OBJS = $(addprefix clownmdemu-frontend-common/, $(EMU_CD_OBJS) $(EMU_SND_OBJS) $(EMU_CORE_OBJS)) $(FRONTEND_OBJS)

all: clownmdemu

clownmdemu: $(ALL_OBJS:%.o=$(OBJDIR)/%.o)
	$(CC) $(CFLAGS) -framework Cocoa -framework OpenGL -framework AudioToolbox $^ -o $@

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.m | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)/clownmdemu-frontend-common/{,clowncd/{,audio/},clownresampler/,core/{,clown68000/{common,interpreter}/}}

clean:
	$(RM) -r $(OBJDIR) clownmdemu
