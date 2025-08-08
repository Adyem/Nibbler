# Makefile for graphics libraries

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -fPIC -O2
LDFLAGS = -shared

# Library targets
SDL2_LIB = ../lib_sdl2.so
NCURSES_LIB = ../lib_ncurses.so
SFML_LIB = ../lib_sfml.so

# All libraries
ALL_LIBS = $(NCURSES_LIB) $(SDL2_LIB)
# SFML_LIB temporarily disabled until implementation is complete

# Sources
SDL2_SOURCES = SDL2Graphics.cpp
NCURSES_SOURCES = NCursesGraphics.cpp
SFML_SOURCES = SFMLGraphics.cpp

# Headers
SDL2_HEADERS = SDL2Graphics.hpp ../IGraphicsLibrary.hpp
NCURSES_HEADERS = NCursesGraphics.hpp ../IGraphicsLibrary.hpp
SFML_HEADERS = SFMLGraphics.hpp ../IGraphicsLibrary.hpp

# Game data dependencies (we need these object files)
GAME_DATA_OBJS = ../objs/game_data_core.o ../objs/game_data_board.o ../objs/game_data_movement.o ../objs/game_data_io.o ../objs/MenuSystem.o
LIBFT = ../libft/Full_Libft.a

# System libraries
NCURSES_LIBS = -lncurses
SDL2_CFLAGS = $(shell pkg-config --cflags sdl2 2>/dev/null || echo "-I/opt/homebrew/include/SDL2")
SDL2_LIBS = $(shell pkg-config --libs sdl2 2>/dev/null || echo "-L/opt/homebrew/lib -lSDL2")
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# Default target
all: $(ALL_LIBS)

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

# SDL2 graphics library (requires SDL2)
$(SDL2_LIB): $(SDL2_SOURCES) $(SDL2_HEADERS) $(GAME_DATA_OBJS)
	@echo "Building SDL2 Graphics Library..."
	@if ! pkg-config --exists sdl2 2>/dev/null && ! [ -f /usr/include/SDL2/SDL.h ] && ! [ -f /usr/local/include/SDL2/SDL.h ] && ! [ -f /opt/homebrew/include/SDL2/SDL.h ]; then \
		echo "Warning: SDL2 development headers not found. Skipping SDL2 library."; \
		echo "Install SDL2 with: brew install sdl2 (macOS) or apt-get install libsdl2-dev (Ubuntu)"; \
		touch $@; \
	else \
		$(CXX) $(CXXFLAGS) $(SDL2_CFLAGS) $(LDFLAGS) -o $@ $(SDL2_SOURCES) $(GAME_DATA_OBJS) $(LIBFT) $(SDL2_LIBS); \
		echo "SDL2 Graphics Library built successfully!"; \
	fi

# SFML graphics library (requires SFML)
$(SFML_LIB): $(SFML_SOURCES) $(SFML_HEADERS) $(GAME_DATA_OBJS)
	@echo "Building SFML Graphics Library..."
	@if ! pkg-config --exists sfml-graphics 2>/dev/null && ! [ -f /usr/include/SFML/Graphics.hpp ] && ! [ -f /usr/local/include/SFML/Graphics.hpp ] && ! [ -f /opt/homebrew/include/SFML/Graphics.hpp ]; then \
		echo "Warning: SFML development headers not found. Skipping SFML library."; \
		echo "Install SFML with: brew install sfml (macOS) or apt-get install libsfml-dev (Ubuntu)"; \
		touch $@; \
	else \
		$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SFML_SOURCES) $(GAME_DATA_OBJS) $(LIBFT) $(SFML_LIBS); \
		echo "SFML Graphics Library built successfully!"; \
	fi

# Individual library targets
ncurses: $(NCURSES_LIB)

sdl2: $(SDL2_LIB)

sfml: $(SFML_LIB)

# Check dependencies
check-deps:
	@echo "Checking dependencies..."
	@if pkg-config --exists ncurses 2>/dev/null || [ -f /usr/include/ncurses.h ] || [ -f /usr/local/include/ncurses.h ]; then \
		echo "NCurses: Available"; \
	else \
		echo "NCurses: NOT AVAILABLE - install with 'brew install ncurses' or 'apt-get install libncurses5-dev'"; \
	fi
	@if pkg-config --exists sdl2 2>/dev/null || [ -f /usr/include/SDL2/SDL.h ] || [ -f /usr/local/include/SDL2/SDL.h ] || [ -f /opt/homebrew/include/SDL2/SDL.h ]; then \
		echo "SDL2: Available"; \
	else \
		echo "SDL2: NOT AVAILABLE - install with 'brew install sdl2' or 'apt-get install libsdl2-dev'"; \
	fi
	@if pkg-config --exists sfml-graphics 2>/dev/null || [ -f /usr/include/SFML/Graphics.hpp ] || [ -f /usr/local/include/SFML/Graphics.hpp ] || [ -f /opt/homebrew/include/SFML/Graphics.hpp ]; then \
		echo "SFML: Available"; \
	else \
		echo "SFML: NOT AVAILABLE - install with 'brew install sfml' or 'apt-get install libsfml-dev'"; \
	fi

# Install dependencies (macOS with Homebrew)
install-deps-macos:
	@echo "Installing dependencies for macOS..."
	@if command -v brew >/dev/null 2>&1; then \
		brew install ncurses sdl2 sfml; \
	else \
		echo "Homebrew not found. Please install Homebrew first: https://brew.sh/"; \
	fi

# Install dependencies (Ubuntu/Debian)
install-deps-ubuntu:
	@echo "Installing dependencies for Ubuntu/Debian..."
	sudo apt-get update
	sudo apt-get install libncurses5-dev libsdl2-dev libsfml-dev

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
	@echo "  ncurses          - Build NCurses graphics library only"
	@echo "  sdl2             - Build SDL2 graphics library only"
	@echo "  sfml             - Build SFML graphics library only"
	@echo "  check-deps       - Check for required dependencies"
	@echo "  install-deps-macos   - Install dependencies on macOS"
	@echo "  install-deps-ubuntu  - Install dependencies on Ubuntu/Debian"
	@echo "  clean            - Remove all built libraries"
	@echo "  rebuild          - Clean and rebuild all libraries"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Libraries:"
	@echo "  lib_ncurses.so   - Enhanced terminal graphics with colors"
	@echo "  lib_sdl2.so      - Modern windowed graphics with SDL2"
	@echo "  lib_sfml.so      - Advanced graphics with SFML"

.PHONY: all ncurses sdl2 sfml check-deps install-deps-macos install-deps-ubuntu clean rebuild help