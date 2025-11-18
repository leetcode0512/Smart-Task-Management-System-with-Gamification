#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <chrono>
#include <map>

// 提醒相关实体
struct Reminder {
    int id;
    std::string title;
    std::string message;
    std::string trigger_time;
    std::string recurrence; // "once", "daily", "weekly", "monthly"
    bool triggered;
    int task_id;
    bool enabled;
    std::string last_triggered;
    
    Reminder() 
        : id(0), triggered(false), task_id(0), enabled(true) {}
        
    Reminder(int id, const std::string& title, const std::string& message, 
             const std::string& trigger_time, const std::string& recurrence = "once", 
             int task_id = 0)
        : id(id), title(title), message(message), trigger_time(trigger_time),
          recurrence(recurrence), triggered(false), task_id(task_id), enabled(true) {}
};

// 提醒类型枚举
enum class ReminderType {
    ONCE,
    DAILY,
    WEEKLY,
    MONTHLY
};

// 提醒状态枚举
enum class ReminderStatus {
    PENDING,
    TRIGGERED,
    COMPLETED,
    CANCELLED
};

// 成就定义实体
struct AchievementDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string icon;
    int target_value;
    int reward_xp;
    std::string category;
    std::string unlock_condition;
    
    AchievementDefinition() : target_value(0), reward_xp(0) {}
    
    AchievementDefinition(const std::string& id, const std::string& name, 
                         const std::string& description, int target_value, 
                         const std::string& category, int reward_xp = 0)
        : id(id), name(name), description(description), target_value(target_value),
          category(category), reward_xp(reward_xp) {}
};

// 用户成就实体
struct UserAchievement {
    int id;
    int user_id;
    std::string achievement_id;
    bool unlocked;
    std::string unlocked_date;
    int progress;
    
    UserAchievement() 
        : id(0), user_id(0), unlocked(false), progress(0) {}
        
    UserAchievement(int user_id, const std::string& achievement_id)
        : id(0), user_id(user_id), achievement_id(achievement_id), 
          unlocked(false), progress(0) {}
};

// 成就统计结构
struct AchievementStats {
    int totalAchievements;
    int unlockedAchievements;
    int lockedAchievements;
    double unlockRate;
    std::map<std::string, int> achievementsByCategory;
    
    AchievementStats() 
        : totalAchievements(0), unlockedAchievements(0), 
          lockedAchievements(0), unlockRate(0.0) {}
};

#endif // ENTITIES_H
