CC = gcc
CFLAGS = -c
LDFLAGS = -lncurses -lm
TARGET = sheet
SOURCES = user_interface.c \
          draw.c \
          command_processing.c \
          compute_unit.c \
          cell_indexing.c \
          constants.c \
          primary_storage.c \
          main.c

OBJECTS = $(SOURCES:.c=.o)
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	rm *.o

%.o: %.c
	$(CC) $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)