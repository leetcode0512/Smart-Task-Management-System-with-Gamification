#include "ui/UIManager.h"
#include <iostream>
#include <iomanip>
#include <limits>

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
    
    cout << "✅ UI管理器初始化成功" << endl;
}

UIManager::~UIManager() {
    delete statsAnalyzer;
    delete xpSystem;
    delete heatmap;
    delete projectManager;
}

// === UI辅助方法 ===

void UIManager::clearScreen() {
    // ANSI转义序列清屏
    cout << "\033[2J\033[H";
}

void UIManager::printHeader(const string& title) {
    cout << "\n";
    cout << BOLD << COLOR_CYAN;
    printSeparator('═', 55);
    cout << "    " << title << "\n";
    printSeparator('═', 55);
    cout << COLOR_RESET << "\n";
}

void UIManager::printSeparator(char symbol, int length) {
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

void UIManager::displayUserStatusBar() {
    int level = xpSystem->getCurrentLevel();
    int totalXP = xpSystem->getTotalXP();
    int nextLevelXP = xpSystem->getXPForNextLevel();
    string title = xpSystem->getCurrentLevelTitle();
    int achievements = statsAnalyzer->getAchievementsUnlocked();
    
    cout << BOLD << COLOR_CYAN;
    printSeparator('━', 55);
    cout << COLOR_RESET;
    
    cout << COLOR_MAGENTA << "👤 " << COLOR_RESET 
         << "等级 " << BOLD << level << COLOR_RESET 
         << " (" << title << ") | "
         << COLOR_YELLOW << "XP: " << COLOR_RESET << totalXP << "/" << nextLevelXP << " | "
         << COLOR_GREEN << "⭐ 成就: " << COLOR_RESET << achievements << "\n";
    
    cout << BOLD << COLOR_CYAN;
    printSeparator('━', 55);
    cout << COLOR_RESET << "\n";
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
    
    displayUserStatusBar();
    
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
            case 1:
                showTaskMenu();
                break;
            case 2:
                showProjectMenu();
                break;
            case 3:
                showStatisticsMenu();
                break;
            case 4:
                showGamificationMenu();
                break;
            case 5:
                showSettingsMenu();
                break;
            case 0:
                exitProgram();
                break;
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

// === 任务管理界面 ===

void UIManager::showTaskMenu() {
    clearScreen();
    printHeader("📋 任务管理");
    
    vector<string> options = {
        "创建新任务",
        "查看所有任务",
        "更新任务",
        "删除任务",
        "完成任务"
    };
    
    printMenu(options);
    int choice = getUserChoice(5);
    
    switch (choice) {
        case 1: createTask(); break;
        case 2: listTasks(); break;
        case 3: updateTask(); break;
        case 4: deleteTask(); break;
        case 5: completeTask(); break;
        case 0: return;
    }
}

void UIManager::createTask() {
    clearScreen();
    printHeader("✨ 创建新任务");
    
    displayInfo("注意：任务管理模块尚未完全实现");
    displayWarning("需要等待成员C完成TaskManager模块");
    
    pause();
}

void UIManager::listTasks() {
    clearScreen();
    printHeader("📋 任务列表");
    
    displayInfo("注意：任务管理模块尚未完全实现");
    displayWarning("需要等待成员C完成TaskManager模块");
    
    pause();
}

void UIManager::updateTask() {
    clearScreen();
    printHeader("✏️  更新任务");
    
    displayInfo("注意：任务管理模块尚未完全实现");
    
    pause();
}

void UIManager::deleteTask() {
    clearScreen();
    printHeader("🗑️  删除任务");
    
    displayInfo("注意：任务管理模块尚未完全实现");
    
    pause();
}

void UIManager::completeTask() {
    clearScreen();
    printHeader("✅ 完成任务");
    
    displayInfo("注意：任务管理模块尚未完全实现");
    displayInfo("完成任务后会自动获得经验值奖励");
    
    pause();
}

// === 项目管理界面 ===

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
    printHeader("📁 项目列表");
    
    vector<Project*> projects = projectManager->getAllProjects();
    
    if (projects.empty()) {
        displayInfo("暂无项目");
    } else {
        cout << "\n";
        printSeparator('─', 55);
        
        for (Project* p : projects) {
            cout << COLOR_BLUE << "ID: " << p->getId() << COLOR_RESET << " | "
                 << BOLD << p->getName() << COLOR_RESET << "\n";
            cout << "  描述: " << p->getDescription() << "\n";
            cout << "  进度: " << COLOR_GREEN 
                 << fixed << setprecision(1) << (p->getProgress() * 100) << "%" 
                 << COLOR_RESET << " ("
                 << p->getCompletedTasks() << "/" << p->getTotalTasks() << ")\n";
            printSeparator('─', 55);
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

// === 统计分析界面 ===

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
    
    // 获取热力图数据
    map<string, int> data = statsAnalyzer->getTaskCompletionData(90);
    
    // 添加数据到热力图
    for (const auto& pair : data) {
        heatmap->addTaskData(pair.first, pair.second);
    }
    
    // 显示热力图
    cout << heatmap->generateHeatmap(90);
    
    pause();
}

// === 游戏化界面 ===

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

// === 设置界面 ===

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

// === 消息显示 ===

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
