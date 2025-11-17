#include "database/DatabaseManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>

// 静态成员初始化
std::unique_ptr<DatabaseManager> DatabaseManager::instance = nullptr;
std::mutex DatabaseManager::instanceMutex;

DatabaseManager::DatabaseManager() 
    : db(nullptr)
    , dbPath("task_manager.db")
    , isTransactionActive(false)
    , totalQueryCount(0)
    , failedQueryCount(0) {
}

DatabaseManager::~DatabaseManager() {
    close();
    cleanupPreparedStatements();
}

void DatabaseManager::cleanupPreparedStatements() {
    std::lock_guard<std::mutex> lock(stmtMutex);
    for (auto& [key, stmt] : preparedStatements) {
        if (stmt) {
            sqlite3_finalize(stmt);
        }
    }
    preparedStatements.clear();
}

DatabaseManager& DatabaseManager::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (!instance) {
        instance = std::unique_ptr<DatabaseManager>(new DatabaseManager());
    }
    return *instance;
}

void DatabaseManager::destroyInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    instance.reset();
}

bool DatabaseManager::initialize(const std::string& databasePath) {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    dbPath = databasePath;
    
    // 打开数据库连接
    sqlite3* rawDb = nullptr;
    int result = sqlite3_open(dbPath.c_str(), &rawDb);
    
    if (result != SQLITE_OK) {
        std::cerr << "无法打开数据库: " << sqlite3_errmsg(rawDb) << std::endl;
        if (rawDb) {
            sqlite3_close(rawDb);
        }
        return false;
    }
    
    db.reset(rawDb);
    
    // 启用外键约束和WAL模式以提高性能
    execute("PRAGMA foreign_keys = ON;");
    execute("PRAGMA journal_mode = WAL;");
    execute("PRAGMA synchronous = NORMAL;");
    execute("PRAGMA cache_size = -64000;"); // 64MB缓存
    
    // 创建表
    if (!createTables()) {
        std::cerr << "创建数据库表失败" << std::endl;
        return false;
    }
    
    std::cout << "数据库初始化成功: " << dbPath << std::endl;
    return true;
}

bool DatabaseManager::close() {
    std::lock_guard<std::mutex> lock(dbMutex);
    
    if (isTransactionActive) {
        rollbackTransaction();
    }
    
    cleanupPreparedStatements();
    
    if (db) {
        db.reset();
        std::cout << "数据库连接已关闭" << std::endl;
        return true;
    }
    
    return false;
}

bool DatabaseManager::isOpen() const {
    return db != nullptr;
}

bool DatabaseManager::createTables() {
    bool success = true;
    
    success = success && createTaskTable();
    success = success && createProjectTable();
    success = success && createChallengeTable();
    success = success && createReminderTable();
    success = success && createAchievementTable();
    success = success && createUserStatsTable();
    success = success && createUserSettingsTable();
    success = success && createPomodoroTable();
    
    return success;
}

bool DatabaseManager::createTaskTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_date TEXT NOT NULL DEFAULT (datetime('now')),
            updated_date TEXT NOT NULL DEFAULT (datetime('now')),
            title TEXT NOT NULL,
            description TEXT,
            priority INTEGER DEFAULT 1 CHECK(priority IN (0, 1, 2)),
            due_date TEXT,
            completed BOOLEAN DEFAULT 0,
            tags TEXT,
            project_id INTEGER DEFAULT 0,
            pomodoro_count INTEGER DEFAULT 0,
            estimated_pomodoros INTEGER DEFAULT 0,
            completed_date TEXT,
            reminder_time TEXT,
            deleted BOOLEAN DEFAULT 0,
            FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE SET NULL
        );
        
        CREATE INDEX IF NOT EXISTS idx_tasks_completed ON tasks(completed);
        CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks(priority);
        CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks(due_date);
        CREATE INDEX IF NOT EXISTS idx_tasks_project_id ON tasks(project_id);
        CREATE INDEX IF NOT EXISTS idx_tasks_created_date ON tasks(created_date);
        CREATE INDEX IF NOT EXISTS idx_tasks_deleted ON tasks(deleted);
    )";
    
    return execute(sql);
}

bool DatabaseManager::createProjectTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS projects (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_date TEXT NOT NULL DEFAULT (datetime('now')),
            updated_date TEXT NOT NULL DEFAULT (datetime('now')),
            name TEXT NOT NULL,
            description TEXT,
            color_label TEXT DEFAULT '#3498db',
            progress REAL DEFAULT 0.0 CHECK(progress >= 0.0 AND progress <= 1.0),
            total_tasks INTEGER DEFAULT 0,
            completed_tasks INTEGER DEFAULT 0,
            target_date TEXT,
            archived BOOLEAN DEFAULT 0
        );
        
        CREATE INDEX IF NOT EXISTS idx_projects_archived ON projects(archived);
        CREATE INDEX IF NOT EXISTS idx_projects_target_date ON projects(target_date);
    )";
    
    return execute(sql);
}

bool DatabaseManager::createChallengeTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS challenges (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_date TEXT NOT NULL DEFAULT (datetime('now')),
            updated_date TEXT NOT NULL DEFAULT (datetime('now')),
            title TEXT NOT NULL,
            description TEXT,
            type TEXT NOT NULL CHECK(type IN ('daily', 'weekly', 'monthly')),
            criteria TEXT,
            target_value INTEGER DEFAULT 0,
            current_value INTEGER DEFAULT 0,
            reward_xp INTEGER DEFAULT 0,
            completed BOOLEAN DEFAULT 0,
            claimed BOOLEAN DEFAULT 0,
            expiry_date TEXT,
            category TEXT CHECK(category IN ('task', 'pomodoro', 'project'))
        );
        
        CREATE INDEX IF NOT EXISTS idx_challenges_type ON challenges(type);
        CREATE INDEX IF NOT EXISTS idx_challenges_completed ON challenges(completed);
        CREATE INDEX IF NOT EXISTS idx_challenges_category ON challenges(category);
    )";
    
    return execute(sql);
}

bool DatabaseManager::createReminderTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS reminders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_date TEXT NOT NULL DEFAULT (datetime('now')),
            updated_date TEXT NOT NULL DEFAULT (datetime('now')),
            title TEXT NOT NULL,
            message TEXT,
            trigger_time TEXT NOT NULL,
            recurrence TEXT DEFAULT 'once' CHECK(recurrence IN ('once', 'daily', 'weekly', 'monthly')),
            triggered BOOLEAN DEFAULT 0,
            task_id INTEGER,
            enabled BOOLEAN DEFAULT 1,
            last_triggered TEXT,
            FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE CASCADE
        );
        
        CREATE INDEX IF NOT EXISTS idx_reminders_trigger_time ON reminders(trigger_time);
        CREATE INDEX IF NOT EXISTS idx_reminders_enabled ON reminders(enabled);
        CREATE INDEX IF NOT EXISTS idx_reminders_task_id ON reminders(task_id);
    )";
    
    return execute(sql);
}

bool DatabaseManager::createAchievementTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS achievements (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_date TEXT NOT NULL DEFAULT (datetime('now')),
            updated_date TEXT NOT NULL DEFAULT (datetime('now')),
            name TEXT NOT NULL UNIQUE,
            description TEXT,
            icon TEXT,
            unlock_condition TEXT,
            unlocked BOOLEAN DEFAULT 0,
            unlocked_date TEXT,
            reward_xp INTEGER DEFAULT 0,
            category TEXT CHECK(category IN ('task', 'time', 'streak', 'special')),
            progress INTEGER DEFAULT 0 CHECK(progress >= 0 AND progress <= 100),
            target_value INTEGER DEFAULT 0
        );
        
        CREATE INDEX IF NOT EXISTS idx_achievements_unlocked ON achievements(unlocked);
        CREATE INDEX IF NOT EXISTS idx_achievements_category ON achievements(category);
    )";
    
    return execute(sql);
}

bool DatabaseManager::createUserStatsTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS user_stats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_date TEXT NOT NULL DEFAULT (datetime('now')),
            updated_date TEXT NOT NULL DEFAULT (datetime('now')),
            total_tasks_created INTEGER DEFAULT 0,
            total_tasks_completed INTEGER DEFAULT 0,
            total_pomodoros INTEGER DEFAULT 0,
            current_streak INTEGER DEFAULT 0,
            longest_streak INTEGER DEFAULT 0,
            total_xp INTEGER DEFAULT 0,
            level INTEGER DEFAULT 1,
            last_active_date TEXT,
            completion_rate REAL DEFAULT 0.0,
            achievements_unlocked INTEGER DEFAULT 0
        );
    )";
    
    return execute(sql);
}

bool DatabaseManager::createUserSettingsTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS user_settings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_date TEXT NOT NULL DEFAULT (datetime('now')),
            updated_date TEXT NOT NULL DEFAULT (datetime('now')),
            pomodoro_duration INTEGER DEFAULT 25,
            short_break_duration INTEGER DEFAULT 5,
            long_break_duration INTEGER DEFAULT 15,
            pomodoros_until_long_break INTEGER DEFAULT 4,
            sound_enabled BOOLEAN DEFAULT 1,
            notifications_enabled BOOLEAN DEFAULT 1,
            theme TEXT DEFAULT 'default',
            language TEXT DEFAULT 'zh',
            auto_start_pomodoros BOOLEAN DEFAULT 0
        );
    )";
    
    return execute(sql);
}

bool DatabaseManager::createPomodoroTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS pomodoro_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_date TEXT NOT NULL DEFAULT (datetime('now')),
            task_id INTEGER,
            start_time TEXT NOT NULL,
            end_time TEXT,
            duration INTEGER,  -- 单位：分钟
            completed BOOLEAN DEFAULT 0,
            interrupted BOOLEAN DEFAULT 0,
            interruption_reason TEXT,
            FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE SET NULL
        );
        
        CREATE INDEX IF NOT EXISTS idx_pomodoro_task_id ON pomodoro_sessions(task_id);
        CREATE INDEX IF NOT EXISTS idx_pomodoro_start_time ON pomodoro_sessions(start_time);
        CREATE INDEX IF NOT EXISTS idx_pomodoro_completed ON pomodoro_sessions(completed);
    )";
    
    return execute(sql);
}

bool DatabaseManager::execute(const std::string& sql) {
    if (!db) return false;
    
    std::lock_guard<std::mutex> lock(dbMutex);
    totalQueryCount++;
    
    char* errorMsg = nullptr;
    int result = sqlite3_exec(db.get(), sql.c_str(), nullptr, nullptr, &errorMsg);
    
    if (result != SQLITE_OK) {
        failedQueryCount++;
        std::cerr << "SQL执行错误: " << (errorMsg ? errorMsg : "未知错误") 
                  << " (SQL: " << sql << ")" << std::endl;
        if (errorMsg) {
            sqlite3_free(errorMsg);
        }
        return false;
    }
    
    return true;
}

bool DatabaseManager::executeParameterized(const std::string& sql, 
                                         const std::vector<std::string>& params) {
    if (!db) return false;
    
    std::lock_guard<std::mutex> lock(dbMutex);
    totalQueryCount++;
    
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        failedQueryCount++;
        std::cerr << "准备参数化SQL失败: " << getLastErrorMessage() << std::endl;
        return false;
    }
    
    // 绑定参数
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
    }
    
    result = sqlite3_step(stmt);
    bool success = (result == SQLITE_DONE || result == SQLITE_ROW);
    
    if (!success) {
        failedQueryCount++;
        std::cerr << "执行参数化SQL失败: " << getLastErrorMessage() << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::executeQuery(const std::string& sql, 
                                 std::function<bool(sqlite3_stmt*)> rowCallback) {
    if (!db || !rowCallback) return false;
    
    std::lock_guard<std::mutex> lock(dbMutex);
    totalQueryCount++;
    
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        failedQueryCount++;
        std::cerr << "准备查询SQL失败: " << getLastErrorMessage() << std::endl;
        return false;
    }
    
    bool success = true;
    while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (!rowCallback(stmt)) {
            break;  // 回调返回false时停止处理
        }
    }
    
    if (result != SQLITE_DONE && result != SQLITE_ROW) {
        failedQueryCount++;
        success = false;
        std::cerr << "执行查询SQL失败: " << getLastErrorMessage() << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return success;
}

sqlite3_stmt* DatabaseManager::getPreparedStatement(const std::string& sql) {
    std::lock_guard<std::mutex> lock(stmtMutex);
    
    auto it = preparedStatements.find(sql);
    if (it != preparedStatements.end()) {
        return it->second;
    }
    
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        preparedStatements[sql] = stmt;
        return stmt;
    }
    
    return nullptr;
}

void DatabaseManager::releasePreparedStatement(const std::string& key) {
    std::lock_guard<std::mutex> lock(stmtMutex);
    
    auto it = preparedStatements.find(key);
    if (it != preparedStatements.end()) {
        sqlite3_finalize(it->second);
        preparedStatements.erase(it);
    }
}

bool DatabaseManager::beginTransaction() {
    if (isTransactionActive) {
        std::cerr << "事务已在进行中" << std::endl;
        return false;
    }
    
    if (execute("BEGIN TRANSACTION;")) {
        isTransactionActive = true;
        return true;
    }
    
    return false;
}

bool DatabaseManager::commitTransaction() {
    if (!isTransactionActive) {
        std::cerr << "没有活跃的事务可提交" << std::endl;
        return false;
    }
    
    if (execute("COMMIT TRANSACTION;")) {
        isTransactionActive = false;
        return true;
    }
    
    return false;
}

bool DatabaseManager::rollbackTransaction() {
    if (!isTransactionActive) {
        std::cerr << "没有活跃的事务可回滚" << std::endl;
        return false;
    }
    
    if (execute("ROLLBACK TRANSACTION;")) {
        isTransactionActive = false;
        return true;
    }
    
    return false;
}

bool DatabaseManager::isInTransaction() const {
    return isTransactionActive;
}

bool DatabaseManager::backupDatabase(const std::string& backupPath) {
    if (!db) return false;
    
    try {
        std::filesystem::copy_file(dbPath, backupPath, 
                                 std::filesystem::copy_options::overwrite_existing);
        std::cout << "数据库备份成功: " << backupPath << std::endl;
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "数据库备份失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::restoreDatabase(const std::string& backupPath) {
    if (!std::filesystem::exists(backupPath)) {
        std::cerr << "备份文件不存在: " << backupPath << std::endl;
        return false;
    }
    
    close();  // 关闭当前连接
    
    try {
        std::filesystem::copy_file(backupPath, dbPath, 
                                 std::filesystem::copy_options::overwrite_existing);
        
        // 重新初始化数据库
        return initialize(dbPath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "数据库恢复失败: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::vacuumDatabase() {
    return execute("VACUUM;");
}

bool DatabaseManager::checkDatabaseIntegrity() {
    bool integrityOk = false;
    
    executeQuery("PRAGMA integrity_check;", [&](sqlite3_stmt* stmt) {
        const char* result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        integrityOk = (result && std::string(result) == "ok");
        return false;  // 只处理第一行
    });
    
    return integrityOk;
}

bool DatabaseManager::dropTables() {
    const char* tables[] = {
        "pomodoro_sessions", "user_settings", "user_stats", 
        "achievements", "reminders", "challenges", "tasks", "projects"
    };
    
    bool success = true;
    for (const char* table : tables) {
        std::string sql = "DROP TABLE IF EXISTS " + std::string(table) + ";";
        success = success && execute(sql);
    }
    
    return success;
}

bool DatabaseManager::tableExists(const std::string& tableName) {
    bool exists = false;
    
    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, tableName.c_str(), -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = true;
        }
        
        sqlite3_finalize(stmt);
    }
    
    return exists;
}

std::vector<std::string> DatabaseManager::getAllTableNames() {
    std::vector<std::string> tables;
    
    executeQuery("SELECT name FROM sqlite_master WHERE type='table';", 
                [&](sqlite3_stmt* stmt) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (name) {
            tables.push_back(name);
        }
        return true;
    });
    
    return tables;
}

int DatabaseManager::getLastInsertId() const {
    if (!db) return 0;
    return sqlite3_last_insert_rowid(db.get());
}

std::string DatabaseManager::getLastErrorMessage() const {
    if (!db) return "Database not initialized";
    const char* errorMsg = sqlite3_errmsg(db.get());
    return errorMsg ? errorMsg : "Unknown error";
}

int DatabaseManager::getLastErrorCode() const {
    if (!db) return -1;
    return sqlite3_errcode(db.get());
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

double DatabaseManager::getSuccessRate() const {
    if (totalQueryCount == 0) return 100.0;
    return 100.0 * (totalQueryCount - failedQueryCount) / totalQueryCount;
}

void DatabaseManager::resetStatistics() {
    totalQueryCount = 0;
    failedQueryCount = 0;
}

sqlite3* DatabaseManager::getRawConnection() {
    return db.get();
}

std::string DatabaseManager::getDatabasePath() const {
    return dbPath;
}
