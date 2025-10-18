#ifndef ACHIEVEMENT_DAO_H
#define ACHIEVEMENT_DAO_H

#include "Entities.h"
#include <vector>
#include <optional>
#include <map>

struct AchievementStats {
    int totalAchievements;
    int unlockedAchievements;
    int lockedAchievements;
    double unlockRate;
    std::map<std::string, int> achievementsByCategory;
    
    AchievementStats();
};

class AchievementDAO {
public:
    virtual ~AchievementDAO() = default;
    
    // 成就定义管理
    virtual bool insertAchievementDefinition(const AchievementDefinition& definition) = 0;
    virtual bool updateAchievementDefinition(const AchievementDefinition& definition) = 0;
    
    // 用户成就管理
    virtual bool unlockAchievement(int userId, const std::string& achievementId) = 0;
    virtual bool updateAchievementProgress(int userId, const std::string& achievementId, int progress) = 0;
    
    // 查询操作
    virtual std::optional<AchievementDefinition> getAchievementDefinition(const std::string& achievementId) = 0;
    virtual std::vector<AchievementDefinition> getAllAchievementDefinitions() = 0;
    virtual std::vector<UserAchievement> getUserAchievements(int userId) = 0;
    virtual std::vector<UserAchievement> getUnlockedAchievements(int userId) = 0;
    virtual std::vector<UserAchievement> getLockedAchievements(int userId) = 0;
    virtual std::vector<UserAchievement> getInProgressAchievements(int userId) = 0;
    
    // 进度查询
    virtual std::optional<int> getAchievementProgress(int userId, const std::string& achievementId) = 0;
    virtual bool isAchievementUnlocked(int userId, const std::string& achievementId) = 0;
    
    // 统计查询
    virtual int getUnlockedAchievementCount(int userId) = 0;
    virtual int getTotalAchievementCount() = 0;
    virtual double getAchievementCompletionRate(int userId) = 0;
    virtual AchievementStats getAchievementStatistics() = 0;
    
    // 类别管理
    virtual std::vector<std::string> getAchievementCategories() = 0;
    virtual std::vector<AchievementDefinition> getAchievementsByCategory(const std::string& category) = 0;
};

#endif // ACHIEVEMENT_DAO_H