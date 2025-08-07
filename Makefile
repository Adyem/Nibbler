ifeq ($(OS),Windows_NT)
    EXE_EXT := .exe
else
    EXE_EXT :=
endif

NAME        = nibbler$(EXE_EXT)
NAME_DEBUG  = nibbler_debug$(EXE_EXT)

HEADER      = game_data.hpp IGraphicsLibrary.hpp LibraryManager.hpp GameEngine.hpp \

SRC         = game_data_core.cpp game_data_board.cpp game_data_movement.cpp game_data_io.cpp main.cpp LibraryManager.cpp GameEngine.cpp \

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

ifeq ($(OS),Windows_NT)
    LDFLAGS     = $(LIBFT) -ldl
else
    LDFLAGS     = $(LIBFT) -lreadline -ldl
endif

OBJS        = $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: dirs $(TARGET) graphics_libs

dirs:
	-$(MKDIR) $(OBJ_DIR)
	-$(MKDIR) $(OBJ_DIR_DEBUG)

debug:
	$(MAKE) all DEBUG=1

$(TARGET): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR) $(if $(DEBUG), debug)

$(OBJ_DIR)/%.o: %.cpp $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

graphics_libs:
	$(MAKE) -C graphics_libs

clean:
	-$(RM) $(OBJ_DIR)/*.o $(OBJ_DIR_DEBUG)/*.o
	$(MAKE) -C $(LIBFT_DIR) fclean
	$(MAKE) -C graphics_libs clean

fclean: clean
	-$(RM) $(NAME) $(NAME_DEBUG)
	-$(RMDIR) $(OBJ_DIR) $(OBJ_DIR_DEBUG) data
	-$(RM) lib_*.so

re: fclean all

both: all debug

re_both: re both

.PHONY: all dirs clean fclean re debug both re_both graphics_libs
