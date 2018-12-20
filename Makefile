################################################################################
#|  Project: RicoTech
#|     Date: 2018-02-05
#|   Author: Dan Bechard
################################################################################

NAME := ricpak

# Directories
SRC_DIR := src
OBJ_DIR := obj
LIB_DIR := lib
BIN_DIR := bin
DATA_DIR := data

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(subst $(SRC_DIR)/,$(OBJ_DIR)/,$(SRC_FILES:.c=.o))
DEP_FILES := $(OBJ_FILES:.o=.d)
EXE_FILE := $(BIN_DIR)/$(NAME).exe

# -O0 Disabled
# -Og Debug
# -O2 Release
# -O3 Extreme (Careful, might make EXE bigger or invoke undefined behavior!)
FLAGS = -std=c99 -g -MMD -O0 -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter #-Wno-missing-field-initializers -Wno-missing-braces -Wno-deprecated-declarations #-Wno-error=incompatible-pointer-types
GCC_FLAGS = -fmax-errors=3
#GCC_FLAGS_LINUX = -fsanitize=address -fno-omit-frame-pointer
CLANG_FLAGS = -ferror-limit=3 -fcolor-diagnostics -Wno-macro-redefined
CC = gcc #-v
CFLAGS = $(FLAGS) #$(GCC_FLAGS)

.PHONY: default
default: prebuild $(EXE_FILE) postbuild

.PHONY: prebuild
prebuild: banner makedirs

.PHONY: banner
banner:
	$(info =========================================================)
	$(info #                        RICPAK                         #)
	$(info #              Copyright 2018  Dan Bechard              #)
	$(info =========================================================)

.PHONY: makedirs
makedirs:
	$(info --- prebuild --------------------------)
	$(info Creating $(BIN_DIR)/)
	@mkdir -p $(BIN_DIR)
	$(info Creating $(OBJ_DIR)/)
	@mkdir -p $(OBJ_DIR)

# Link executable
$(EXE_FILE): $(OBJ_FILES)
	$(info )
	$(info --- linking objects -------------------)
	$(info $(EXE_FILE))
	$(foreach O,$^,$(info - ${O}))
	@$(CC) -o $@ $^ $(LIBS)

# Compile C files into OBJ files and generate dependencies
$(OBJ_FILES): $(SRC_FILES)
	$(info )
	$(info --- compiling sources -----------------)
	$(info $@)
	$(foreach S,$^,$(info - ${S}))
	@$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $@ -c $<

# Include generated dependency files
-include $(DEP_FILES)

# Copy dlls
#$(BIN_DIR)/%: $(DLL_DIR)/%
#	$(info [DLL] $^ -> $@)
#	@cp $^ $@

.PHONY: postbuild
postbuild:

.PHONY: clean
clean:
	$(info - $(EXE_FILE))
	-@rm -f $(EXE_FILE)
	$(info - $(OBJ_FILES))
	-@rm -f $(OBJ_FILES)
	$(info - $(DEP_FILES))
	-@rm -f $(DEP_FILES)

################################################################################
# Miscellaneous notes
################################################################################
#|  $@ File name of target
#|  $< Name of first prerequisite
#|  $^ Name of all prerequisites, with spaces between them
#|  $? Name of all prerequisites which have changed
#|  %  Wildcard, can be neighbored by prefix, suffix, or both
################################################################################

# Note: To use this, run e.g. `make print-SOURCES`
print-%  : ; @echo $* = $($*)
