CC = clang
TARGET = maxwellsim
CFLAGS= -g -O3 -Isrc -Wall `sdl2-config --cflags`
LFLAGS = -lm `sdl2-config --libs`

SRCS = \
	  src/wire.c \
	  src/gfx.c \
	  src/main.c

OBJECTS = $(notdir $(SRCS:.c=.o))

all:	$(TARGET)

-include depends

%.o: src/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

#wire.o: src/wire.c
#	$(CC) -o wire.o -c src/wire.c $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(CFLAGS) $(LFLAGS)


clean:
	rm $(OBJECTS)
	rm $(TARGET)

depend:
	$(CC) $(CFLAGS) $(SRCS) -MM >depends

.PHONY: all clean depend
