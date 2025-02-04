CC = gcc
CFLAGS = -c
LDFLAGS1 = -lm
LDFLAGS2 = -lm -lncurses
TARGET_CLI = sheet
TARGET_GUI = gui_int
TARGET_AUT = test
SOURCES_CLI = cli/user_interface.c \
              parsing/command_processing.c \
              backend/compute_unit.c \
              parsing/cell_indexing.c \
              constants.c \
              backend/primary_storage.c \
              data_structures/stack.c \
              data_structures/set.c \
              main_cli.c

SOURCES_GUI = gui/user_interface.c \
              gui/draw.c \
              parsing/command_processing.c \
              backend/compute_unit.c \
              parsing/cell_indexing.c \
              constants.c \
              backend/primary_storage.c \
              data_structures/stack.c \
              data_structures/set.c \
              main_gui.c

SOURCES_AUT = parsing/command_processing.c \
              backend/compute_unit.c \
              parsing/cell_indexing.c \
              constants.c \
              backend/primary_storage.c \
              data_structures/stack.c \
              data_structures/set.c \
              main_auto.c


OBJECTS_CLI = $(SOURCES_CLI:.c=.o)
OBJECTS_GUI = $(SOURCES_GUI:.c=.o)
OBJECTS_AUT = $(SOURCES_AUT:.c=.o)

$(TARGET_CLI): $(OBJECTS_CLI)
	$(CC) $(OBJECTS_CLI) -o $(TARGET_CLI) $(LDFLAGS1)
	find . -name "*.o" -delete

$(TARGET_GUI): $(OBJECTS_GUI)
	$(CC) $(OBJECTS_GUI) -o $(TARGET_GUI) $(LDFLAGS2)
	find . -name "*.o" -delete

$(TARGET_AUT): $(OBJECTS_AUT)
	$(CC) $(OBJECTS_AUT) -o $(TARGET_AUT) $(LDFLAGS1)
	find . -name "*.o" -delete

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	find . -name "*.o" -delete
	rm -f $(TARGET)