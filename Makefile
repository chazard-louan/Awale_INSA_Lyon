# Tool macros
CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra -std=c11
CXXFLAGS := -Wall -Wextra -std=c++17
DBGFLAGS := -g
COBJFLAGS := $(CFLAGS) -c

# Path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug

# Target names
TARGET_NAMES := serveur client

ifeq ($(OS),Windows_NT)
    EXE := .exe
else
    EXE :=
endif

TARGETS := $(addprefix $(BIN_PATH)/,$(addsuffix $(EXE),$(TARGET_NAMES)))
TARGETS_DEBUG := $(addprefix $(DBG_PATH)/,$(addsuffix $(EXE),$(TARGET_NAMES)))

# Source and object files
SRC := $(wildcard $(SRC_PATH)/*.c)
OBJ := $(addprefix $(OBJ_PATH)/,$(notdir $(SRC:.c=.o)))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/,$(notdir $(SRC:.c=.o)))

# Individual executable dependencies
Serveur_OBJ := $(OBJ_PATH)/serveur.o $(filter-out $(OBJ_PATH)/client.o $(OBJ_PATH)/serveur.o,$(OBJ))
Client_OBJ := $(OBJ_PATH)/client.o $(filter-out $(OBJ_PATH)/client.o $(OBJ_PATH)/serveur.o,$(OBJ))

# Default rule
default: makedir all

# Compilation rules
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(COBJFLAGS) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(COBJFLAGS) $(DBGFLAGS) -o $@ $<

# Linking rules
$(BIN_PATH)/serveur$(EXE): $(Serveur_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_PATH)/client$(EXE): $(Client_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(DBG_PATH)/serveur$(EXE): $(Serveur_OBJ)
	$(CC) $(CFLAGS) $(DBGFLAGS) -o $@ $^

$(DBG_PATH)/client$(EXE): $(Client_OBJ)
	$(CC) $(CFLAGS) $(DBGFLAGS) -o $@ $^

# Directories
.PHONY: makedir
makedir:
	mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH)

# Main targets
.PHONY: all debug clean distclean

all: makedir $(TARGETS)

debug: makedir $(TARGETS_DEBUG)

# Cleaning
clean:
	rm -f $(TARGETS) $(TARGETS_DEBUG)

distclean: clean
	rm -f $(OBJ) $(OBJ_DEBUG)
