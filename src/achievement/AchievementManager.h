#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "include/database/DatabaseManager.h" // 包含队友的头文件

// 成就数据结构 - 与数据库表结构对应
struct Achievement {
    int id;
    std::string name;
    std::string description;
    std::string icon;
    std::string unlock_condition;
    bool unlocked;
    std::string unlocked_date;
    int reward_xp;
    std::string category;
    int progress;
    int target_value;
    
    Achievement(int id, const std::string& name, const std::string& desc, 
                const std::string& condition, int target, const std::string& cat)
        : id(id), name(name), description(desc), unlock_condition(condition),
          unlocked(false), reward_xp(0), category(cat), progress(0), target_value(target) {}
};

class AchievementManager {
private:
    std::vector<Achievement> achievements;
    DatabaseManager* dbManager; // 使用队友的DatabaseManager
    
    // 初始化成就定义
    void initializeAchievements();
    
public:
    AchievementManager(DatabaseManager* dm);
    
    // 核心方法
    void checkAllAchievements();
    void unlockAchievement(int achievementId);
    void updateAchievementProgress(int achievementId, int progress);
    void displayUnlockedAchievements();
    void displayAllAchievements();
    
    // 具体成就检查方法
    void checkFirstTaskAchievement();
    void checkSevenDayStreakAchievement();
    void checkTimeManagementAchievement();
    void checkPomodoroMasterAchievement();
    
    // 工具方法
    int getAchievementIndexById(int id) const;
    void printAchievement(const Achievement& achievement) const;
    
    // 数据库操作方法
    bool loadAchievementsFromDB();
    bool saveAchievementToDB(const Achievement& achievement);
};