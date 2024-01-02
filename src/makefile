CC = gcc
LD = ld
CFLAGS = -noixemul -nostdlib -O2 -fomit-frame-pointer -m68000 -mregparm=2 -D__NOLIBBASE__ -DUSE_INLINE_STDARG
LDFLAGS = -noixemul -nostartfiles -nostdlib
LIBS =
OBJS = start.o main.o morsegen.o backend-stdout.o
SRCS = start.c main.c morsegen.c morsegen.h backend-stdout.c
EXE = MorsConv

all: $(OBJS)
	@echo "Linking $(EXE)..."
	@$(LD) $(LDFLAGS) -o $(EXE).db $^ $(LIBS)
	@strip $(EXE).db -o $(EXE) --strip-unneeded
	@Protect $(EXE) +E

clean:
	rm -vf *.o $(EXE) $(EXE).db

devdoc:
	Join $(SRCS) TO T:$(EXE).sources
	RoboDoc T:$(EXE).sources $(EXE)-Dev.guide GUIDE TOC SORT INTERNAL
	Delete T:$(EXE).sources

start.o: start.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -fwritable-strings -c -o $@ $<

%.o: %.c
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

# dependencies

backend-stdout.o: backend-stdout.c main.h morsegen.h
main.o: main.c main.h backend-stdout.h morsegen.h
morsegen.o: morsegen.c main.h morsegen.h
start.o: start.c
