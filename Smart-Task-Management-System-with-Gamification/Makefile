# Makefile for Smart Task Management System
# C++17 Standard, SQLite3 Database

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include -I./common -I./sqlite
LDFLAGS = -L./sqlite -lsqlite3

# For Windows with SQLite DLL, add the sqlite directory to PATH
ifeq ($(OS),Windows_NT)
    LDFLAGS += -static -static-libgcc -static-libstdc++
    TARGET = $(BIN_DIR)/task_manager.exe
else
    TARGET = $(BIN_DIR)/task_manager
endif

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Source files
SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/database/databasemanager.cpp \
       $(SRC_DIR)/database/DAO/ProjectDAO.cpp \
       $(SRC_DIR)/database/DAO/TaskDAOImpl.cpp \
       $(SRC_DIR)/project/Project.cpp \
       $(SRC_DIR)/project/ProjectManager.cpp \
       $(SRC_DIR)/statistics/StatisticsAnalyzer.cpp \
       $(SRC_DIR)/gamification/XPSystem.cpp \
       $(SRC_DIR)/HeatmapVisualizer/HeatmapVisualizer.cpp \
       $(SRC_DIR)/ui/UIManager.cpp \
       $(SRC_DIR)/task/task.cpp \
       $(SRC_DIR)/task/TaskManager.cpp

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

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
	@mkdir -p $(BUILD_DIR)/task
	@mkdir -p $(BIN_DIR)

# Link
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete!"
	@echo "Note: On Windows, ensure sqlite3.dll is in the same directory as the executable or in PATH"

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

# Install SQLite DLL (Windows only)
install-dll:
ifeq ($(OS),Windows_NT)
	@echo "Copying SQLite3 DLL to binary directory..."
	@cp sqlite/sqlite3.dll $(BIN_DIR)/
	@echo "DLL installed to $(BIN_DIR)/"
else
	@echo "This target is only for Windows systems"
endif

# Show build information
info:
	@echo "Build Information:"
	@echo "  C++ Compiler: $(CXX)"
	@echo "  C++ Flags: $(CXXFLAGS)"
	@echo "  Link Flags: $(LDFLAGS)"
	@echo "  Source Files: $(words $(SRCS))"
	@echo "  Target: $(TARGET)"

# Help
help:
	@echo "Smart Task Management System - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build the project (default)"
	@echo "  clean      - Remove build files"
	@echo "  run        - Build and run the program"
	@echo "  debug      - Build with debug symbols"
	@echo "  release    - Build optimized release version"
	@echo "  install-dll- Copy SQLite3 DLL to binary dir (Windows)"
	@echo "  info       - Show build information"
	@echo "  help       - Show this help message"

.PHONY: all clean run debug release install-dll info help directories