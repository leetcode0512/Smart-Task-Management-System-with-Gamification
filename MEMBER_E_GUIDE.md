# 成员E (毛靖淇) - 开发指南

## 📊 当前项目进度总结

### 1. 项目整体进度：约40%

**已完成的模块：**
- ✅ **数据库层** (成员D - Yu Zhixuan): 80%完成
  - DatabaseManager基础实现完成
  - 所有数据表结构定义完成
  - DAO接口定义完成（需实现具体实现类）
  
- ✅ **核心实体定义**: 100%完成
  - entities.h定义了7个核心实体
  - 完整的实体文档ENTITY_DEFINITION.md
  
- ✅ **项目管理模块** (成员B - Zhou Tianjian): 70%完成
  - Project类完整实现
  - ProjectManager业务逻辑完成（使用map存储）
  - HeatmapVisualizer头文件定义完成
  - **待集成**: 需要替换为数据库存储
  
- ✅ **提醒系统** (成员A - Fei Yifan): 部分完成
  - ReminderSystem框架存在
  - 已集成ReminderDAO

**未完成的模块：**
- ❌ **任务管理模块** (成员C - Kuang Wenqing): 未见实现
- ❌ **成就系统**: 只有AchievementManager.h
- ❌ **挑战系统**: 未见实现
- ❌ **统计分析模块**: 未实现（你的任务）
- ❌ **UI模块**: 未实现（你的任务）
- ❌ **XP/Level系统**: 未实现（你的任务）
- ❌ **主程序**: 未实现（你的任务）

---

## 💻 你需要编写的代码

### 1. StatisticsAnalyzer (统计分析引擎)

**文件路径：**
- `include/statistics/StatisticsAnalyzer.h`
- `src/statistics/StatisticsAnalyzer.cpp`

**核心功能：**
```cpp
class StatisticsAnalyzer {
private:
    DatabaseManager* dbManager;
    
public:
    StatisticsAnalyzer();
    ~StatisticsAnalyzer();
    
    // 任务统计
    int getTotalTasksCompleted();
    int getTotalTasksCreated();
    double getCompletionRate();
    
    // 时间统计
    int getTasksCompletedToday();
    int getTasksCompletedThisWeek();
    int getTasksCompletedThisMonth();
    
    // 生产力分析
    double getAverageTasksPerDay();
    vector<int> getWeeklyTrends(int weeks = 4);
    
    // 连续打卡统计
    int getCurrentStreak();
    int getLongestStreak();
    
    // 番茄钟统计
    int getTotalPomodoros();
    int getPomodorosToday();
    
    // 项目统计
    int getTotalProjects();
    double getAverageProjectProgress();
    
    // 成就和挑战统计
    int getAchievementsUnlocked();
    int getChallengesCompleted();
    
    // 生成报告
    string generateDailyReport();
    string generateWeeklyReport();
    string generateMonthlyReport();
};
```

**依赖关系：**
- 需要使用 `StatisticsDAO` 接口（已在include/database/DAO/StatisticsDAO.h定义）
- 需要访问 `DatabaseManager` 获取数据库连接
- 需要使用 `UserStats` 实体（已在common/entities.h定义）

---

### 2. XPSystem / LevelManager (经验值和等级系统)

**文件路径：**
- `include/gamification/XPSystem.h`
- `src/gamification/XPSystem.cpp`

**核心功能：**
```cpp
class XPSystem {
private:
    DatabaseManager* dbManager;
    ExperienceDAO* xpDAO;
    
    // 等级配置
    map<int, int> levelThresholds; // level -> required_xp
    
    int calculateLevel(int totalXP);
    
public:
    XPSystem();
    ~XPSystem();
    
    // 经验值管理
    bool awardXP(int amount, const string& source);
    int getCurrentXP();
    int getTotalXP();
    
    // 等级管理
    int getCurrentLevel();
    int getXPForNextLevel();
    int getXPProgressToNextLevel();
    double getLevelProgress(); // 返回0.0-1.0
    
    // 经验值来源
    int getXPForTaskCompletion(int priority); // 根据优先级给XP
    int getXPForPomodoro(); // 完成番茄钟
    int getXPForStreak(int days); // 连续打卡奖励
    int getXPForChallenge(int challengeId);
    int getXPForAchievement(int achievementId);
    
    // 等级定义
    void initializeLevelSystem();
    string getLevelTitle(int level); // 等级称号
    
    // 显示
    string displayLevelInfo();
    string displayXPBar(); // ASCII进度条
};
```

**等级系统设计建议：**
```cpp
// 等级阈值示例
Level 1: 0 XP      (新手)
Level 2: 100 XP    (初学者)
Level 3: 250 XP    (学徒)
Level 4: 500 XP    (熟练者)
Level 5: 1000 XP   (专家)
Level 10: 5000 XP  (大师)
Level 20: 20000 XP (传奇)

// XP奖励标准
完成任务：
  - 低优先级: 10 XP
  - 中优先级: 20 XP
  - 高优先级: 50 XP
完成番茄钟: 5 XP
连续打卡奖励: 天数 × 10 XP
完成挑战: challenge.reward_xp
解锁成就: achievement.reward_xp
```

**依赖关系：**
- 需要使用 `ExperienceDAO` 接口（已在include/database/DAO/ExperienceDAO.h定义）
- 需要访问 `UserStats` 更新总经验值和等级

---

### 3. HeatmapVisualizer 实现

**文件路径：**
- `src/HeatmapVisualizer/HeatmapVisualizer.cpp` (头文件已存在)

**实现要点：**
```cpp
// 已定义的接口（在include/HeatmapVisualizer/HeatmapVisualizer.h）
class HeatmapVisualizer {
private:
    map<string, int> taskData;
    string getColorBlock(int count);
    int getTaskCount(string date);
    vector<string> generateDateRange(int days);
    
public:
    void addTaskData(string date, int count);
    string generateHeatmap(int days = 90);
    string generateMonthView(string month);
    string generateWeekView(string startDate);
    int getTotalTasks();
    string getMostActiveDay();
    int getCurrentStreak();
};
```

**实现建议：**
- 使用ANSI转义码实现彩色输出
- 颜色方案：
  - `\033[90m░░\033[0m` = 0任务 (灰色)
  - `\033[32m▒▒\033[0m` = 1-3任务 (绿色)
  - `\033[33m▓▓\033[0m` = 4-6任务 (黄色)
  - `\033[31m██\033[0m` = 7+任务 (红色)
- 需要从数据库获取任务完成数据
- 日期处理使用C++标准库 `<chrono>` 和 `<ctime>`

**参考README.md中的热力图效果**

---

### 4. UIManager (用户界面管理器)

**文件路径：**
- `include/ui/UIManager.h`
- `src/ui/UIManager.cpp`

**核心功能：**
```cpp
class UIManager {
private:
    // 各个管理器的引用
    TaskManager* taskManager;
    ProjectManager* projectManager;
    StatisticsAnalyzer* statsAnalyzer;
    XPSystem* xpSystem;
    HeatmapVisualizer* heatmap;
    
    // UI辅助方法
    void clearScreen();
    void printHeader(const string& title);
    void printSeparator();
    void printMenu(const vector<string>& options);
    int getUserChoice(int maxChoice);
    string getInput(const string& prompt);
    
    // ANSI颜色
    void setColor(const string& colorCode);
    void resetColor();
    
public:
    UIManager();
    ~UIManager();
    
    // 主界面
    void showMainMenu();
    void run(); // 主循环
    
    // 任务管理界面
    void showTaskMenu();
    void createTask();
    void listTasks();
    void updateTask();
    void deleteTask();
    void completeTask();
    
    // 项目管理界面
    void showProjectMenu();
    void createProject();
    void listProjects();
    void viewProjectDetails(int projectId);
    
    // 统计分析界面
    void showStatistics();
    void showHeatmap();
    void showReports();
    
    // 游戏化界面
    void showGamificationMenu();
    void showXPAndLevel();
    void showAchievements();
    void showChallenges();
    
    // 设置界面
    void showSettings();
    
    // 工具方法
    void displayMessage(const string& msg, const string& type = "info");
    void displayError(const string& error);
    void displaySuccess(const string& msg);
    bool confirmAction(const string& prompt);
};
```

**ANSI颜色代码：**
```cpp
// 颜色定义
const string COLOR_RESET = "\033[0m";
const string COLOR_RED = "\033[31m";
const string COLOR_GREEN = "\033[32m";
const string COLOR_YELLOW = "\033[33m";
const string COLOR_BLUE = "\033[34m";
const string COLOR_MAGENTA = "\033[35m";
const string COLOR_CYAN = "\033[36m";
const string COLOR_WHITE = "\033[37m";

// 背景色
const string BG_RED = "\033[41m";
const string BG_GREEN = "\033[42m";

// 样式
const string BOLD = "\033[1m";
const string UNDERLINE = "\033[4m";
```

**界面示例：**
```
═══════════════════════════════════════════════════
    🎮 Smart Task Management System v1.0
═══════════════════════════════════════════════════
👤 等级 5 | 经验值 1250/2000 | ⭐ 成就 8/20
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

📋 主菜单:
  1. 任务管理
  2. 项目管理
  3. 统计分析
  4. 游戏化功能
  5. 设置
  0. 退出

请选择 (0-5): _
```

---

### 5. Main Program (主程序)

**文件路径：**
- `src/main.cpp`

**实现结构：**
```cpp
#include "database/DatabaseManager.h"
#include "ui/UIManager.h"
#include "statistics/StatisticsAnalyzer.h"
#include "gamification/XPSystem.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    cout << "初始化游戏化任务管理系统..." << endl;
    
    // 1. 初始化数据库
    DatabaseManager* db = DatabaseManager::getInstance();
    if (!db->initialize("task_manager.db")) {
        cerr << "❌ 数据库初始化失败！" << endl;
        return 1;
    }
    cout << "✅ 数据库初始化成功" << endl;
    
    // 2. 创建管理器实例
    StatisticsAnalyzer* stats = new StatisticsAnalyzer();
    XPSystem* xpSystem = new XPSystem();
    
    // 3. 创建UI管理器并运行
    UIManager* ui = new UIManager();
    ui->run();
    
    // 4. 清理资源
    delete ui;
    delete xpSystem;
    delete stats;
    DatabaseManager::destroyInstance();
    
    cout << "👋 感谢使用！再见！" << endl;
    return 0;
}
```

---

## 🔗 需要其他成员提供的代码

### 从成员C (Kuang Wenqing) 需要：

**TaskManager类** - 任务管理核心
```cpp
// 需要的接口
class TaskManager {
public:
    int createTask(const Task& task);
    Task* getTask(int id);
    vector<Task*> getAllTasks();
    vector<Task*> getTasksByStatus(bool completed);
    vector<Task*> getTasksByProject(int projectId);
    bool updateTask(const Task& task);
    bool deleteTask(int id);
    bool completeTask(int id);
    
    // 统计需要
    int getTaskCount();
    int getCompletedTaskCount();
};
```

**文件位置：**
- `include/task/TaskManager.h`
- `src/task/TaskManager.cpp`

---

### 从成员D (Yu Zhixuan) 需要：

**DAO接口的具体实现类**
```cpp
// 需要实现的DAO类
class TaskDAOImpl : public TaskDAO { ... };
class ProjectDAOImpl : public ProjectDAO { ... };
class StatisticsDAOImpl : public StatisticsDAO { ... };
class ExperienceDAOImpl : public ExperienceDAO { ... };
class AchievementDAOImpl : public AchievementDAO { ... };
class ChallengeDAOImpl : public ChallengeDAO { ... };
```

**文件位置：**
- `src/database/DAO/*DAOImpl.cpp`

**目前状态：** DAO接口已定义，但实现类未完成

---

### 从成员B (Zhou Tianjian) 需要：

**HeatmapVisualizer.cpp实现** - 如果你不实现的话
- 90天热力图生成逻辑
- ASCII艺术渲染

**ProjectManager数据库集成**
- 将内存存储替换为ProjectDAO调用

---

## 📝 开发步骤建议

### 第1步：完成基础框架 (1-2天)

1. 创建目录结构：
```bash
mkdir -p include/statistics src/statistics
mkdir -p include/gamification src/gamification
mkdir -p include/ui src/ui
```

2. 实现StatisticsAnalyzer基础框架
3. 实现XPSystem基础框架
4. 创建简单的main.cpp测试

### 第2步：实现核心功能 (2-3天)

5. 完成StatisticsAnalyzer核心统计方法
6. 完成XPSystem经验值和等级逻辑
7. 实现HeatmapVisualizer.cpp

### 第3步：UI开发 (2-3天)

8. 实现UIManager基础框架
9. 实现主菜单和导航
10. 实现各个子菜单

### 第4步：集成和测试 (1-2天)

11. 集成所有模块到main.cpp
12. 测试各个功能
13. 修复bug

---

## 🛠️ 编译和测试

### 编译命令示例：
```bash
# 编译单个模块测试
g++ -std=c++17 -o test_stats \
    src/statistics/StatisticsAnalyzer.cpp \
    src/database/databasemanager.cpp \
    -I./include -I./common \
    -lsqlite3

# 编译完整程序
g++ -std=c++17 -o task_manager \
    src/main.cpp \
    src/statistics/StatisticsAnalyzer.cpp \
    src/gamification/XPSystem.cpp \
    src/ui/UIManager.cpp \
    src/HeatmapVisualizer/HeatmapVisualizer.cpp \
    src/database/databasemanager.cpp \
    src/project/Project.cpp \
    src/project/ProjectManager.cpp \
    -I./include -I./common \
    -lsqlite3
```

---

## 📞 联系协作

### 需要协调的事项：

1. **与成员C协调**: 确认TaskManager接口和数据交互
2. **与成员D协调**: 确认DAO实现类的进度和接口
3. **与成员B协调**: 确认HeatmapVisualizer由谁实现
4. **与成员A协调**: 确认ReminderSystem集成方式

### 当前代码状态：
- 总代码量：约974行C++代码
- 已实现模块：约40%
- 你的任务代码量估计：约1500-2000行

---

## 🎯 优先级建议

**高优先级（必须完成）：**
1. ✅ StatisticsAnalyzer基础实现
2. ✅ XPSystem基础实现
3. ✅ Main.cpp程序入口
4. ✅ UIManager基础框架

**中优先级（重要功能）：**
5. ⚡ HeatmapVisualizer实现
6. ⚡ UI完整菜单系统
7. ⚡ 统计报告生成

**低优先级（增强功能）：**
8. 🌟 高级统计分析
9. 🌟 UI美化和动画
10. 🌟 额外的游戏化特性

---

## 📚 参考资源

- **实体定义**: `common/entities.h` 和 `common/ENTITY_DEFINITION.md`
- **数据库接口**: `include/database/DatabaseManager.h`
- **DAO接口**: `include/database/DAO/*.h`
- **项目管理参考**: `src/project/ProjectManager.cpp`
- **README**: 查看热力图效果示例

---

**祝开发顺利！如有问题随时在团队中沟通。** 🚀
