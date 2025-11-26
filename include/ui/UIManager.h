#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <vector>
#include <iostream>

// 前向声明，避免循环依赖
class StatisticsAnalyzer;
class XPSystem;
class HeatmapVisualizer;
class ProjectManager;
class TaskManager; 

class UIManager {
private:
    // === 核心模块指针 ===
    StatisticsAnalyzer* statsAnalyzer;
    XPSystem* xpSystem;
    HeatmapVisualizer* heatmap;
    ProjectManager* projectManager;
    TaskManager* taskManager; // ⭐ 新增：任务管理器

    bool running;

    // === 颜色常量 (保留原定义) ===
    static const std::string COLOR_RESET;
    static const std::string COLOR_RED;
    static const std::string COLOR_GREEN;
    static const std::string COLOR_YELLOW;
    static const std::string COLOR_BLUE;
    static const std::string COLOR_MAGENTA;
    static const std::string COLOR_CYAN;
    static const std::string COLOR_WHITE;
    static const std::string BOLD;
    static const std::string UNDERLINE;

    // === 内部辅助方法 ===
    void clearScreen();
    void printHeader(const std::string& title);
    void printSeparator(char symbol, int length);
    void printMenu(const std::vector<std::string>& options);
    int getUserChoice(int maxChoice);
    std::string getInput(const std::string& prompt);
    int getIntInput(const std::string& prompt);
    void pause();
    bool confirmAction(const std::string& prompt);

    // === ⭐ 游戏化视觉特效 (UI增强) ===
    void displayHUD(); // 替代原有的 displayUserStatusBar
    void printProgressBar(int current, int total, int width = 30, std::string color = COLOR_CYAN);
    void printEncouragement();
    void showTaskCompleteCelebration(int xpGained);

public:
    UIManager();
    ~UIManager();

    void run();

    // === 菜单逻辑 ===
    void showMainMenu();
    void exitProgram();
    
    // 子菜单
    void showTaskMenu();
    void showProjectMenu();
    void showStatisticsMenu();
    void showGamificationMenu();
    void showSettingsMenu();

    // === 消息显示 ===
    void displayMessage(const std::string& msg, const std::string& type = "info");
    void displayError(const std::string& error);
    void displaySuccess(const std::string& msg);
    void displayWarning(const std::string& warning);
    void displayInfo(const std::string& info);

    // === 业务逻辑方法 ===
    
    // 任务功能 (已增强逻辑)
    void createTask();
    void listTasks();
    void updateTask();
    void deleteTask();
    void completeTask();

    // 项目功能 (完整保留)
    void createProject();
    void listProjects();
    void viewProjectDetails();
    void updateProject();
    void deleteProject();

    // 统计功能 (完整保留)
    void showStatisticsSummary();
    void showDailyReport();
    void showWeeklyReport();
    void showMonthlyReport();
    void showHeatmap();

    // 游戏化功能 (完整保留)
    void showXPAndLevel();
    void showAchievements();
    void showChallenges();

    // 设置功能 (完整保留)
    void viewSettings();
    void updateSettings();
};

#endif // UI_MANAGER_H
