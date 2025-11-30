#include <iostream>
#include <string>
#include <thread> // 动画支持
#include <chrono> // 时间控制
#include <vector>
#include <cstdlib> // 用于 rand, srand
#include <ctime>   // 用于 time

// 引入 Windows 特定头文件以解决编码和颜色问题
#ifdef _WIN32
#include <windows.h>
#endif

#include "../include/database/DatabaseManager.h"
#include "../include/ui/UIManager.h"
#include "../include/statistics/StatisticsAnalyzer.h"
#include "../include/gamification/XPSystem.h"

using namespace std;

// === 核心修复：控制台环境设置 ===
void setupConsole() {
#ifdef _WIN32
    // 1. 设置控制台输入输出编码为 UTF-8 (代码页 65001)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 2. 启用虚拟终端处理 (支持 \033 颜色代码)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif
    // Linux/macOS 默认通常支持 UTF-8 和 ANSI，无需额外设置
}

// === 视觉辅助工具 (本地静态函数) ===

void sleepMs(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

// 打字机效果：逐字输出
void typewriterPrint(const string& text, int speedMs = 20, string color = "\033[1;37m") {
    cout << color;
    for (char c : text) {
        cout << c << flush;
        sleepMs(speedMs);
    }
    cout << "\033[0m" << endl; // 重置颜色
}

// 模拟加载进度条
void simulateLoading(const string& taskName) {
    cout << "  " << taskName << " [";
    for (int i = 0; i < 20; ++i) {
        cout << "\033[1;36m#\033[0m" << flush; // 青色进度块
        sleepMs(30 + (rand() % 50)); // 随机延迟，模拟真实感
    }
    cout << "] \033[1;32mOK\033[0m" << endl;
    sleepMs(200);
}

/**
 * @brief 显示欢迎横幅 (增强版)
 */
void displayWelcomeBanner() {
    sleepMs(500);
    // 清屏指令：兼容不同终端
    cout << "\033[2J\033[H"; 

    sleepMs(800);
    
    // 欢迎语
    typewriterPrint("    >> Welcome back, Hero.", 40, "\033[1;33m"); // 黄色
    typewriterPrint("    >> Preparing your workspace...", 30, "\033[0m");
    cout << endl;
}

/**
 * @brief 初始化系统 (游戏化文案)
 * @return 是否初始化成功
 */
bool initializeSystem() {   
    // 1. 初始化数据库 -> "Opening Quest Log"
    simulateLoading("Opening Quest Log (DB)   ");
    
    DatabaseManager& db = DatabaseManager::getInstance();
    
    if (!db.initialize("task_manager.db")) {
        cerr << "\033[1;31m[CRITICAL ERROR] The Quest Log is corrupted!\033[0m" << endl;
        cerr << "Error: " << db.getLastErrorMessage() << endl;
        return false;
    }
    
    // 2. 验证数据库表 -> "Verifying World State"
    simulateLoading("Verifying World State    ");
    
    bool allTablesExist = true;
    vector<string> requiredTables = {
        "tasks", "projects", "challenges", "reminders", 
        "achievements", "user_stats", "user_settings"
    };
    
    for (const string& table : requiredTables) {
        if (!db.tableExists(table)) {
            cerr << "\033[1;31m[MISSING] Artifact '" << table << "' not found.\033[0m" << endl;
            allTablesExist = false;
        }
    }
    
    if (!allTablesExist) {
        return false;
    }
    
    // 3. 检查完整性 -> "Syncing with Server"
    simulateLoading("Syncing Player Stats     ");
    
    if (!db.checkDatabaseIntegrity()) {
        cerr << "\033[1;31m[ERROR] Data integrity breach detected!\033[0m" << endl;
        return false;
    }
    
    cout << "\n";
    typewriterPrint(">> System ready. Let's get things done.", 20, "\033[1;32m");
    cout << "\n";
    
    // 给用户一点时间看清楚加载结果
    sleepMs(1000);
    
    return true;
}

/**
 * @brief 清理系统资源
 */
void cleanupSystem() {
    cout << "\n\033[1;33m>> Saving progress...\033[0m\n";
    sleepMs(500);
    
    // 关闭数据库连接
    DatabaseManager::destroyInstance();
    
    simulateLoading("Closing Quest Log        ");
}

/**
 * @brief 主函数
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    // === 步骤 0: 设置控制台环境 (修复乱码的关键) ===
    setupConsole();
    
    // 初始化随机数种子 (让加载动画的延迟真正随机)
    srand(static_cast<unsigned int>(time(nullptr)));

    try {
        // 1. 酷炫的开场
        displayWelcomeBanner();
        
        // 2. 游戏化的初始化流程
        if (!initializeSystem()) {
            cerr << "\n\033[1;31m💥 System Launch Aborted.\033[0m\n";
            // 防止窗口在报错后瞬间关闭
            cout << "Press ENTER to exit...";
            cin.get();
            return 1;
        }
        
        // 3. 启动 UI (将控制权交给 UIManager)
        cout << "\033[1;36m>> Press ENTER to Start Session <<\033[0m";
        cin.get(); 
        
        UIManager* ui = new UIManager();
        ui->run();
        
        // 4. 优雅退出
        delete ui;
        cleanupSystem();
        
        cout << "\n";
        cout << "\033[1;36m"; // 青色
        cout << "╔═══════════════════════════════════════════════════╗\n";
        cout << "║                                                   ║\n";
        cout << "║      See you next time. Keep the streak! 🔥       ║\n";
        cout << "║                                                   ║\n";
        cout << "╚═══════════════════════════════════════════════════╝\n";
        cout << "\033[0m\n";
        
        return 0;
        
    } catch (const exception& e) {
        cerr << "\n\033[1;31m[CRITICAL EXCEPTION] " << e.what() << "\033[0m\n";
        cleanupSystem();
        return 1;
    } catch (...) {
        cerr << "\n\033[1;31m[UNKNOWN ERROR] The system crashed unexpectedly.\033[0m\n";
        cleanupSystem();
        return 1;
    }
}