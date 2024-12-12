# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2 -fPIC -std=c11
AR = ar
ARFLAGS = rcs

# Platform detection
ifeq ($(OS),Windows_NT)
    RM = del /Q
    MKDIR = if not exist "$(1)" mkdir "$(1)"
    INSTALL = echo "Install not supported on Windows"
    UNINSTALL = echo "Uninstall not supported on Windows"
    STATIC_EXT = .lib
    SHARED_EXT = .dll
    EXEC_EXT = .exe
else
    RM = rm -rf
    MKDIR = mkdir -p $(1)
    INSTALL = install -D
    UNINSTALL = rm -rf
    STATIC_EXT = .a
    SHARED_EXT = .so
    EXEC_EXT =
endif

# Directories
SOURCE_DIRECTORY = src
BUILD_DIRECTORY = build
LIBRARY_DIRECTORY = lib
INCLUDE_DIRECTORY = include

# Targets
STATIC_LIBRARY = $(LIBRARY_DIRECTORY)/libmemmgr$(STATIC_EXT)
SHARED_LIBRARY = $(LIBRARY_DIRECTORY)/libmemmgr$(SHARED_EXT)
TEST_EXECUTABLE = $(BUILD_DIRECTORY)/test$(EXEC_EXT)

# Source and object files
SOURCES = $(wildcard $(SOURCE_DIRECTORY)/*.c)
OBJECTS = $(SOURCES:$(SOURCE_DIRECTORY)/%.c=$(BUILD_DIRECTORY)/%.o)

# Test files
TEST_SOURCES = $(wildcard $(SOURCE_DIRECTORY)/test/*.c)

# Default target
all: $(STATIC_LIBRARY) $(SHARED_LIBRARY)

# Create build directory
$(BUILD_DIRECTORY):
	@$(call MKDIR,$(BUILD_DIRECTORY))

# Create lib directory
$(LIBRARY_DIRECTORY):
	@$(call MKDIR,$(LIBRARY_DIRECTORY))

# Build object files
$(BUILD_DIRECTORY)/%.o: $(SOURCE_DIRECTORY)/%.c | $(BUILD_DIRECTORY)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIRECTORY) -c $< -o $@

# Build static library
$(STATIC_LIBRARY): $(OBJECTS) | $(LIBRARY_DIRECTORY)
	$(AR) $(ARFLAGS) $@ $^

# Build include library
$(SHARED_LIBRARY): $(OBJECTS) | $(LIBRARY_DIRECTORY)
	$(CC) $(CFLAGS) -shared -o $@ $^

# Build tests
test: $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(TEST_SOURCES) $(STATIC_LIBRARY)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIRECTORY) -o $@ $^
	@echo "Test executable built: $@"

# Run tests
run-tests: test
	@echo "Running tests..."
	@$(TEST_EXECUTABLE)

# Clean build artifacts
clean:
	$(RM) $(BUILD_DIRECTORY) $(LIBRARY_DIRECTORY)

# Install libraries
install: $(STATIC_LIBRARY) $(SHARED_LIBRARY)
ifeq ($(OS),Windows_NT)
	@echo "Install not supported on Windows"
else
	@echo "Installing libraries..."
	@$(INSTALL) $(STATIC_LIBRARY) /usr/local/lib/libmemmgr$(STATIC_EXT)
	@$(INSTALL) $(SHARED_LIBRARY) /usr/local/lib/libmemmgr$(SHARED_EXT)
	@$(INSTALL) $(INCLUDE_DIRECTORY)/*.h /usr/local/include/
	@echo "Installation complete!"
endif

# Uninstall libraries
uninstall:
ifeq ($(OS),Windows_NT)
	@echo "Uninstall not supported on Windows"
else
	@echo "Uninstalling libraries..."
	@$(UNINSTALL) /usr/local/lib/libmemmgr$(STATIC_EXT)
	@$(UNINSTALL) /usr/local/lib/libmemmgr$(SHARED_EXT)
	@$(UNINSTALL) /usr/local/include/memmgr.h
	@echo "Uninstallation complete!"
endif

.PHONY: all test run-tests clean install uninstall
