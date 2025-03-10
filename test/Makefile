# Directories.
BUILD_DIR := ../build
LIBRARY := $(BUILD_DIR)/libfcio.a
SRC_DIR := ./src
BIN_DIR := ./bin

# Compiler and flags.
CC := clang
CFLAGS := -Wall -Wextra -O2 -flto=auto -fno-fat-lto-objects\
 -Wextra -pedantic -Wno-unused-parameter -Wstrict-prototypes -Wshadow -Wconversion -Wvla -Wdouble-promotion -Wmissing-noreturn -Wmissing-format-attribute\
 -Wmissing-prototypes -fsigned-char -fstack-protector-strong -Wno-conversion -fno-common -Wno-unused-result -Wimplicit-fallthrough -fdiagnostics-color=always\
 -march=native -Rpass=loop-vectorize -mavx -Wno-vla

# Find all tests in ./src/.
SOURCES := $(wildcard $(SRC_DIR)/*.c)
# Convert each test source file into its own binary in ./bin/.
BINARIES := $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%)

# Rule to compile each test source file into its own binary.
$(BIN_DIR)/%: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -MMD -MP $< -o $@ $(LIBRARY)

# Rule to create the bin directory if it ever does not exist, for some reason.
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build all test bins.
tests: $(BINARIES)
	@echo "All test binaries compiled successfully."

# Clean up (remove all test bins).
clean:
	rm -f $(BIN_DIR)/*

.PHONY: tests clean
