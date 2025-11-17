# Project Management Module

Student B's contribution to the TODO List Task Management System.

## Features

### Basic Functions (360 lines)
- Project CRUD operations (Create, Read, Update, Delete)
- Project categorization with color labels
- Progress tracking (completed tasks / total tasks)
- Target date management
- Archive/unarchive functionality

### Innovation Function (120 lines)
- Heatmap Visualizer: 90-day task completion density map
- Visual progress tracking with color-coded blocks
- Statistics: total tasks, most active day, current streak

## File Structure

```
include/
├── entities/
│   └── Project.h          - Project entity class
├── dao/
│   └── ProjectDAO.h       - Database access layer
├── ProjectManager.h       - Main management class
└── HeatmapVisualizer.h    - Heatmap visualization

src/
├── entities/
│   └── Project.cpp
├── dao/
│   └── ProjectDAO.cpp
├── ProjectManager.cpp
└── HeatmapVisualizer.cpp

sqlite/                     - SQLite library files
```

## Database Schema

### projects table
```sql
CREATE TABLE projects (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    color_label TEXT,
    progress REAL DEFAULT 0.0,
    total_tasks INTEGER DEFAULT 0,
    completed_tasks INTEGER DEFAULT 0,
    target_date TEXT,
    archived INTEGER DEFAULT 0,
    created_date TEXT DEFAULT CURRENT_TIMESTAMP,
    updated_date TEXT DEFAULT CURRENT_TIMESTAMP
);
```

## Usage Example

```cpp
#include "ProjectManager.h"

// Initialize
ProjectManager pm("task_manager.db");
pm.initialize();

// Create project
Project p("My Project", "Description", "blue");
p.setTargetDate("2025-12-31");
int id = pm.createProject(p);

// Get project
Project* proj = pm.getProject(id);
cout << proj->getName() << endl;

// Update progress
proj->setCompletedTasks(5);
proj->setTotalTasks(10);
proj->updateProgress();
pm.updateProject(*proj);

// Generate heatmap
HeatmapVisualizer hv("task_manager.db");
hv.initialize();
cout << hv.generateHeatmap(90);
```

## Compilation

### Windows
```cmd
g++ -o test.exe test_project.cpp src/ProjectManager.cpp src/HeatmapVisualizer.cpp src/dao/ProjectDAO.cpp src/entities/Project.cpp -I./include -I./sqlite -L./sqlite -lsqlite3
```

### Linux/Mac
```bash
g++ -o test test_project.cpp src/ProjectManager.cpp src/HeatmapVisualizer.cpp src/dao/ProjectDAO.cpp src/entities/Project.cpp -I./include -I./sqlite -L./sqlite -lsqlite3
```

## Testing

Run the test program:
```cmd
compile_and_run_fixed.bat
```

All 10 tests should pass:
- ✅ Database initialization
- ✅ Project creation
- ✅ Project retrieval
- ✅ Project update
- ✅ Progress calculation
- ✅ Statistics
- ✅ Archive functionality
- ✅ Heatmap generation
- ✅ Heatmap statistics

## Dependencies

- C++11 or higher
- SQLite3 (included in `sqlite/` folder)

## Code Statistics

- Total lines: 480
- Basic functionality: 360 lines
- Innovation functionality: 120 lines
- Percentage: 20.9% of total project

## Author

Student B - Project Management Module
CSC3002 Course Project
