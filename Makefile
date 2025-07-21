# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude -pthread -O2

# Directories
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build

# Source and object files
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Output binary
TARGET := $(BUILD_DIR)/wifi_simulator

# Default target
all: $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@echo "Created build directory"

# Build the target
$(TARGET): $(BUILD_DIR) $(OBJ_FILES)
	@echo "Linking WiFi Simulator..."
	@$(CXX) $(CXXFLAGS) $(OBJ_FILES) -o $@
	@echo "Build completed: $(TARGET)"

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean completed"

# Create build directory
setup:
	@mkdir -p $(BUILD_DIR)
	@echo "Setup completed"

# Run the program
run: $(TARGET)
	@echo "Running WiFi Communication Simulator..."
	@echo ""
	@./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "Debug build completed"

# Release build with optimizations
release: CXXFLAGS += -O3 -DNDEBUG
release: clean $(TARGET)
	@echo "Release build completed"

# Show help
help:
	@echo "WiFi Communication Simulator - Build System"
	@echo "Available targets:"
	@echo "  all     - Build the simulator (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  setup   - Create build directory"
	@echo "  run     - Build and run the simulator"
	@echo "  debug   - Build with debug symbols"
	@echo "  release - Build optimized release version"
	@echo "  help    - Show this help message"

# Phony targets
.PHONY: all clean setup run debug release help
