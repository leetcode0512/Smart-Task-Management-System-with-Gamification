#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <vector>
#include <iostream>

// 前向声明
class StatisticsAnalyzer;
class XPSystem;
class HeatmapVisualizer;
class ProjectManager;
class TaskManager; // 假设你也需要集成任务管理

class UIManager {
private:
    // === 核心模块指针 (保留原项目结构) ===
    StatisticsAnalyzer* statsAnalyzer;
    XPSystem* xpSystem;
    HeatmapVisualizer* heatmap;
    ProjectManager* projectManager;
    TaskManager* taskManager; // 新增

    bool running;

    // === 颜色常量 (保留) ===
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

    // === ⭐ 新增：游戏化视觉特效 ===
    void printProgressBar(int current, int total, int width = 30, std::string color = COLOR_CYAN);
    void showLevelUpAnimation(int newLevel); // 升级全屏特效
    void printEncouragement();               // 随机鼓励语
    void displayHUD();                       // 抬头显示 (XP, Level, Streak)

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
    void showSettingsMenu(); // 占位

    // 消息显示
    void displayMessage(const std::string& msg, const std::string& type = "info");
    void displayError(const std::string& error);
    void displaySuccess(const std::string& msg);
    void displayWarning(const std::string& warning);
    void displayInfo(const std::string& info);

    // 功能实现 (部分)
    void listTasks();
    void completeTask(); 
};

#endif // UI_MANAGER_H
