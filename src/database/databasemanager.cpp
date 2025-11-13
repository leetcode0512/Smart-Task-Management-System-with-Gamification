#include "database/DatabaseManager.h"
#include <iostream>
#include <sstream>
#include <fstream>

// 初始化静态成员
DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager::DatabaseManager() 
    : db(nullptr)
    , isTransactionActive(false)
    , totalQueryCount(0)
    , failedQueryCount(0) {
}

DatabaseManager::~DatabaseManager() {
    close();
}

DatabaseManager* DatabaseManager::getInstance() {
    if (instance == nullptr) {
        instance = new DatabaseManager();
    }
    return instance;
}

void DatabaseManager::destroyInstance() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

bool DatabaseManager::initialize(const std::string& databasePath) {
    dbPath = databasePath;
    
    // 如果数据库已经打开，先关闭
    if (db != nullptr) {
        close();
    }
    
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "无法打开数据库: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // 启用外键约束
    execute("PRAGMA foreign_keys = ON;");
    
    // 启用WAL模式以提高并发性能
    execute("PRAGMA journal_mode = WAL;");
    
    // 创建所有表
    return createTables();
}

bool DatabaseManager::close() {
    if (db != nullptr) {
        // 如果事务正在进行，回滚
        if (isTransactionActive) {
            rollbackTransaction();
        }
        
        int rc = sqlite3_close(db);
        if (rc != SQLITE_OK) {
            std::cerr << "关闭数据库时出错: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        db = nullptr;
    }
    return true;
}

bool DatabaseManager::isOpen() const {
    return db != nullptr;
}

bool DatabaseManager::execute(const std::string& sql) {
    if (db == nullptr) {
        std::cerr << "数据库未初始化" << std::endl;
        return false;
    }
    
    char* errorMsg = nullptr;
    totalQueryCount++;
    
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMsg);
    
    if (rc != SQLITE_OK) {
        failedQueryCount++;
        std::cerr << "SQL错误: " << errorMsg << std::endl;
        std::cerr << "查询: " << sql << std::endl;
        sqlite3_free(errorMsg);
        return false;
    }
    
    return true;
}

int DatabaseManager::getLastInsertId() const {
    if (db == nullptr) {
        return -1;
    }
    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

bool DatabaseManager::beginTransaction() {
    if (isTransactionActive) {
        return false; // 事务已在进行中
    }
    
    if (execute("BEGIN TRANSACTION;")) {
        isTransactionActive = true;
        return true;
    }
    return false;
}

bool DatabaseManager::commitTransaction() {
    if (!isTransactionActive) {
        return false; // 没有活动的事务
    }
    
    if (execute("COMMIT;")) {
        isTransactionActive = false;
        return true;
    }
    return false;
}

bool DatabaseManager::rollbackTransaction() {
    if (!isTransactionActive) {
        return false; // 没有活动的事务
    }
    
    if (execute("ROLLBACK;")) {
        isTransactionActive = false;
        return true;
    }
    return false;
}

bool DatabaseManager::isInTransaction() const {
    return isTransactionActive;
}

bool DatabaseManager::backupDatabase(const std::string& backupPath) {
    if (db == nullptr) return false;
    
    sqlite3* backupDb;
    int rc = sqlite3_open(backupPath.c_str(), &backupDb);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    sqlite3_backup* backup = sqlite3_backup_init(backupDb, "main", db, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1); // 备份所有页面
        sqlite3_backup_finish(backup);
    }
    
    rc = sqlite3_errcode(backupDb);
    sqlite3_close(backupDb);
    
    return rc == SQLITE_OK;
}

bool DatabaseManager::restoreDatabase(const std::string& backupPath) {
    // 先关闭当前数据库连接
    close();
    
    // 备份当前数据库
    std::string currentBackup = dbPath + ".backup";
    if (backupDatabase(currentBackup)) {
        // 复制备份文件到当前数据库位置
        std::ifstream src(backupPath, std::ios::binary);
        std::ofstream dst(dbPath, std::ios::binary);
        dst << src.rdbuf();
        
        // 重新初始化数据库
        return initialize(dbPath);
    }
    return false;
}

bool DatabaseManager::vacuumDatabase() {
    return execute("VACUUM;");
}

bool DatabaseManager::checkDatabaseIntegrity() {
    bool integrityOk = true;
    
    sqlite3_exec(db, "PRAGMA integrity_check;", [](void* data, int argc, char** argv, char** colNames) -> int {
        if (argc > 0 && argv[0]) {
            std::string result = argv[0];
            if (result != "ok") {
                *static_cast<bool*>(data) = false;
                std::cerr << "数据库完整性检查失败: " << result << std::endl;
            }
        }
        return 0;
    }, &integrityOk, nullptr);
    
    return integrityOk;
}

bool DatabaseManager::createTables() {
    bool success = true;
    
    success &= createProjectTable();
    success &= createTaskTable();
    success &= createChallengeTable();
    success &= createReminderTable();
    success &= createAchievementTable();
    success &= createUserStatsTable();
    success &= createUserSettingsTable();
    
    if (success) {
        std::cout << "所有表创建成功!" << std::endl;
    } else {
        std::cerr << "部分表创建失败!" << std::endl;
    }
    
    return success;
}

bool DatabaseManager::dropTables() {
    std::vector<std::string> tables = {
        "reminders", "tasks", "projects", "challenges", 
        "achievements", "user_stats", "user_settings"
    };
    
    bool success = true;
    for (const auto& table : tables) {
        std::string sql = "DROP TABLE IF EXISTS " + table + ";";
        success &= execute(sql);
    }
    
    return success;
}

bool DatabaseManager::tableExists(const std::string& tableName) {
    if (db == nullptr) return false;
    
    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "';";
    
    bool exists = false;
    sqlite3_exec(db, sql.c_str(), [](void* data, int argc, char** argv, char** colNames) -> int {
        if (argc > 0) {
            *static_cast<bool*>(data) = true;
        }
        return 0;
    }, &exists, nullptr);
    
    return exists;
}

std::vector<std::string> DatabaseManager::getAllTableNames() {
    std::vector<std::string> tables;
    
    if (db == nullptr) return tables;
    
    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';";
    
    sqlite3_exec(db, sql.c_str(), [](void* data, int argc, char** argv, char** colNames) -> int {
        if (argc > 0 && argv[0]) {
            static_cast<std::vector<std::string>*>(data)->push_back(argv[0]);
        }
        return 0;
    }, &tables, nullptr);
    
    return tables;
}

std::string DatabaseManager::getLastErrorMessage() const {
    if (db == nullptr) {
        return "Database not initialized";
    }
    return sqlite3_errmsg(db);
}

int DatabaseManager::getLastErrorCode() const {
    if (db == nullptr) {
        return -1;
    }
    return sqlite3_errcode(db);
}

bool DatabaseManager::hasError() const {
    return getLastErrorCode() != SQLITE_OK;
}

long DatabaseManager::getTotalQueryCount() const {
    return totalQueryCount;
}

long DatabaseManager::getFailedQueryCount() const {
    return failedQueryCount;
}

void DatabaseManager::resetStatistics() {
    totalQueryCount = 0;
    failedQueryCount = 0;
}

sqlite3* DatabaseManager::getRawConnection() {
    return db;
}

std::string DatabaseManager::getDatabasePath() const {
    return dbPath;
}

// 以下是创建各个表的私有方法实现
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
    
    if (!execute(sql)) {
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
    
    if (!execute(sql)) {
        std::cerr << "创建任务表失败!" << std::endl;
        return false;
    }
    
    // 创建索引以提高查询性能
    execute("CREATE INDEX IF NOT EXISTS idx_tasks_project_id ON tasks(project_id);");
    execute("CREATE INDEX IF NOT EXISTS idx_tasks_completed ON tasks(completed);");
    execute("CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks(due_date);");
    
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
    
    if (!execute(sql)) {
        std::cerr << "创建挑战表失败!" << std::endl;
        return false;
    }
    
    // 创建索引
    execute("CREATE INDEX IF NOT EXISTS idx_challenges_type ON challenges(type);");
    execute("CREATE INDEX IF NOT EXISTS idx_challenges_completed ON challenges(completed);");
    execute("CREATE INDEX IF NOT EXISTS idx_challenges_expiry ON challenges(expiry_date);");
    
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
    
    if (!execute(sql)) {
        std::cerr << "创建提醒表失败!" << std::endl;
        return false;
    }
    
    // 创建索引
    execute("CREATE INDEX IF NOT EXISTS idx_reminders_trigger_time ON reminders(trigger_time);");
    execute("CREATE INDEX IF NOT EXISTS idx_reminders_enabled ON reminders(enabled);");
    execute("CREATE INDEX IF NOT EXISTS idx_reminders_task_id ON reminders(task_id);");
    
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
    
    if (!execute(sql)) {
        std::cerr << "创建成就表失败!" << std::endl;
        return false;
    }
    
    // 创建索引
    execute("CREATE INDEX IF NOT EXISTS idx_achievements_category ON achievements(category);");
    execute("CREATE INDEX IF NOT EXISTS idx_achievements_unlocked ON achievements(unlocked);");
    
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
    
    if (!execute(sql)) {
        std::cerr << "创建用户统计表失败!" << std::endl;
        return false;
    }
    
    // 初始化默认统计记录
    std::string initSql = R"(
        INSERT OR IGNORE INTO user_stats (id, total_tasks_created, total_tasks_completed, total_pomodoros, 
                                        current_streak, longest_streak, total_xp, level, completion_rate, achievements_unlocked)
        VALUES (1, 0, 0, 0, 0, 0, 0, 1, 0.0, 0);
    )";
    execute(initSql);
    
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
    
    if (!execute(sql)) {
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
    execute(initSql);
    
    std::cout << "用户设置表创建/检查成功!" << std::endl;
    return true;
}
