# compiler and default flags
CC = clang
CFLAGS = -std=c23 -g3 -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-function

# debug flags
CFLAGS += -fsanitize-trap=undefined

# release flags
#CFLAGS += -O3 -march=native -flto=auto

# sources, objects, and programs
SRC = $(shell find . -type f -name '*.c')
BIN = $(patsubst %.c, %, $(SRC))

# make functions
.PHONY: all clean check
all: $(BIN)

clean:
	@rm -rf $(BIN)

check:
	@cppcheck --quiet --project=compile_commands.json --inline-suppr \
		--enable=all --inconclusive --check-level=exhaustive \
		--suppress=checkersReport --suppress=missingIncludeSystem

# build rules
.SUFFIXES:
%: %.c Makefile
	-@$(CC) $(CFLAGS) $< -o $@
