CC = g++
LD = g++
CFLAGS = -nostdlib -O2 -fbaserel -fomit-frame-pointer -mregparm -fno-exceptions -fno-rtti -D__NOLIBBASE__
LDFLAGS = -nostdlib -fbaserel -fomit-frame-pointer -nostartfiles
LIBS = -lminteger -lmfloat -lmtransfloat
OBJS = start.o main.o morsegen.o morsegenaudio.o backend-count.o backend-stdout.o backend-8svx.o \
 backend-wave.o backend-audio.o purevirtual.o
EXE = MorsConv

all: $(OBJS)
	@echo "Linking $(EXE)..."
	@$(LD) $(LDFLAGS) -o $(EXE).db $^ $(LIBS)
	@strip $(EXE).db -o $(EXE) --strip-unneeded
	@Protect $(EXE) +E
	List $(EXE)

dep:
	$(CC) -MM $(OBJS)

clean:
	rm -vf *.o $(EXE) $(EXE).db

start.o: start.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -fwritable-strings -c -o $@ $<

purevirtual.o: purevirtual.c
	@echo "Compiling $@..."
	@gcc $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

# dependencies

backend-8svx.o: backend-8svx.cpp main.h backend-8svx.h morsegenaudio.h morsegen.h \
 backend-count.h
backend-audio.o: backend-audio.cpp main.h backend-audio.h morsegenaudio.h morsegen.h
backend-count.o: backend-count.cpp main.h backend-count.h morsegen.h
backend-stdout.o: backend-stdout.cpp backend-stdout.h main.h morsegen.h
backend-wave.o: backend-wave.cpp main.h backend-wave.h morsegenaudio.h morsegen.h
main.o: main.cpp main.h backend-stdout.h morsegen.h backend-count.h backend-8svx.h \
 morsegenaudio.h backend-wave.h backend-audio.h
morsegen.o: morsegen.cpp morsegen.h main.h
morsegenaudio.o: morsegenaudio.cpp main.h morsegenaudio.h morsegen.h
start.o: start.cpp
