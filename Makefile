DEBUG ?= 0
FRONTEND_NO_OPENGL ?= 0

OBJDIR = obj
ifeq ($(DEBUG), $(filter $(DEBUG), 1 y))
OPT := -g3 -Og
else
OPT := -O2
endif
CFLAGS := -std=c99 $(OPT)

ifeq ($(FRONTEND_NO_OPENGL), $(filter $(FRONTEND_NO_OPENGL), 1 y))
CFLAGS += -DFRONTEND_NO_OPENGL
endif

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
	wav.o \
	$(addprefix libraries/clownresampler/, clownresampler.o)))

EMU_CORE_OBJS = $(addprefix core/, bus-common.o \
	bus-main-m68k.o \
	bus-sub-m68k.o \
	bus-z80.o \
	cdc.o \
	cdda.o \
	clownmdemu.o \
	controller.o \
	fm.o \
	fm-channel.o \
	fm-lfo.o \
	fm-operator.o \
	fm-phase.o \
	io-port.o \
	log.o \
	low-pass-filter.o \
	pcm.o \
	psg.o \
	vdp.o \
	z80.o \
	$(addprefix clown68000/interpreter/, clown68000.o) \
	$(addprefix clown68000/common/, opcode.o))

FRONTEND_OBJS = frontend.o \
	frontend_log.o \
	frontend_view.o \
	frontend_bridge.o \
	emulator.o \
	audio.o \
	main.o

ALL_OBJS = $(addprefix common/, $(EMU_CD_OBJS) $(EMU_CORE_OBJS)) $(FRONTEND_OBJS)

all: clownmdemu

clownmdemu: $(ALL_OBJS:%.o=$(OBJDIR)/%.o)
	$(CC) $(CFLAGS) -framework Cocoa -framework OpenGL -framework AudioUnit $^ -o $@

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.m | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)/common/{,clowncd/{,audio/libraries/clownresampler},core/{,clown68000/{common,interpreter}/}}

clean:
	$(RM) -r $(OBJDIR) clownmdemu
