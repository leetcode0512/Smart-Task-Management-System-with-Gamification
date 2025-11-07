// src/database/DatabaseManager.cpp
#include "include/database/DatabaseManager.h"
#include "common/entities.h"
#include <iostream>
#include <sstream>

DatabaseManager::DatabaseManager(const std::string& dbPath) : dbPath_(dbPath), db_(nullptr) {
    initialize();
}

DatabaseManager::~DatabaseManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool DatabaseManager::initialize() {
    int rc = sqlite3_open(dbPath_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "无法打开数据库: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    // 启用外键约束
    executeQuery("PRAGMA foreign_keys = ON;");
    
    // 创建所有表
    return createTables();
}

bool DatabaseManager::executeQuery(const std::string& query) {
    char* errorMsg = nullptr;
    int rc = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errorMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL错误: " << errorMsg << std::endl;
        std::cerr << "查询: " << query << std::endl;
        sqlite3_free(errorMsg);
        return false;
    }
    
    return true;
}

bool DatabaseManager::createTables() {
    bool success = true;
    
    // 创建项目表
    success &= createProjectTable();
    
    // 创建任务表（依赖于项目表）
    success &= createTaskTable();
    
    // 创建挑战表
    success &= createChallengeTable();
    
    // 创建提醒表（依赖于任务表）
    success &= createReminderTable();
    
    // 创建成就表
    success &= createAchievementTable();
    
    // 创建用户统计表
    success &= createUserStatsTable();
    
    // 创建用户设置表
    success &= createUserSettingsTable();
    
    if (success) {
        std::cout << "所有表创建成功!" << std::endl;
    } else {
        std::cerr << "部分表创建失败!" << std::endl;
    }
    
    return success;
}

bool DatabaseManager::createProjectTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS projects (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT,
            color_label TEXT DEFAULT '#3498db',
            progress REAL DEFAULT 0.0,
            total_tasks INTEGER DEFAULT 0,
            completed_tasks INTEGER DEFAULT 0,
            target_date TEXT,
            archived BOOLEAN DEFAULT FALSE,
            created_date TEXT DEFAULT (datetime('now')),
            updated_date TEXT DEFAULT (datetime('now'))
        );
    )";
    
    if (!executeQuery(sql)) {
        std::cerr << "创建项目表失败!" << std::endl;
        return false;
    }
    
    std::cout << "项目表创建/检查成功!" << std::endl;
    return true;
}

bool DatabaseManager::createTaskTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT,
            priority INTEGER DEFAULT 1 CHECK(priority IN (0, 1, 2)),
            due_date TEXT,
            completed BOOLEAN DEFAULT FALSE,
            tags TEXT,
            project_id INTEGER,
            pomodoro_count INTEGER DEFAULT 0,
            estimated_pomodoros INTEGER DEFAULT 0,
            completed_date TEXT,
            reminder_time TEXT,
            created_date TEXT DEFAULT (datetime('now')),
            updated_date TEXT DEFAULT (datetime('now')),
            FOREIGN KEY (project_id) REFERENCES projects (id) ON DELETE SET NULL
        );
    )";
    
    if (!executeQuery(sql)) {
        std::cerr << "创建任务表失败!" << std::endl;
        return false;
    }
    
    // 创建索引以提高查询性能
    executeQuery("CREATE INDEX IF NOT EXISTS idx_tasks_project_id ON tasks(project_id);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_tasks_completed ON tasks(completed);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks(due_date);");
    
    std::cout << "任务表创建/检查成功!" << std::endl;
    return true;
}

bool DatabaseManager::createChallengeTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS challenges (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT,
            type TEXT NOT NULL CHECK(type IN ('daily', 'weekly', 'monthly')),
            criteria TEXT,
            target_value INTEGER DEFAULT 0,
            current_value INTEGER DEFAULT 0,
            reward_xp INTEGER DEFAULT 0,
            completed BOOLEAN DEFAULT FALSE,
            claimed BOOLEAN DEFAULT FALSE,
            expiry_date TEXT,
            category TEXT CHECK(category IN ('task', 'pomodoro', 'project')),
            created_date TEXT DEFAULT (datetime('now')),
            updated_date TEXT DEFAULT (datetime('now'))
        );
    )";
    
    if (!executeQuery(sql)) {
        std::cerr << "创建挑战表失败!" << std::endl;
        return false;
    }
    
    // 创建索引
    executeQuery("CREATE INDEX IF NOT EXISTS idx_challenges_type ON challenges(type);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_challenges_completed ON challenges(completed);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_challenges_expiry ON challenges(expiry_date);");
    
    std::cout << "挑战表创建/检查成功!" << std::endl;
    return true;
}

bool DatabaseManager::createReminderTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS reminders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            message TEXT,
            trigger_time TEXT NOT NULL,
            recurrence TEXT DEFAULT 'once' CHECK(recurrence IN ('once', 'daily', 'weekly', 'monthly')),
            triggered BOOLEAN DEFAULT FALSE,
            task_id INTEGER,
            enabled BOOLEAN DEFAULT TRUE,
            last_triggered TEXT,
            created_date TEXT DEFAULT (datetime('now')),
            updated_date TEXT DEFAULT (datetime('now')),
            FOREIGN KEY (task_id) REFERENCES tasks (id) ON DELETE CASCADE
        );
    )";
    
    if (!executeQuery(sql)) {
        std::cerr << "创建提醒表失败!" << std::endl;
        return false;
    }
    
    // 创建索引
    executeQuery("CREATE INDEX IF NOT EXISTS idx_reminders_trigger_time ON reminders(trigger_time);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_reminders_enabled ON reminders(enabled);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_reminders_task_id ON reminders(task_id);");
    
    std::cout << "提醒表创建/检查成功!" << std::endl;
    return true;
}

bool DatabaseManager::createAchievementTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS achievements (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            description TEXT,
            icon TEXT,
            unlock_condition TEXT,
            unlocked BOOLEAN DEFAULT FALSE,
            unlocked_date TEXT,
            reward_xp INTEGER DEFAULT 0,
            category TEXT CHECK(category IN ('task', 'time', 'streak', 'special')),
            progress INTEGER DEFAULT 0 CHECK(progress >= 0 AND progress <= 100),
            target_value INTEGER DEFAULT 0,
            created_date TEXT DEFAULT (datetime('now')),
            updated_date TEXT DEFAULT (datetime('now'))
        );
    )";
    
    if (!executeQuery(sql)) {
        std::cerr << "创建成就表失败!" << std::endl;
        return false;
    }
    
    // 创建索引
    executeQuery("CREATE INDEX IF NOT EXISTS idx_achievements_category ON achievements(category);");
    executeQuery("CREATE INDEX IF NOT EXISTS idx_achievements_unlocked ON achievements(unlocked);");
    
    std::cout << "成就表创建/检查成功!" << std::endl;
    return true;
}

bool DatabaseManager::createUserStatsTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS user_stats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            total_tasks_created INTEGER DEFAULT 0,
            total_tasks_completed INTEGER DEFAULT 0,
            total_pomodoros INTEGER DEFAULT 0,
            current_streak INTEGER DEFAULT 0,
            longest_streak INTEGER DEFAULT 0,
            total_xp INTEGER DEFAULT 0,
            level INTEGER DEFAULT 1,
            last_active_date TEXT DEFAULT (date('now')),
            completion_rate REAL DEFAULT 0.0,
            achievements_unlocked INTEGER DEFAULT 0,
            created_date TEXT DEFAULT (datetime('now')),
            updated_date TEXT DEFAULT (datetime('now'))
        );
    )";
    
    if (!executeQuery(sql)) {
        std::cerr << "创建用户统计表失败!" << std::endl;
        return false;
    }
    
    // 初始化默认统计记录
    std::string initSql = R"(
        INSERT OR IGNORE INTO user_stats (id, total_tasks_created, total_tasks_completed, total_pomodoros, 
                                        current_streak, longest_streak, total_xp, level, completion_rate, achievements_unlocked)
        VALUES (1, 0, 0, 0, 0, 0, 0, 1, 0.0, 0);
    )";
    executeQuery(initSql);
    
    std::cout << "用户统计表创建/检查成功!" << std::endl;
    return true;
}

bool DatabaseManager::createUserSettingsTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS user_settings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            pomodoro_duration INTEGER DEFAULT 25 CHECK(pomodoro_duration > 0),
            short_break_duration INTEGER DEFAULT 5 CHECK(short_break_duration > 0),
            long_break_duration INTEGER DEFAULT 15 CHECK(long_break_duration > 0),
            pomodoros_until_long_break INTEGER DEFAULT 4 CHECK(pomodoros_until_long_break > 0),
            sound_enabled BOOLEAN DEFAULT TRUE,
            notifications_enabled BOOLEAN DEFAULT TRUE,
            theme TEXT DEFAULT 'default',
            language TEXT DEFAULT 'zh',
            auto_start_pomodoros BOOLEAN DEFAULT FALSE,
            created_date TEXT DEFAULT (datetime('now')),
            updated_date TEXT DEFAULT (datetime('now'))
        );
    )";
    
    if (!executeQuery(sql)) {
        std::cerr << "创建用户设置表失败!" << std::endl;
        return false;
    }
    
    // 初始化默认设置记录
    std::string initSql = R"(
        INSERT OR IGNORE INTO user_settings (id, pomodoro_duration, short_break_duration, long_break_duration, 
                                           pomodoros_until_long_break, sound_enabled, notifications_enabled, 
                                           theme, language, auto_start_pomodoros)
        VALUES (1, 25, 5, 15, 4, 1, 1, 'default', 'zh', 0);
    )";
    executeQuery(initSql);
    
    std::cout << "用户设置表创建/检查成功!" << std::endl;
    return true;
}

sqlite3* DatabaseManager::getDatabase() const {
    return db_;
}

bool DatabaseManager::beginTransaction() {
    return executeQuery("BEGIN TRANSACTION;");
}

bool DatabaseManager::commitTransaction() {
    return executeQuery("COMMIT;");
}

bool DatabaseManager::rollbackTransaction() {
    return executeQuery("ROLLBACK;");
}
