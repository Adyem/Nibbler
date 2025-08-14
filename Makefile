ifeq ($(OS),Windows_NT)
    EXE_EXT := .exe
else
    EXE_EXT :=
endif

NAME        = nibbler$(EXE_EXT)
NAME_DEBUG  = nibbler_debug$(EXE_EXT)

HEADER      = game_data.hpp IGraphicsLibrary.hpp LibraryManager.hpp GameEngine.hpp MenuSystem.hpp \

SRC         = game_data_core.cpp game_data_board.cpp game_data_movement.cpp game_data_io.cpp main.cpp LibraryManager.cpp GameEngine.cpp MenuSystem.cpp \

CC          = g++

OPT_LEVEL ?= 0

ifeq ($(OPT_LEVEL),0)
	OPT_FLAGS = -O0 -g
else ifeq ($(OPT_LEVEL),1)
	OPT_FLAGS = -O1 -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections
else ifeq ($(OPT_LEVEL),2)
	OPT_FLAGS = -O2 -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections
else ifeq ($(OPT_LEVEL),3)
	OPT_FLAGS = -O3 -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections
else
	$(error Unsupported OPT_LEVEL=$(OPT_LEVEL))
endif

COMPILE_FLAGS = -Wall -Werror -Wextra -std=c++17 -Wmissing-declarations \
				-Wshadow  -Wformat=2 -Wundef \
				-Wfloat-equal -Wodr  \
				-Wzero-as-null-pointer-constant  $(OPT_FLAGS)

CFLAGS = $(COMPILE_FLAGS)

ifeq ($(OS),Windows_NT)
    MKDIR   = mkdir
    RMDIR   = rmdir /S /Q
    RM      = del /F /Q
else
    MKDIR   = mkdir -p
    RMDIR   = rm -rf
    RM      = rm -f
endif

LIBFT_DIR   = ./libft

OBJ_DIR         = ./objs
OBJ_DIR_DEBUG   = ./objs_debug
DLLIBS_DIR      = ./dllibs

# Export absolute paths to sub-makes so graphics_libs can use correct paths
export OBJ_DIR := $(abspath $(OBJ_DIR))
export OBJ_DIR_DEBUG := $(abspath $(OBJ_DIR_DEBUG))
export DLLIBS_DIR := $(abspath $(DLLIBS_DIR))

# Objects needed by graphics_libs shared libraries
SHARED_OBJS = \
	$(OBJ_DIR)/game_data_core.o \
	$(OBJ_DIR)/game_data_board.o \
	$(OBJ_DIR)/game_data_movement.o \
	$(OBJ_DIR)/game_data_io.o \
	$(OBJ_DIR)/MenuSystem.o

ENABLE_LTO  ?= 0
ENABLE_PGO  ?= 0
export ENABLE_LTO ENABLE_PGO

ifeq ($(ENABLE_LTO),1)
    COMPILE_FLAGS   += -flto
    LDFLAGS  += -flto
endif

ifeq ($(ENABLE_PGO),1)
    COMPILE_FLAGS  += -fprofile-generate
endif

ifeq ($(DEBUG),1)
    CFLAGS    += -DDEBUG=1
    OBJ_DIR    = $(OBJ_DIR_DEBUG)
    TARGET     = $(NAME_DEBUG)
    LIBFT      = $(LIBFT_DIR)/Full_Libft_debug.a
else
    TARGET     = $(NAME)
    LIBFT      = $(LIBFT_DIR)/Full_Libft.a
endif

export COMPILE_FLAGS

# Re-export absolute paths after OBJ_DIR may have changed due to DEBUG
export OBJ_DIR := $(abspath $(OBJ_DIR))
export DLLIBS_DIR := $(abspath $(DLLIBS_DIR))

ifeq ($(OS),Windows_NT)
    LDFLAGS     = $(LIBFT) -ldl
else
    # Export all symbols from the main executable so dlopened plugins can
    # resolve references (e.g., MenuSystem, GameEngine) at runtime.
    LDFLAGS     = $(LIBFT) -lreadline -ldl -rdynamic
endif

OBJS        = $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: dirs graphics_libs $(TARGET)

dirs:
	-$(MKDIR) $(OBJ_DIR)
	-$(MKDIR) $(OBJ_DIR_DEBUG)
	-$(MKDIR) $(DLLIBS_DIR)

debug:
	$(MAKE) all DEBUG=1

$(TARGET): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR) $(if $(DEBUG), debug)

$(OBJ_DIR)/%.o: %.cpp $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

graphics_libs: $(SHARED_OBJS) $(LIBFT)
	$(MAKE) -C graphics_libs

# Explicitly force a full rebuild of dynamic libs (clean + build)
graphics_re: dirs $(SHARED_OBJS) $(LIBFT)
	$(MAKE) -C graphics_libs rebuild

clean:
	-$(RM) $(OBJ_DIR)/*.o $(OBJ_DIR_DEBUG)/*.o
	$(MAKE) -C $(LIBFT_DIR) fclean
	$(MAKE) -C graphics_libs clean
	-$(RMDIR) $(DLLIBS_DIR)

fclean: clean
	-$(RM) $(NAME) $(NAME_DEBUG)
	-$(RMDIR) $(OBJ_DIR) $(OBJ_DIR_DEBUG) data
	-$(RM) lib_*.so

# Ensure dynamic libs are fully rebuilt as part of re (sequentially)
re:
	$(MAKE) fclean
	$(MAKE) all

both: all debug

re_both: re both

.PHONY: all dirs clean fclean re debug both re_both graphics_libs graphics_re
