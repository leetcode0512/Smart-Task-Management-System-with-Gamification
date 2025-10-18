#ifndef MIGRATION_MANAGER_H
#define MIGRATION_MANAGER_H

#include <string>
#include <vector>

struct MigrationInfo {
    int version;
    std::string description;
    std::string script;
    std::chrono::system_clock::time_point appliedAt;
    
    MigrationInfo();
};

class MigrationManager {
public:
    virtual ~MigrationManager() = default;
    
    // 版本管理
    virtual bool checkDatabaseVersion() = 0;
    virtual bool upgradeDatabase(int targetVersion = -1) = 0;
    virtual int getCurrentSchemaVersion() = 0;
    virtual int getLatestSchemaVersion() = 0;
    
    // 迁移操作
    virtual bool runMigration(int fromVersion, int toVersion) = 0;
    virtual bool rollbackMigration(int toVersion) = 0;
    
    // 迁移历史
    virtual std::vector<MigrationInfo> getMigrationHistory() = 0;
    virtual bool recordMigration(const MigrationInfo& migration) = 0;
    
    // 数据迁移
    virtual bool migrateUserData() = 0;
    virtual bool migrateTaskData() = 0;
    virtual bool migrateProjectData() = 0;
    
    // 备份和恢复
    virtual bool createMigrationBackup() = 0;
    virtual bool restoreFromMigrationBackup() = 0;
    
    // 完整性检查
    virtual bool verifyDataIntegrity() = 0;
    virtual bool repairDataInconsistencies() = 0;
    
    // 迁移脚本管理
    virtual bool addMigrationScript(int version, const std::string& description, const std::string& script) = 0;
    virtual bool removeMigrationScript(int version) = 0;
};

#endif // MIGRATION_MANAGER_H