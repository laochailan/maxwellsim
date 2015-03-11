CC = clang
TARGET = maxwellsim
CFLAGS= -g -O3 -Isrc -Wall `sdl2-config --cflags` `libpng-config --cflags`
LFLAGS = -lm `sdl2-config --libs` `libpng-config --libs`

SRCS = \
	  src/model.c \
	  src/gfx.c \
	  src/main.c \
	  src/vec.c \
	  src/loadmap.c

OBJECTS = $(notdir $(SRCS:.c=.o))

all:	$(TARGET)

-include depends

%.o: src/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(CFLAGS) $(LFLAGS)


clean:
	rm $(OBJECTS)
	rm $(TARGET)

depend:
	$(CC) $(CFLAGS) $(SRCS) -MM >depends

.PHONY: all clean depend
