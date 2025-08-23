# Makefile for graphics libraries

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -fPIC -O2 -Iinclude
LDFLAGS = -shared

# Parent-provided object directory fallback
OBJ_DIR ?= ../objs

# Output directory for shared libs (prefer absolute from parent)
OUT_DIR := $(if $(DLLIBS_DIR),$(DLLIBS_DIR),../dllibs)

# Library targets
SDL2_LIB = $(OUT_DIR)/lib_sdl2.so
NCURSES_LIB = $(OUT_DIR)/lib_ncurses.so
OPENGL_LIB = $(OUT_DIR)/lib_opengl.so
RAYLIB_LIB = $(OUT_DIR)/lib_raylib.so

# All libraries
ALL_LIBS = $(NCURSES_LIB) $(SDL2_LIB) $(OPENGL_LIB) $(RAYLIB_LIB)

# Sources (moved to src/)
SDL2_SOURCES = src/SDL2Graphics.cpp
NCURSES_SOURCES = src/NCursesGraphics.cpp
OPENGL_SOURCES = src/OpenGLGraphics.cpp
RAYLIB_SOURCES = src/RaylibGraphics.cpp

# Headers (moved to include/)
SDL2_HEADERS = include/SDL2Graphics.hpp ../IGraphicsLibrary.hpp
NCURSES_HEADERS = include/NCursesGraphics.hpp ../IGraphicsLibrary.hpp
OPENGL_HEADERS = include/OpenGLGraphics.hpp ../IGraphicsLibrary.hpp
RAYLIB_HEADERS = include/RaylibGraphics.hpp ../IGraphicsLibrary.hpp

# Game data dependencies (object files produced by top-level)
# NOTE: We no longer link these into the shared libraries (to avoid non-PIC issues).
#       Symbols will be resolved at runtime from the main executable (exported via -rdynamic).
GAME_DATA_OBJS = $(OBJ_DIR)/game_data_core.o $(OBJ_DIR)/game_data_board.o $(OBJ_DIR)/game_data_movement.o $(OBJ_DIR)/game_data_io.o $(OBJ_DIR)/MenuSystem.o
LIBFT = ../libft/Full_Libft.a

# System libraries (prefer pkg-config; provide sane Linux fallbacks)
NCURSES_LIBS = $(shell pkg-config --libs ncursesw 2>/dev/null || pkg-config --libs ncurses 2>/dev/null || echo "-lncurses")
SDL2_CFLAGS = $(shell pkg-config --cflags sdl2 sdl2_ttf 2>/dev/null || echo "")
SDL2_LIBS = $(shell pkg-config --libs sdl2 sdl2_ttf 2>/dev/null || echo "-lSDL2 -lSDL2_ttf")
OPENGL_CFLAGS = $(shell pkg-config --cflags glfw3 2>/dev/null || echo "")
OPENGL_LIBS = $(shell pkg-config --libs glfw3 2>/dev/null || echo "-lglfw -lGL")
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib 2>/dev/null || echo "")
RAYLIB_LIBS = $(shell pkg-config --libs raylib 2>/dev/null || echo "-lraylib")

# Default target
all: $(OUT_DIR) $(ALL_LIBS)

$(OUT_DIR):
	@mkdir -p $(OUT_DIR)

# Helper to run link and echo success only if it succeeds
# Usage: $(call link_so, CXXFLAGS..., LIBS..., output, sources)
# Not using a generic function to keep it simple/portable.

# NCurses graphics library (requires ncurses/ncursesw)
$(NCURSES_LIB): $(NCURSES_SOURCES) $(NCURSES_HEADERS) | $(OUT_DIR)
	@echo "Building NCurses Graphics Library..."
	@if ! pkg-config --exists ncursesw 2>/dev/null && ! pkg-config --exists ncurses 2>/dev/null \
		&& ! [ -f /usr/include/ncurses.h ] && ! [ -f /usr/include/ncursesw/ncurses.h ] \
		&& ! [ -f /usr/local/include/ncurses.h ]; then \
		echo "Warning: NCurses development headers not found. Skipping NCurses library."; \
		:; \
	else \
		$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(NCURSES_SOURCES) $(NCURSES_LIBS) && echo "NCurses Graphics Library built successfully!"; \
	fi

# SDL2 graphics library (requires SDL2 and SDL2_ttf)
$(SDL2_LIB): $(SDL2_SOURCES) $(SDL2_HEADERS) | $(OUT_DIR)
	@echo "Building SDL2 Graphics Library..."
	@if ! pkg-config --exists sdl2 2>/dev/null && ! [ -f /usr/include/SDL2/SDL.h ] && ! [ -f /usr/local/include/SDL2/SDL.h ]; then \
		echo "Warning: SDL2 development headers not found. Skipping SDL2 library."; \
		:; \
	elif ! pkg-config --exists sdl2_ttf 2>/dev/null && ! [ -f /usr/include/SDL2/SDL_ttf.h ] && ! [ -f /usr/local/include/SDL2/SDL_ttf.h ]; then \
		echo "Warning: SDL2_ttf development headers not found. Skipping SDL2 library."; \
		:; \
	else \
		$(CXX) $(CXXFLAGS) $(SDL2_CFLAGS) $(LDFLAGS) -o $@ $(SDL2_SOURCES) $(SDL2_LIBS) && echo "SDL2 Graphics Library built successfully!"; \
	fi

# OpenGL graphics library (requires GLFW and OpenGL)
$(OPENGL_LIB): $(OPENGL_SOURCES) $(OPENGL_HEADERS) | $(OUT_DIR)
	@echo "Building OpenGL Graphics Library..."
	@if ! pkg-config --exists glfw3 2>/dev/null && ! [ -f /usr/include/GLFW/glfw3.h ] && ! [ -f /usr/local/include/GLFW/glfw3.h ]; then \
		echo "Warning: GLFW development headers not found. Skipping OpenGL library."; \
		:; \
	else \
		$(CXX) $(CXXFLAGS) $(OPENGL_CFLAGS) $(LDFLAGS) -o $@ $(OPENGL_SOURCES) $(OPENGL_LIBS) && echo "OpenGL Graphics Library built successfully!"; \
	fi

# Raylib graphics library (requires raylib)
$(RAYLIB_LIB): $(RAYLIB_SOURCES) $(RAYLIB_HEADERS) | $(OUT_DIR)
	@echo "Building Raylib Graphics Library..."
	@if ! pkg-config --exists raylib 2>/dev/null \
		&& ! [ -f /usr/include/raylib.h ] && ! [ -f /usr/local/include/raylib.h ] \
		&& ! [ -f /usr/lib/x86_64-linux-gnu/libraylib.so ] && ! [ -f /usr/local/lib/libraylib.so ]; then \
		echo "Warning: raylib development package not found. Skipping raylib library."; \
		:; \
	else \
		$(CXX) $(CXXFLAGS) $(RAYLIB_CFLAGS) $(LDFLAGS) -o $@ $(RAYLIB_SOURCES) $(RAYLIB_LIBS) && echo "Raylib Graphics Library built successfully!"; \
	fi

# Individual library targets
ncurses: $(NCURSES_LIB)

sdl2: $(SDL2_LIB)

opengl: $(OPENGL_LIB)

raylib: $(RAYLIB_LIB)

# Check dependencies
check-deps:
	@echo "Checking dependencies..."
	@if pkg-config --exists ncursesw 2>/dev/null || pkg-config --exists ncurses 2>/dev/null || [ -f /usr/include/ncurses.h ] || [ -f /usr/include/ncursesw/ncurses.h ] || [ -f /usr/local/include/ncurses.h ]; then \
		echo "NCurses: Available"; \
	else \
		echo "NCurses: NOT AVAILABLE - install with 'apt-get install libncurses5-dev' or 'brew install ncurses'"; \
	fi
	@if pkg-config --exists sdl2 2>/dev/null || [ -f /usr/include/SDL2/SDL.h ] || [ -f /usr/local/include/SDL2/SDL.h ]; then \
		if pkg-config --exists sdl2_ttf 2>/dev/null || [ -f /usr/include/SDL2/SDL_ttf.h ] || [ -f /usr/local/include/SDL2/SDL_ttf.h ]; then \
			echo "SDL2: Available (with TTF support)"; \
		else \
			echo "SDL2: Available (missing TTF support) - install SDL2_ttf with 'apt-get install libsdl2-ttf-dev' or 'brew install sdl2_ttf'"; \
		fi; \
	else \
		echo "SDL2: NOT AVAILABLE - install with 'apt-get install libsdl2-dev libsdl2-ttf-dev' or 'brew install sdl2 sdl2_ttf'"; \
	fi
	@if PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig pkg-config --exists sfml-graphics 2>/dev/null || [ -f /usr/include/SFML/Graphics.hpp ] || [ -f /usr/local/include/SFML/Graphics.hpp ]; then \
		echo "SFML: Available"; \
	else \
		echo "SFML: NOT AVAILABLE - install with 'apt-get install libsfml-dev' or 'brew install sfml'"; \
	fi
	@if pkg-config --exists glfw3 2>/dev/null || [ -f /usr/include/GLFW/glfw3.h ] || [ -f /usr/local/include/GLFW/glfw3.h ]; then \
		echo "OpenGL/GLFW: Available"; \
	else \
		echo "OpenGL/GLFW: NOT AVAILABLE - install with 'apt-get install libglfw3-dev libgl1-mesa-dev' or 'brew install glfw'"; \
	fi
	@if pkg-config --exists raylib 2>/dev/null || [ -f /usr/include/raylib.h ] || [ -f /usr/local/include/raylib.h ] || [ -f /usr/lib/x86_64-linux-gnu/libraylib.so ] || [ -f /usr/local/lib/libraylib.so ]; then \
		echo "Raylib: Available"; \
	else \
		echo "Raylib: NOT AVAILABLE - install with 'apt-get install libraylib-dev' or 'brew install raylib'"; \
	fi

# Install dependencies (macOS with Homebrew)
install-deps-macos:
	@echo "Installing dependencies for macOS..."
	@if command -v brew >/dev/null 2>&1; then \
		brew install ncurses sdl2 sdl2_ttf glfw raylib; \
	else \
		echo "Homebrew not found. Please install Homebrew first: https://brew.sh/"; \
	fi

# Install dependencies (Ubuntu/Debian)
install-deps-ubuntu:
	@echo "Installing dependencies for Ubuntu/Debian..."
	sudo apt-get update
	sudo apt-get install libncurses5-dev libsdl2-dev libsdl2-ttf-dev libglfw3-dev libgl1-mesa-dev libraylib-dev

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
	@echo "  opengl           - Build OpenGL graphics library only"
	@echo "  raylib           - Build Raylib graphics library only"
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
	@echo "  lib_opengl.so    - Modern OpenGL graphics with GLFW"
	@echo "  lib_raylib.so    - Windowed graphics with raylib"

.PHONY: all ncurses sdl2 opengl raylib check-deps install-deps-macos install-deps-ubuntu clean rebuild help