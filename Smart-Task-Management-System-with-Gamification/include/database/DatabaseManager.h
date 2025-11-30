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
class Pomodoro;
class Achievement;
class Gamification;  // ✅ 修正：类名首字母大写
class Project;
class Reminder;
class Statistics;    // ✅ 修正：类名首字母大写
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
    static std::mutex instanceMutex;
    
    std::unique_ptr<sqlite3, SQLiteDeleter> db;
    std::string dbPath;
    std::atomic<bool> isTransactionActive{false};
    std::atomic<long> totalQueryCount{0};
    std::atomic<long> failedQueryCount{0};
    
    mutable std::mutex dbMutex;
    
    // 预编译语句缓存
    std::unordered_map<std::string, sqlite3_stmt*> preparedStatements;
    std::mutex stmtMutex;  // ✅ 新增：预编译语句的互斥锁
    
    // 私有方法
    bool createProjectTable();
    bool createTaskTable();
    bool createChallengeTable();
    bool createReminderTable();
    bool createAchievementTable();
    bool createUserStatsTable();
    bool createUserSettingsTable();
    bool createPomodoroTable();  // ✅ 新增：Pomodoro表
    
    // 清理预编译语句
    void cleanupPreparedStatements();

public:
    DatabaseManager();
    ~DatabaseManager();
    
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // 单例访问
    static DatabaseManager& getInstance();
    static void destroyInstance();
    
    // 数据库连接管理
    bool initialize(const std::string& databasePath = "task_manager.db");
    bool close();
    bool isOpen() const;
    
    // 工具方法
    int getLastInsertId() const;
    bool execute(const std::string& sql);
    
    // 参数化查询
    bool executeParameterized(const std::string& sql, 
                             const std::vector<std::string>& params);
    
    // 查询结果处理
    bool executeQuery(const std::string& sql, 
                     std::function<bool(sqlite3_stmt*)> rowCallback);  // ✅ 修改：返回bool表示是否继续
    
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
    double getSuccessRate() const;  // ✅ 新增：成功率
    void resetStatistics();
    
    // 获取原始连接（谨慎使用）
    sqlite3* getRawConnection();
    
    std::string getDatabasePath() const;
    
    // ✅ 新增：预编译语句管理
    sqlite3_stmt* getPreparedStatement(const std::string& sql);
    void releasePreparedStatement(const std::string& key);
};

#endif // DATABASE_MANAGER_H
