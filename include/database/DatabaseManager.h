#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <atomic>
#include <sqlite3.h>

// 前置声明
class HeatmapVisualizer;
class Pomodoro;  // ✅ 修复：添加分号
class Achievement;
class gamification;
class Project;
class Reminder;
class statistics;
class Task;

// SQLite 删除器
struct SQLiteDeleter {
    void operator()(sqlite3* db) {
        if (db) sqlite3_close(db);
    }
};

class DatabaseManager {
private:
    static std::unique_ptr<DatabaseManager> instance;
    static std::mutex instanceMutex;  // 线程安全
    
    std::unique_ptr<sqlite3, SQLiteDeleter> db;  // 智能指针管理
    std::string dbPath;
    std::atomic<bool> isTransactionActive{false};
    std::atomic<long> totalQueryCount{0};
    std::atomic<long> failedQueryCount{0};
    
    mutable std::mutex dbMutex;  // 数据库操作互斥锁
    
    // 预编译语句缓存
    std::unordered_map<std::string, sqlite3_stmt*> preparedStatements;
    
    // 私有方法
    bool createProjectTable();
    bool createTaskTable();
    bool createChallengeTable();
    bool createReminderTable();
    bool createAchievementTable();
    bool createUserStatsTable();
    bool createUserSettingsTable();
    
    DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

public:
    ~DatabaseManager();
    
    // 单例访问
    static DatabaseManager* getInstance();
    static void destroyInstance();
    
    // 数据库连接管理
    bool initialize(const std::string& databasePath = "task_manager.db");
    bool close();
    bool isOpen() const;
    
    // 工具方法
    int getLastInsertId() const;
    bool execute(const std::string& sql);
    
    // ✅ 新增：参数化查询
    bool executeParameterized(const std::string& sql, 
                             const std::vector<std::string>& params);
    
    // ✅ 新增：查询结果处理
    bool executeQuery(const std::string& sql, 
                     std::function<void(sqlite3_stmt*)> callback);
    
    // 事务管理
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool isInTransaction() const;
    
    // 数据库维护
    bool backupDatabase(const std::string& backupPath);
    bool restoreDatabase(const std::string& backupPath);
    bool vacuumDatabase();
    bool checkDatabaseIntegrity();
    
    // 表管理
    bool createTables();
    bool dropTables();
    bool tableExists(const std::string& tableName);
    std::vector<std::string> getAllTableNames();
    
    // 错误处理
    std::string getLastErrorMessage() const;
    int getLastErrorCode() const;
    bool hasError() const;
    
    // 性能统计
    long getTotalQueryCount() const;
    long getFailedQueryCount() const;
    void resetStatistics();
    
    // 获取原始连接（谨慎使用）
    sqlite3* getRawConnection();
    
    std::string getDatabasePath() const;
};

#endif // DATABASE_MANAGER_H
