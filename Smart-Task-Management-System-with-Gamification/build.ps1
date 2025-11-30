# build.ps1 - PowerShell编译脚本

# 编译器设置
$CXX = "g++"
$CXXFLAGS = "-std=c++17 -Wall -Wextra -I./include -I./common -I./sqlite"
$LDFLAGS = "-L./sqlite -lsqlite3"
$TARGET = "bin/task_manager.exe"

# 源文件列表
$SOURCES = @(
    "src/main.cpp",
    "src/database/databasemanager.cpp",
    "src/database/DAO/ProjectDAO.cpp",
    "src/database/DAO/TaskDAOImpl.cpp",
    "src/project/Project.cpp",
    "src/project/ProjectManager.cpp",
    "src/statistics/StatisticsAnalyzer.cpp",
    "src/gamification/XPSystem.cpp",
    "src/HeatmapVisualizer/HeatmapVisualizer.cpp",
    "src/ui/UIManager.cpp",
    "src/task/task.cpp",
    "src/task/TaskManager.cpp"
)

# 创建目录
Write-Host "Creating directories..." -ForegroundColor Green
$directories = @("build", "bin", "build/database", "build/database/DAO", "build/project", 
                 "build/statistics", "build/gamification", "build/HeatmapVisualizer", 
                 "build/ui", "build/task")
foreach ($dir in $directories) {
    if (!(Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
    }
}

# 编译源文件
Write-Host "Compiling source files..." -ForegroundColor Green
$OBJECTS = @()
foreach ($source in $SOURCES) {
    $object = $source -replace '^src/', 'build/' -replace '\.cpp$', '.o'
    $OBJECTS += $object
    
    $dir = [System.IO.Path]::GetDirectoryName($object)
    if (!(Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
    }
    
    Write-Host "Compiling $source..."
    & $CXX $CXXFLAGS.Split(' ') -c $source -o $object
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error compiling $source" -ForegroundColor Red
        exit 1
    }
}

# 链接
Write-Host "Linking $TARGET..." -ForegroundColor Green
& $CXX $OBJECTS -o $TARGET $LDFLAGS.Split(' ')
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error linking" -ForegroundColor Red
    exit 1
}

# 复制DLL（如果存在）
if (Test-Path "sqlite/sqlite3.dll") {
    Write-Host "Copying SQLite3 DLL..." -ForegroundColor Green
    Copy-Item "sqlite/sqlite3.dll" "bin/" -ErrorAction SilentlyContinue
}

Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Executable: $TARGET" -ForegroundColor Yellow