# Compiler configuration
CC       := gcc
CFLAGS   := -Wall -Wextra -pedantic -O3 -march=native -mavx2 -std=c11
LDFLAGS  :=
INCLUDES := -Iinclude

# Directories
SRC_DIR  := src
INC_DIR  := include
OBJ_DIR  := obj
BIN_DIR  := bin

# Target binary name
TARGET   := $(BIN_DIR)/planar_iso

# Source and Object matching
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean debug profile

# Default production build rule
all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "🔗 Linking executable with native SIMD target: $@"
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "🔨 Compiling $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Direct directory creation rules
$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

# Debug build configuration (with AddressSanitizer and Valgrind hooks)
debug: CFLAGS := -Wall -Wextra -g3 -O0 -std=c11 -fsanitize=address,undefined
debug: clean $(TARGET)
	@echo "⚠️ Debug binary built with Address & Undefined Behavior Sanitizers."

# Profiling build layout (for gprof or perf verification)
profile: CFLAGS := -Wall -Wextra -O2 -pg -march=native -mavx2 -std=c11
profile: clean $(TARGET)
	@echo "📊 Profiling binary built. Run target then execute 'gprof'."

# Housekeeping
clean:
	@echo "🧹 Cleaning workspace object files and binaries..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
