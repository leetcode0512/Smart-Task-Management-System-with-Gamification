#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <memory>
#include <sqlite3.h>
#include "../entities/Task.h"
#include "../entities/Project.h"
#include "../entities/Reminder.h"


// 前置声明
class Task;
class Project;
class Reminder;
class Challenge;
class Achievement;
class ChallengeTemplate;
class UserAchievement;
class ExperienceRecord;
class UserRanking;
//后续补充

class DatabaseManager {
private:
    static DatabaseManager* instance;
    sqlite3* db;
    std::string dbPath;
    bool isTransactionActive;
    
    // 私有构造函数，单例模式
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
    
    // 获取原始数据库连接（谨慎使用）
    sqlite3* getRawConnection();
};


#endif // DATABASE_MANAGER_H
