# Makefile for graphics libraries

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -fPIC -O2
LDFLAGS = -shared

# Library targets
CONSOLE_LIB = ../lib_console.so
NCURSES_LIB = ../lib_ncurses.so

# All libraries
ALL_LIBS = $(CONSOLE_LIB) $(NCURSES_LIB)

# Sources
CONSOLE_SOURCES = ConsoleGraphics.cpp
NCURSES_SOURCES = NCursesGraphics.cpp

# Headers
CONSOLE_HEADERS = ConsoleGraphics.hpp ../IGraphicsLibrary.hpp
NCURSES_HEADERS = NCursesGraphics.hpp ../IGraphicsLibrary.hpp

# Game data dependencies (we need these object files)
GAME_DATA_OBJS = ../objs/game_data_core.o ../objs/game_data_board.o ../objs/game_data_movement.o ../objs/game_data_io.o
LIBFT = ../libft/Full_Libft.a

# System libraries
NCURSES_LIBS = -lncurses

# Default target
all: $(ALL_LIBS)

# Console graphics library (no external dependencies)
$(CONSOLE_LIB): $(CONSOLE_SOURCES) $(CONSOLE_HEADERS) $(GAME_DATA_OBJS)
	@echo "Building Console Graphics Library..."
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(CONSOLE_SOURCES) $(GAME_DATA_OBJS) $(LIBFT)
	@echo "Console Graphics Library built successfully!"

# NCurses graphics library (requires ncurses)
$(NCURSES_LIB): $(NCURSES_SOURCES) $(NCURSES_HEADERS) $(GAME_DATA_OBJS)
	@echo "Building NCurses Graphics Library..."
	@if ! pkg-config --exists ncurses 2>/dev/null && ! [ -f /usr/include/ncurses.h ] && ! [ -f /usr/local/include/ncurses.h ]; then \
		echo "Warning: NCurses development headers not found. Skipping NCurses library."; \
		echo "Install ncurses with: brew install ncurses (macOS) or apt-get install libncurses5-dev (Ubuntu)"; \
		touch $@; \
	else \
		$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(NCURSES_SOURCES) $(GAME_DATA_OBJS) $(LIBFT) $(NCURSES_LIBS); \
		echo "NCurses Graphics Library built successfully!"; \
	fi

# Individual library targets
console: $(CONSOLE_LIB)

ncurses: $(NCURSES_LIB)

# Check dependencies
check-deps:
	@echo "Checking dependencies..."
	@echo "Console Graphics: No external dependencies required"
	@if pkg-config --exists ncurses 2>/dev/null || [ -f /usr/include/ncurses.h ] || [ -f /usr/local/include/ncurses.h ]; then \
		echo "NCurses: Available"; \
	else \
		echo "NCurses: NOT AVAILABLE - install with 'brew install ncurses' or 'apt-get install libncurses5-dev'"; \
	fi

# Install dependencies (macOS with Homebrew)
install-deps-macos:
	@echo "Installing dependencies for macOS..."
	@if command -v brew >/dev/null 2>&1; then \
		brew install ncurses; \
	else \
		echo "Homebrew not found. Please install Homebrew first: https://brew.sh/"; \
	fi

# Install dependencies (Ubuntu/Debian)
install-deps-ubuntu:
	@echo "Installing dependencies for Ubuntu/Debian..."
	sudo apt-get update
	sudo apt-get install libncurses5-dev

# Clean all libraries
clean:
	@echo "Cleaning graphics libraries..."
	rm -f $(ALL_LIBS)
	@echo "Clean complete!"

# Force rebuild
rebuild: clean all

# Help target
help:
	@echo "Graphics Libraries Makefile"
	@echo "=========================="
	@echo "Targets:"
	@echo "  all              - Build all available graphics libraries"
	@echo "  console          - Build console graphics library only"
	@echo "  ncurses          - Build NCurses graphics library only"
	@echo "  check-deps       - Check for required dependencies"
	@echo "  install-deps-macos   - Install dependencies on macOS"
	@echo "  install-deps-ubuntu  - Install dependencies on Ubuntu/Debian"
	@echo "  clean            - Remove all built libraries"
	@echo "  rebuild          - Clean and rebuild all libraries"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Libraries:"
	@echo "  lib_console.so   - Simple console-based graphics"
	@echo "  lib_ncurses.so   - Enhanced terminal graphics with colors"

.PHONY: all console ncurses check-deps install-deps-macos install-deps-ubuntu clean rebuild help