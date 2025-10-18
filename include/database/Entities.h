#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <vector>
#include <chrono>
#include <optional>

// 枚举定义
enum class Priority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2
};

enum class TaskStatus {
    PENDING = 0,
    IN_PROGRESS = 1,
    COMPLETED = 2,
    CANCELLED = 3
};

enum class ReminderType {
    ONE_TIME = 0,
    DAILY = 1,
    WEEKLY = 2,
    CUSTOM = 3
};

enum class ReminderStatus {
    PENDING = 0,
    TRIGGERED = 1,
    COMPLETED = 2,
    CANCELLED = 3
};

enum class ChallengeType {
    DAILY = 0,
    WEEKLY = 1,
    SPECIAL = 2
};

enum class ChallengeStatus {
    ACTIVE = 0,
    COMPLETED = 1,
    EXPIRED = 2,
    FAILED = 3
};

// 实体结构体定义
struct Task {
    int id;
    std::string title;
    std::string description;
    Priority priority;
    std::chrono::system_clock::time_point dueDate;
    TaskStatus status;
    std::vector<std::string> tags;
    int projectId;
    int pomodoroCount;
    std::chrono::system_clock::time_point creationTime;
    std::chrono::system_clock::time_point lastModified;
    
    Task();
};

struct Project {
    int id;
    std::string name;
    std::string description;
    std::string colorCode;
    std::chrono::system_clock::time_point creationTime;
    double progress;
    
    Project();
};

struct Reminder {
    int id;
    std::string message;
    std::chrono::system_clock::time_point triggerTime;
    ReminderType type;
    int taskId;
    bool isRecurring;
    ReminderStatus status;
    std::chrono::minutes repeatInterval; // 用于自定义重复提醒
    
    Reminder();
};

struct ChallengeTemplate {
    std::string id;
    std::string name;
    std::string description;
    ChallengeType type;
    std::string objective;
    int targetValue;
    int experienceReward;
    bool isActive;
    
    ChallengeTemplate();
};

struct Challenge {
    int id;
    std::string templateId;
    std::string name;
    std::string description;
    ChallengeType type;
    int currentProgress;
    int targetValue;
    int experienceReward;
    ChallengeStatus status;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    
    Challenge();
};

struct AchievementDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string category;
    std::string unlockCriteria;
    int experienceReward;
    bool isSecret;
    
    AchievementDefinition();
};

struct UserAchievement {
    std::string achievementId;
    int userId;
    bool unlocked;
    std::chrono::system_clock::time_point unlockTime;
    int currentProgress;
    int targetValue;
    
    UserAchievement();
};

struct PomodoroSettings {
    int workDuration;           // 工作时间（分钟）
    int shortBreakDuration;     // 短休息时间（分钟）
    int longBreakDuration;      // 长休息时间（分钟）
    int sessionsBeforeLongBreak; // 长休息前的工作会话数
    
    PomodoroSettings();
};

struct ThemeSettings {
    std::string themeName;
    std::string colorScheme;
    bool enableColors;
    
    ThemeSettings();
};

struct ExperienceRecord {
    int id;
    int userId;
    int amount;
    std::string source;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    
    ExperienceRecord();
};

struct UserLevelInfo {
    int userId;
    int currentLevel;
    int currentExperience;
    int experienceToNextLevel;
    int totalExperience;
    
    UserLevelInfo();
};

struct LevelDefinition {
    int level;
    int experienceRequired;
    std::string levelName;
    
    LevelDefinition();
};

struct UserRanking {
    int userId;
    std::string userName;
    int experience;
    int level;
    int rank;
    
    UserRanking();
};

// 统计相关结构体
struct DailyCompletionStats {
    std::chrono::system_clock::time_point date;
    int totalTasks;
    int completedTasks;
    double completionRate;
    
    DailyCompletionStats();
};

struct ProductivityReport {
    std::chrono::system_clock::time_point startDate;
    std::chrono::system_clock::time_point endDate;
    int totalTasks;
    int completedTasks;
    double averageCompletionTime;
    int totalPomodoros;
    double productivityScore;
    
    ProductivityReport();
};

struct HeatmapData {
    std::chrono::system_clock::time_point date;
    int completionCount;
    int intensityLevel; // 0-4, 用于颜色显示
    
    HeatmapData();
};

struct StreakRecord {
    std::chrono::system_clock::time_point startDate;
    std::chrono::system_clock::time_point endDate;
    int streakLength;
    
    StreakRecord();
};

#endif // ENTITIES_H