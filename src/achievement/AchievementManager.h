#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "AchievementDAO.h"  // 包含队友的DAO头文件

// 使用 Entities.h 中的 AchievementDefinition 和 UserAchievement
// 不再需要本地定义的 Achievement 结构体

class AchievementManager {
private:
    std::unique_ptr<AchievementDAO> achievementDAO;  // 使用智能指针管理DAO
    int currentUserId;  // 当前用户ID
    
    // 成就定义缓存
    std::vector<AchievementDefinition> achievementDefinitions;
    std::unordered_map<std::string, UserAchievement> userAchievements;
    
public:
    // 构造函数接收 AchievementDAO 和用户ID
    AchievementManager(std::unique_ptr<AchievementDAO> dao, int userId = 1);
    
    // 核心方法
    void initialize();
    void checkAllAchievements();
    void unlockAchievement(const std::string& achievementId);
    void updateAchievementProgress(const std::string& achievementId, int progress);
    void displayUnlockedAchievements();
    void displayAllAchievements();
    void displayAchievementStatistics();
    
    // 具体成就检查方法
    void checkFirstTaskAchievement();
    void checkSevenDayStreakAchievement();
    void checkTimeManagementAchievement();
    void checkPomodoroMasterAchievement();
    
    // 工具方法
    bool loadAchievementDefinitions();
    bool loadUserAchievements();
    void printAchievement(const AchievementDefinition& definition, 
                         const UserAchievement* userAchievement = nullptr) const;
    
    // 获取统计信息（需要队友实现其他DAO）
    int getCompletedTaskCount() const;
    int getCurrentStreak() const;
    int getDailyTaskCount(const std::string& date) const;
    int getTotalPomodoroCount() const;
    
    // 设置用户ID
    void setCurrentUserId(int userId);
    int getCurrentUserId() const;
};
