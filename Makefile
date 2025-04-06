# Directorys.
SRCDIR := ./src/c
INCDIR := ./src/include
BUILD_DIR := ./build
OBJDIR := $(BUILD_DIR)/obj

# Compiler and flags.
CC := clang
CFLAGS := -Wall -Wextra -O2 -flto=auto -fno-fat-lto-objects -std=gnu99\
 -Wextra -pedantic -Wno-unused-parameter -Wstrict-prototypes -Wshadow -Wconversion -Wvla -Wdouble-promotion -Wmissing-noreturn -Wmissing-format-attribute\
 -Wmissing-prototypes -fsigned-char -fstack-protector-strong -Wno-conversion -fno-common -Wno-unused-result -Wimplicit-fallthrough -fdiagnostics-color=always\
 -march=native -Rpass=loop-vectorize -mavx -Wno-vla

# Get all c source files.
SOURCES := $(wildcard $(SRCDIR)/*.c)

# Generate object files.
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# The output liberary.
LIBRARY := $(BUILD_DIR)/libfcio.a

# Rule to build the library.
$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $^

# Rule to compile source files into object files.
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Include dependency files if they exist.
-include $(OBJECTS:.o=.d)

# Create the object dir if it does not exist.
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Clean up the object files (removes the entire build dir).
clean:
	rm -rf $(BUILD_DIR)

# Install the library globaly
install:
	mkdir -p /usr/include/fcio
	cp $(INCDIR)/*.h /usr/include/fcio
	cp $(BUILD_DIR)/libfcio.a /usr/lib/libfcio.a

tests:
	$(MAKE) -C test tests

clean-tests:
	$(MAKE) -C test clean

# Phony targets.
.PHONY: clean install tests clean-tests
