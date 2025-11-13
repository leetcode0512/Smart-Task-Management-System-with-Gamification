#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <vector>
#include "../statistics/StatisticsAnalyzer.h"
#include "../gamification/XPSystem.h"
#include "../HeatmapVisualizer/HeatmapVisualizer.h"
#include "../project/ProjectManager.h"

using namespace std;

/**
 * @brief 用户界面管理器 - 提供完整的控制台UI交互
 * 
 * 负责人: Mao Jingqi (成员E)
 * 功能: 菜单系统、用户交互、数据展示
 */
class UIManager {
private:
    // 各模块引用
    StatisticsAnalyzer* statsAnalyzer;
    XPSystem* xpSystem;
    HeatmapVisualizer* heatmap;
    ProjectManager* projectManager;
    
    // UI状态
    bool running;
    
    // === ANSI颜色定义 ===
    static const string COLOR_RESET;
    static const string COLOR_RED;
    static const string COLOR_GREEN;
    static const string COLOR_YELLOW;
    static const string COLOR_BLUE;
    static const string COLOR_MAGENTA;
    static const string COLOR_CYAN;
    static const string COLOR_WHITE;
    static const string BOLD;
    static const string UNDERLINE;
    
    // === UI辅助方法 ===
    
    /**
     * @brief 清屏
     */
    void clearScreen();
    
    /**
     * @brief 打印标题头
     */
    void printHeader(const string& title);
    
    /**
     * @brief 打印分隔线
     */
    void printSeparator(char symbol = '=', int length = 50);
    
    /**
     * @brief 打印菜单选项
     */
    void printMenu(const vector<string>& options);
    
    /**
     * @brief 获取用户选择
     * @param maxChoice 最大选项数
     * @return 用户选择的编号
     */
    int getUserChoice(int maxChoice);
    
    /**
     * @brief 获取用户输入
     */
    string getInput(const string& prompt);
    
    /**
     * @brief 获取整数输入
     */
    int getIntInput(const string& prompt);
    
    /**
     * @brief 暂停并等待用户按键
     */
    void pause();
    
    /**
     * @brief 确认操作
     */
    bool confirmAction(const string& prompt);
    
    // === 界面显示方法 ===
    
    /**
     * @brief 显示用户状态栏
     */
    void displayUserStatusBar();
    
public:
    UIManager();
    ~UIManager();
    
    // === 主界面 ===
    
    /**
     * @brief 显示主菜单
     */
    void showMainMenu();
    
    /**
     * @brief 运行主循环
     */
    void run();
    
    /**
     * @brief 退出程序
     */
    void exitProgram();
    
    // === 任务管理界面 ===
    
    /**
     * @brief 显示任务管理菜单
     */
    void showTaskMenu();
    
    void createTask();
    void listTasks();
    void updateTask();
    void deleteTask();
    void completeTask();
    
    // === 项目管理界面 ===
    
    /**
     * @brief 显示项目管理菜单
     */
    void showProjectMenu();
    
    void createProject();
    void listProjects();
    void viewProjectDetails();
    void updateProject();
    void deleteProject();
    
    // === 统计分析界面 ===
    
    /**
     * @brief 显示统计分析菜单
     */
    void showStatisticsMenu();
    
    void showStatisticsSummary();
    void showDailyReport();
    void showWeeklyReport();
    void showMonthlyReport();
    void showHeatmap();
    
    // === 游戏化界面 ===
    
    /**
     * @brief 显示游戏化菜单
     */
    void showGamificationMenu();
    
    void showXPAndLevel();
    void showAchievements();
    void showChallenges();
    
    // === 设置界面 ===
    
    /**
     * @brief 显示设置菜单
     */
    void showSettingsMenu();
    
    void viewSettings();
    void updateSettings();
    
    // === 消息显示 ===
    
    /**
     * @brief 显示消息
     * @param msg 消息内容
     * @param type 消息类型 (info/success/warning/error)
     */
    void displayMessage(const string& msg, const string& type = "info");
    
    void displayError(const string& error);
    void displaySuccess(const string& msg);
    void displayWarning(const string& warning);
    void displayInfo(const string& info);
};

#endif // UI_MANAGER_H
