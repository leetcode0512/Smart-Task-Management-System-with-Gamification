#include "ui/UIManager.h"
#include "database/DatabaseManager.h"
#include "statistics/StatisticsAnalyzer.h"
#include "gamification/XPSystem.h"
#include "HeatmapVisualizer/HeatmapVisualizer.h"
#include "project/ProjectManager.h"
#include "task/TaskManager.h" // ⭐ 引入任务管理器

#include <iostream>
#include <iomanip>
#include <limits>
#include <thread> // ⭐ 动画延迟
#include <chrono> 
#include <vector>
#include <random> // ⭐ 随机鼓励语

using namespace std;

// ANSI颜色定义
const string UIManager::COLOR_RESET = "\033[0m";
const string UIManager::COLOR_RED = "\033[31m";
const string UIManager::COLOR_GREEN = "\033[32m";
const string UIManager::COLOR_YELLOW = "\033[33m";
const string UIManager::COLOR_BLUE = "\033[34m";
const string UIManager::COLOR_MAGENTA = "\033[35m";
const string UIManager::COLOR_CYAN = "\033[36m";
const string UIManager::COLOR_WHITE = "\033[37m";
const string UIManager::BOLD = "\033[1m";
const string UIManager::UNDERLINE = "\033[4m";

UIManager::UIManager() {
    running = true;
    
    // 创建各模块实例
    statsAnalyzer = new StatisticsAnalyzer();
    xpSystem = new XPSystem();
    heatmap = new HeatmapVisualizer();
    projectManager = new ProjectManager();
    taskManager = new TaskManager(); // ⭐ 初始化任务管理器
    
    cout << "✅ UI管理器初始化成功" << endl;
}

UIManager::~UIManager() {
    delete statsAnalyzer;
    delete xpSystem;
    delete heatmap;
    delete projectManager;
    delete taskManager; // ⭐ 清理内存
}

// === UI辅助方法 ===

void UIManager::clearScreen() {
    cout << "\033[2J\033[H";
}

void UIManager::printHeader(const string& title) {
    cout << "\n";
    cout << BOLD << COLOR_CYAN;
    printSeparator("=", 55);
    cout << "    " << title << "\n";
    printSeparator("=", 55);
    cout << COLOR_RESET << "\n";
}

void UIManager::printSeparator(const string& symbol, int length) {
    for (int i = 0; i < length; i++) {
        cout << symbol;
    }
    cout << "\n";
}

void UIManager::printMenu(const vector<string>& options) {
    cout << "\n";
    for (size_t i = 0; i < options.size(); i++) {
        cout << "  " << COLOR_YELLOW << i + 1 << COLOR_RESET 
             << ". " << options[i] << "\n";
    }
    cout << "  " << COLOR_RED << "0" << COLOR_RESET << ". 返回/退出\n";
    cout << "\n";
}

int UIManager::getUserChoice(int maxChoice) {
    int choice;
    cout << COLOR_GREEN << "请选择 (0-" << maxChoice << "): " << COLOR_RESET;
    
    while (!(cin >> choice) || choice < 0 || choice > maxChoice) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << COLOR_RED << "无效输入！请重新选择 (0-" << maxChoice << "): " << COLOR_RESET;
    }
    
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

string UIManager::getInput(const string& prompt) {
    string input;
    cout << COLOR_GREEN << prompt << COLOR_RESET;
    getline(cin, input);
    return input;
}

int UIManager::getIntInput(const string& prompt) {
    int value;
    cout << COLOR_GREEN << prompt << COLOR_RESET;
    
    while (!(cin >> value)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << COLOR_RED << "无效输入！请输入数字: " << COLOR_RESET;
    }
    
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return value;
}

void UIManager::pause() {
    cout << "\n" << COLOR_YELLOW << "按Enter键继续..." << COLOR_RESET;
    cin.get();
}

bool UIManager::confirmAction(const string& prompt) {
    cout << COLOR_YELLOW << prompt << " (y/n): " << COLOR_RESET;
    string response;
    getline(cin, response);
    return (response == "y" || response == "Y" || response == "yes" || response == "YES");
}

// ==========================================
// ⭐ 游戏化 UI 增强实现 (New Features)
// ==========================================

void UIManager::printProgressBar(int current, int total, int width, string color) {
    float percentage = (total == 0) ? 0 : (float)current / total;
    if (percentage > 1.0f) percentage = 1.0f;
    int filled = static_cast<int>(width * percentage);
    
    cout << " [";
    cout << color;
    for (int i = 0; i < width; ++i) {
        if (i < filled) cout << "█";
        else cout << "░";
    }
    cout << COLOR_RESET << "] " << int(percentage * 100) << "%";
}

void UIManager::printEncouragement() {
    static const vector<string> quotes = {
        "Keep the streak alive! 🔥", 
        "Small steps, big progress.", 
        "You are unstoppable today!", 
        "Focus is the key to victory."
    };
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(0, quotes.size() - 1);

    cout << "\n" << BOLD << COLOR_YELLOW << " >> " << quotes[dis(gen)] << COLOR_RESET << "\n";
}

// HUD display constants
namespace {
    const int HUD_SPACING_WIDTH = 10;
}

// Replace displayUserStatusBar
void UIManager::displayHUD() {
    int level = xpSystem->getCurrentLevel();
    int currentXP = xpSystem->getCurrentXP();
    int nextLevelXP = xpSystem->getXPForNextLevel(); 
    string title = xpSystem->getCurrentLevelTitle();
    int achievements = statsAnalyzer->getAchievementsUnlocked();
    
    cout << BOLD << COLOR_CYAN;
    printSeparator("-", 60);
    cout << COLOR_RESET;
    
    // Line 1: Level and achievements
    cout << " Lv." << level << COLOR_RESET 
         << " [" << COLOR_MAGENTA << title << COLOR_RESET << "] "
         << string(HUD_SPACING_WIDTH, ' ')
         << "Achievements: " << COLOR_YELLOW << achievements << COLOR_RESET << "\n";
    
    // Line 2: XP progress bar
    cout << " XP: ";
    printProgressBar(currentXP, nextLevelXP, 35, COLOR_GREEN);
    cout << " (" << currentXP << "/" << nextLevelXP << ")\n";
    
    cout << BOLD << COLOR_CYAN;
    printSeparator("-", 60);
    cout << COLOR_RESET;
    
    printEncouragement();
}

// 任务完成特效
void UIManager::showTaskCompleteCelebration(int xpGained) {
    cout << "\n";
    for(int i=0; i<3; ++i) {
        cout << COLOR_YELLOW << "  ★  Reward Unlocking...  ★  " << COLOR_RESET << "\r";
        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(150));
        cout << "                           \r"; 
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    cout << "\n  " << COLOR_GREEN << BOLD << "✅ TASK COMPLETED! Awesome!" << COLOR_RESET << "\n";
    cout << "  " << COLOR_YELLOW << "+" << xpGained << " XP" << COLOR_RESET << "\n\n";
    
    this_thread::sleep_for(chrono::milliseconds(800)); 
}

// === 主界面 ===

void UIManager::showMainMenu() {
    clearScreen();
    
    cout << BOLD << COLOR_BLUE;
    cout << R"(
   ╔═══════════════════════════════════════════════════╗
   ║    🎮 Smart Task Management System v1.0 🎮        ║
   ╚═══════════════════════════════════════════════════╝
)" << COLOR_RESET;
    
    displayHUD(); // ⭐ 使用新的 HUD
    
    vector<string> options = {
        "📋 任务管理",
        "📁 项目管理",
        "📊 统计分析",
        "🎮 游戏化功能",
        "⚙️  设置"
    };
    
    printMenu(options);
}

void UIManager::run() {
    cout << COLOR_GREEN << "\n🎉 欢迎使用智能任务管理系统！\n" << COLOR_RESET;
    pause();
    
    while (running) {
        showMainMenu();
        int choice = getUserChoice(5);
        
        switch (choice) {
            case 1: showTaskMenu(); break;
            case 2: showProjectMenu(); break;
            case 3: showStatisticsMenu(); break;
            case 4: showGamificationMenu(); break;
            case 5: showSettingsMenu(); break;
            case 0: exitProgram(); break;
        }
    }
}

void UIManager::exitProgram() {
    if (confirmAction("确定要退出吗？")) {
        clearScreen();
        cout << COLOR_GREEN << "\n👋 感谢使用！再见！\n\n" << COLOR_RESET;
        running = false;
    }
}

// === 任务管理界面 (⭐ 已接入逻辑) ===

void UIManager::showTaskMenu() {
    clearScreen();
    printHeader("📋 任务管理");
    
    vector<string> options = {
        "创建新任务",
        "查看所有任务",
        "更新任务",
        "删除任务",
        "完成任务 (获取XP!)" // 文案优化
    };
    
    printMenu(options);
    int choice = getUserChoice(5);
    
    switch (choice) {
        case 1: createTask(); break;
        case 2: listTasks(); break;
        case 3: updateTask(); break;
        case 4: deleteTask(); break;
        case 5: completeTask(); break; // 调用增强版逻辑
        case 0: return;
    }
}

void UIManager::createTask() {
    clearScreen();
    printHeader("✨ 创建新任务（高级模式）");

    string name = getInput("任务名称: ");
    string desc = getInput("任务描述: ");
    int priority = getIntInput("优先级 (0=低,1=中,2=高): ");
    string due = getInput("截止日期 (YYYY-MM-DD，可空): ");
    string tags = getInput("标签（用逗号分隔）: ");
    int estimated = getIntInput("预计番茄数: ");
    string reminder = getInput("提醒时间 (YYYY-MM-DD HH:MM，可空): ");

    Task t(name, desc);
    t.setPriority(priority);
    t.setDueDate(due);
    t.setTags(tags);
    t.setEstimatedPomodoros(estimated);
    t.setReminderTime(reminder);

    int id = taskManager->createTask(t);

    if (id > 0)
        displaySuccess("任务创建成功！ID = " + to_string(id));
    else
        displayError("创建失败。");

    pause();
}


void UIManager::listTasks() {
    clearScreen();
    printHeader("📋 任务列表");
    
    // ⭐ 使用真实 Logic
    auto tasks = taskManager->getAllTasks();
    if (tasks.empty()) {
        displayInfo("暂无任务。赶快创建一个吧！");
    } else {
        cout << "\n";
        for (const auto& t : tasks) {
            if (t.isCompleted()) {
                cout << COLOR_GREEN << " [✔] " << t.getId() << ". " << t.getName() << COLOR_RESET << "\n";
            } else {
                cout << COLOR_RED << " [ ] " << COLOR_RESET << t.getId() << ". " << t.getName() << "\n";
            }
        }
    }
    pause();
}

void UIManager::updateTask() {
    clearScreen();
    printHeader("✏️ 更新任务状态");

    int id = getIntInput("请输入要更新的任务ID: ");

    auto opt = taskManager->getTask(id);
    if (!opt.has_value()) {
        displayError("任务不存在！");
        pause();
        return;
    }

    Task task = opt.value();

    cout << "当前任务：" << task.getName() << "\n";
    cout << "当前状态：" << (task.isCompleted() ? "已完成" : "未完成") << "\n\n";

    cout << "选择新的状态：\n";
    cout << "1. 标记为未完成\n";
    cout << "2. 标记为已完成\n";

    int choice = getIntInput("\n请输入选项 (1-2): ");

    if (choice == 1) {
        task.setCompleted(false);
    } else if (choice == 2) {
        task.setCompleted(true);
    } else {
        displayError("无效输入！");
        pause();
        return;
    }

    if (taskManager->updateTask(task))
        displaySuccess("任务状态更新成功！");
    else
        displayError("更新失败！");

    pause();
}



void UIManager::deleteTask() {
    clearScreen();
    printHeader("🗑️  删除任务");
    int id = getIntInput("请输入要删除的任务ID: ");
    
    // ⭐ 使用真实 Logic
    if (taskManager->deleteTask(id)) {
        displaySuccess("任务已删除。");
    } else {
        displayError("删除失败，ID可能不存在。");
    }
    pause();
}

void UIManager::completeTask() {
    clearScreen();
    printHeader("✅ 完成任务");
    
    auto tasks = taskManager->getAllTasks();
    bool hasPending = false;
    for(const auto& t : tasks) {
        if (!t.isCompleted()) {
            cout << COLOR_CYAN << "ID: " << t.getId() << " | " << t.getName() << COLOR_RESET << "\n";
            hasPending = true;
        }
    }
    
    if (!hasPending) {
        displayInfo("没有待完成的任务！");
        pause();
        return;
    }

    int id = getIntInput("\n请输入完成的任务ID: ");
    
    // ⭐ 调用 Logic 并展示动画
    if (taskManager->completeTask(id)) {
        int xpReward = xpSystem->getXPForTaskCompletion(1); 
        xpSystem->awardXP(xpReward, "任务完成");
        showTaskCompleteCelebration(xpReward);
    } else {
        displayError("操作失败！");
        pause();
    }
}

// === 项目管理界面 (完整保留) ===

void UIManager::showProjectMenu() {
    clearScreen();
    printHeader("📁 项目管理");
    
    vector<string> options = {
        "创建新项目",
        "查看所有项目",
        "查看项目详情",
        "更新项目",
        "删除项目"
    };
    
    printMenu(options);
    int choice = getUserChoice(5);
    
    switch (choice) {
        case 1: createProject(); break;
        case 2: listProjects(); break;
        case 3: viewProjectDetails(); break;
        case 4: updateProject(); break;
        case 5: deleteProject(); break;
        case 0: return;
    }
}

void UIManager::createProject() {
    clearScreen();
    printHeader("✨ 创建新项目");
    
    string name = getInput("项目名称: ");
    if (name.empty()) {
        displayError("项目名称不能为空！");
        pause();
        return;
    }
    
    string desc = getInput("项目描述: ");
    string color = getInput("颜色标签 (如 #4CAF50): ");
    
    Project project(name, desc, color);
    int id = projectManager->createProject(project);
    
    displaySuccess("项目创建成功！ID: " + to_string(id));
    
    pause();
}

void UIManager::listProjects() {
    clearScreen();
    printHeader("Project List");
    
    vector<Project*> projects = projectManager->getAllProjects();
    
    if (projects.empty()) {
        displayInfo("No projects yet");
    } else {
        cout << "\n";
        printSeparator("-", 55);
        
        for (Project* p : projects) {
            cout << COLOR_BLUE << "ID: " << p->getId() << COLOR_RESET << " | "
                 << BOLD << p->getName() << COLOR_RESET << "\n";
            cout << "  Description: " << p->getDescription() << "\n";
            cout << "  Progress: " << COLOR_GREEN 
                 << fixed << setprecision(1) << (p->getProgress() * 100) << "%" 
                 << COLOR_RESET << " ("
                 << p->getCompletedTasks() << "/" << p->getTotalTasks() << ")\n";
            printSeparator("-", 55);
        }
    }
    
    pause();
}

void UIManager::viewProjectDetails() {
    clearScreen();
    printHeader("📊 项目详情");
    
    int id = getIntInput("请输入项目ID: ");
    Project* p = projectManager->getProject(id);
    
    if (p == nullptr) {
        displayError("项目不存在！");
    } else {
        cout << "\n";
        cout << BOLD << "项目名称: " << COLOR_RESET << p->getName() << "\n";
        cout << "描述: " << p->getDescription() << "\n";
        cout << "颜色标签: " << p->getColorLabel() << "\n";
        cout << "进度: " << COLOR_GREEN << fixed << setprecision(1) 
             << (p->getProgress() * 100) << "%" << COLOR_RESET << "\n";
        cout << "任务统计: " << p->getCompletedTasks() << "/" << p->getTotalTasks() << "\n";
        cout << "目标日期: " << p->getTargetDate() << "\n";
        cout << "创建日期: " << p->getCreatedDate() << "\n";
        cout << "状态: " << (p->isArchived() ? "已归档" : "活跃") << "\n";
    }
    
    pause();
}

void UIManager::updateProject() {
    clearScreen();
    printHeader("✏️  更新项目");
    displayInfo("功能开发中...");
    pause();
}

void UIManager::deleteProject() {
    clearScreen();
    printHeader("🗑️  删除项目");
    
    int id = getIntInput("请输入要删除的项目ID: ");
    
    if (confirmAction("确定要删除这个项目吗？")) {
        if (projectManager->deleteProject(id)) {
            displaySuccess("项目删除成功！");
        } else {
            displayError("删除失败！");
        }
    }
    
    pause();
}

// === 统计分析界面 (完整保留) ===

void UIManager::showStatisticsMenu() {
    clearScreen();
    printHeader("📊 统计分析");
    
    vector<string> options = {
        "统计数据总览",
        "每日报告",
        "每周报告",
        "每月报告",
        "任务完成热力图"
    };
    
    printMenu(options);
    int choice = getUserChoice(5);
    
    switch (choice) {
        case 1: showStatisticsSummary(); break;
        case 2: showDailyReport(); break;
        case 3: showWeeklyReport(); break;
        case 4: showMonthlyReport(); break;
        case 5: showHeatmap(); break;
        case 0: return;
    }
}

void UIManager::showStatisticsSummary() {
    clearScreen();
    printHeader("📈 统计数据总览");
    cout << statsAnalyzer->generateSummary();
    pause();
}

void UIManager::showDailyReport() {
    clearScreen();
    printHeader("📅 每日报告");
    cout << statsAnalyzer->generateDailyReport();
    pause();
}

void UIManager::showWeeklyReport() {
    clearScreen();
    printHeader("📈 每周报告");
    cout << statsAnalyzer->generateWeeklyReport();
    pause();
}

void UIManager::showMonthlyReport() {
    clearScreen();
    printHeader("📊 每月报告");
    cout << statsAnalyzer->generateMonthlyReport();
    pause();
}

void UIManager::showHeatmap() {
    clearScreen();
    printHeader("🔥 任务完成热力图");
    // 显示热力图（数据从数据库中获取）
    cout << heatmap->generateHeatmap(90);
    pause();
}

// === 游戏化界面 (完整保留) ===

void UIManager::showGamificationMenu() {
    clearScreen();
    printHeader("🎮 游戏化功能");
    
    vector<string> options = {
        "经验值和等级",
        "成就系统",
        "挑战系统"
    };
    
    printMenu(options);
    int choice = getUserChoice(3);
    
    switch (choice) {
        case 1: showXPAndLevel(); break;
        case 2: showAchievements(); break;
        case 3: showChallenges(); break;
        case 0: return;
    }
}

void UIManager::showXPAndLevel() {
    clearScreen();
    printHeader("⭐ 经验值和等级");
    cout << xpSystem->displayLevelInfo();
    pause();
}

void UIManager::showAchievements() {
    clearScreen();
    printHeader("🏆 成就系统");
    int unlocked = statsAnalyzer->getAchievementsUnlocked();
    cout << "\n已解锁成就: " << COLOR_GREEN << unlocked << COLOR_RESET << " 个\n\n";
    displayInfo("成就系统详细功能开发中...");
    displayInfo("需要成就模块完全实现后集成");
    pause();
}

void UIManager::showChallenges() {
    clearScreen();
    printHeader("🎯 挑战系统");
    int completed = statsAnalyzer->getChallengesCompleted();
    cout << "\n已完成挑战: " << COLOR_GREEN << completed << COLOR_RESET << " 个\n\n";
    displayInfo("挑战系统详细功能开发中...");
    displayInfo("需要挑战模块完全实现后集成");
    pause();
}

// === 设置界面 (完整保留) ===

void UIManager::showSettingsMenu() {
    clearScreen();
    printHeader("⚙️  系统设置");
    
    vector<string> options = {
        "查看设置",
        "修改设置"
    };
    
    printMenu(options);
    int choice = getUserChoice(2);
    
    switch (choice) {
        case 1: viewSettings(); break;
        case 2: updateSettings(); break;
        case 0: return;
    }
}

void UIManager::viewSettings() {
    clearScreen();
    printHeader("📋 当前设置");
    displayInfo("设置模块开发中...");
    pause();
}

void UIManager::updateSettings() {
    clearScreen();
    printHeader("✏️  修改设置");
    displayInfo("设置模块开发中...");
    pause();
}

// === 消息显示 (完整保留) ===

void UIManager::displayMessage(const string& msg, const string& type) {
    if (type == "success") {
        displaySuccess(msg);
    } else if (type == "error") {
        displayError(msg);
    } else if (type == "warning") {
        displayWarning(msg);
    } else {
        displayInfo(msg);
    }
}

void UIManager::displayError(const string& error) {
    cout << COLOR_RED << "❌ " << error << COLOR_RESET << "\n";
}

void UIManager::displaySuccess(const string& msg) {
    cout << COLOR_GREEN << "✅ " << msg << COLOR_RESET << "\n";
}

void UIManager::displayWarning(const string& warning) {
    cout << COLOR_YELLOW << "⚠️  " << warning << COLOR_RESET << "\n";
}

void UIManager::displayInfo(const string& info) {
    cout << COLOR_CYAN << "ℹ️  " << info << COLOR_RESET << "\n";
}
