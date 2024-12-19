# compiler and default flags
CC = clang
CFLAGS = -std=c23 -g -Wall -Wextra -Wpedantic -Wshadow

# debug flags
CFLAGS += -Og -fno-omit-frame-pointer -fsanitize=undefined
CFLAGS += -fsanitize=address
#CFLAGS += -fsanitize=memory -fPIE -pie

# release flags
#CFLAGS += -march=native -Ofast -flto=auto -DNDEBUG

# profiling flags
#CFLAGS += -pg

# sources, objects, and programs
SRC = $(shell find . -type f -name '*.c')
BIN = $(patsubst %.c, %, $(SRC))

# make functions
.PHONY: all clean check tidy format
all: $(BIN)

clean:
	@rm -rf $(BIN)

check:
	@cppcheck --quiet --project=compile_commands.json \
		--enable=all --inconclusive --check-level=exhaustive \
		--suppress=missingIncludeSystem --suppress=checkersReport

tidy:
	@clang-tidy --quiet $(shell find . -type f -name '*.[ch]')

format:
	@clang-format -i $(shell find . -type f -name '*.[ch]')

# build rules
.SUFFIXES:
%: %.c Makefile
	-@$(CC) $(CFLAGS) $< -o $@
