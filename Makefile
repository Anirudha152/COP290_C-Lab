CC = gcc
CFLAGS = -c -o2
LDFLAGS1 = -lm
LDFLAGS2 = -lm -lncurses
TARGET_CLI = cli
TARGET_GUI = gui
TARGET_TES = testbench
TARGET_TCS = test
SOURCES_CLI = command_interface/user_interface.c \
              parsing/command_processing.c \
              backend/compute_unit.c \
              parsing/cell_indexing.c \
              constants.c \
              backend/primary_storage.c \
              data_structures/stack.c \
              data_structures/set.c \
              main_cli.c

SOURCES_GUI = graphical_interface/user_interface.c \
              graphical_interface/draw.c \
              parsing/command_processing.c \
              backend/compute_unit.c \
              parsing/cell_indexing.c \
              constants.c \
              backend/primary_storage.c \
              data_structures/stack.c \
              data_structures/set.c \
              main_gui.c

SOURCES_TES = parsing/command_processing.c \
              backend/compute_unit.c \
              parsing/cell_indexing.c \
              constants.c \
              backend/primary_storage.c \
              data_structures/stack.c \
              data_structures/set.c \
              main_test.c

OBJECTS_CLI = $(SOURCES_CLI:.c=.o)
OBJECTS_GUI = $(SOURCES_GUI:.c=.o)
OBJECTS_TES = $(SOURCES_TES:.c=.o)

$(TARGET_CLI): $(OBJECTS_CLI)
	$(CC) $(OBJECTS_CLI) -o sheet $(LDFLAGS1)
	find . -name "*.o" -delete

$(TARGET_GUI): $(OBJECTS_GUI)
	$(CC) $(OBJECTS_GUI) -o sheet $(LDFLAGS2)
	find . -name "*.o" -delete

$(TARGET_TES): $(OBJECTS_TES)
	$(CC) $(OBJECTS_TES) -o sheet $(LDFLAGS1)
	find . -name "*.o" -delete

$(TARGET_TCS) : $(TARGET_TES)
	$(CC) tester.c -o tester $(LDFLAGS1)
	@for file in testcases/*; do \
		filename=$$(basename $$file); \
		./sheet 1000 1000 < testcases/$$filename > outputs/$$filename; \
		./tester outputs/$$filename expected_outputs/text_files/$$filename; \
	done
	rm -f tester

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	find . -name "*.o" -delete
	rm -f $(TARGET)