CC = gcc
CFLAGS = -c
LDFLAGS = -lncurses -lm
TARGET = sheet
SOURCES = gui/user_interface.c \
          gui/draw.c \
          parsing/command_processing.c \
          backend/compute_unit.c \
          parsing/cell_indexing.c \
          constants.c \
          backend/primary_storage.c \
          backend/stack.c \
          main.c

# Create object files in the same directories as source files
OBJECTS = $(SOURCES:.c=.o)

# Compile and then remove object files
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	find . -name "*.o" -delete

# Pattern rule with directory preservation
%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	find . -name "*.o" -delete
	rm -f $(TARGET)