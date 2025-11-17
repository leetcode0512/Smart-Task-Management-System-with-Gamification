# Makefile for Smart Task Management System
# C++17 Standard, SQLite3 Database

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include -I./common
LDFLAGS = -lsqlite3

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Source files
SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/database/databasemanager.cpp \
       $(SRC_DIR)/database/DAO/ProjectDAO.cpp \
       $(SRC_DIR)/project/Project.cpp \
       $(SRC_DIR)/project/ProjectManager.cpp \
       $(SRC_DIR)/statistics/StatisticsAnalyzer.cpp \
       $(SRC_DIR)/gamification/XPSystem.cpp \
       $(SRC_DIR)/HeatmapVisualizer/HeatmapVisualizer.cpp \
       $(SRC_DIR)/ui/UIManager.cpp
# Note: ReminderSystem excluded due to missing DAO implementation

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Executable
TARGET = $(BIN_DIR)/task_manager

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/database
	@mkdir -p $(BUILD_DIR)/database/DAO
	@mkdir -p $(BUILD_DIR)/project
	@mkdir -p $(BUILD_DIR)/reminder
	@mkdir -p $(BUILD_DIR)/statistics
	@mkdir -p $(BUILD_DIR)/gamification
	@mkdir -p $(BUILD_DIR)/HeatmapVisualizer
	@mkdir -p $(BUILD_DIR)/ui
	@mkdir -p $(BIN_DIR)

# Link
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete!"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	rm -f *.db *.db-shm *.db-wal
	@echo "Clean complete!"

# Run
run: all
	@echo "Running Task Manager..."
	@./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: clean all

# Release build
release: CXXFLAGS += -O2 -DNDEBUG
release: clean all

# Help
help:
	@echo "Smart Task Management System - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all      - Build the project (default)"
	@echo "  clean    - Remove build files"
	@echo "  run      - Build and run the program"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Build optimized release version"
	@echo "  help     - Show this help message"

.PHONY: all clean run debug release help directories
