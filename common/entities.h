// common/entities.h
#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <vector>
#include <chrono>
#include <map>

/**
 * @brief 任务状态枚举
 */
enum class TaskStatus {
    PENDING,        // 待处理
    IN_PROGRESS,    // 进行中
    COMPLETED,      // 已完成
    CANCELLED       // 已取消
};

/**
 * @brief 优先级枚举
 */
enum class Priority {
    LOW = 0,        // 低优先级
    MEDIUM = 1,     // 中优先级
    HIGH = 2        // 高优先级
};

/**
 * @brief 提醒类型枚举
 */
enum class ReminderType {
    ONCE,           // 一次性提醒
    DAILY,          // 每日提醒
    WEEKLY,         // 每周提醒
    MONTHLY         // 每月提醒
};

/**
 * @brief 提醒状态枚举
 */
enum class ReminderStatus {
    PENDING,        // 等待触发
    TRIGGERED,      // 已触发
    COMPLETED,      // 已完成
    CANCELLED       // 已取消
};

/**
 * @brief 基础实体类，包含所有实体的通用字段
 */
struct BaseEntity {
    int id = 0;                          // 唯一标识
    std::string created_date;            // 创建时间
    std::string updated_date;            // 更新时间
    
    BaseEntity() = default;
    virtual ~BaseEntity() = default;
};

/**
 * @brief 任务实体 - 核心业务对象
 * 
 * 负责人: Kuang Wenqing (任务管理模块)
 */
struct Task : BaseEntity {
    std::string title;                   // 任务标题
    std::string description;             // 任务描述
    int priority = 1;                    // 优先级 (0:低, 1:中, 2:高)
    std::string due_date;                // 截止日期 (ISO格式: YYYY-MM-DD)
    bool completed = false;              // 完成状态
    std::string tags;                    // 标签 (逗号分隔)
    int project_id = 0;                  // 所属项目ID
    int pomodoro_count = 0;              // 完成的番茄钟数量
    int estimated_pomodoros = 0;         // 预估番茄钟数
    std::string completed_date;          // 完成时间
    std::string reminder_time;           // 提醒时间
    
    Task() = default;
    Task(const std::string& t, const std::string& desc = "", int prio = 1) 
        : title(t), description(desc), priority(prio) {}
};

/**
 * @brief 项目实体 - 任务分组容器
 * 
 * 负责人: Zhou Tianjian (项目管理模块)
 * 注意: Project类的完整定义在 include/project/Project.h
 * 这里仅作为前向声明，避免循环依赖
 */
class Project;

/**
 * @brief 挑战实体 - 游戏化功能核心
 * 
 * 负责人: Yu Zhixuan (挑战系统)
 */
struct Challenge : BaseEntity {
    std::string title;                   // 挑战标题
    std::string description;             // 挑战描述
    std::string type;                    // 类型 ("daily", "weekly", "monthly")
    std::string criteria;                // 完成条件描述
    int target_value = 0;                // 目标值
    int current_value = 0;               // 当前进度值
    int reward_xp = 0;                   // 奖励经验值
    bool completed = false;              // 是否完成
    bool claimed = false;                // 是否已领取奖励
    std::string expiry_date;             // 过期时间
    std::string category;                // 分类 ("task", "pomodoro", "project")
    
    Challenge() = default;
    Challenge(const std::string& t, const std::string& desc, const std::string& typ) 
        : title(t), description(desc), type(typ) {}
};

/**
 * @brief 提醒实体 - 时间提醒功能
 * 
 * 负责人: Fei Yifan (提醒系统)
 */
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

/**
 * @brief 成就实体 - 用户成就系统
 * 
 * 负责人: Fei Yifan (成就系统)
 */
struct Achievement : BaseEntity {
    std::string name;                 // 成就展示名称
    std::string description;          // 成就描述
    std::string icon;                 // 成就图标
    std::string unlock_condition;     // 用于逻辑判断的唯一标识
    bool unlocked = false;            // 是否已解锁
    std::string unlocked_date;        // 解锁时间
    int reward_xp = 0;                // 奖励XP
    std::string category;             // 成就类别
    int progress = 0;                 // 当前进度
    int target_value = 0;             // 目标值
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

/**
 * @brief 用户统计实体 - 数据统计和分析
 * 
 * 负责人: Mao Jingqi (统计系统)
 */
struct UserStats : BaseEntity {
    int total_tasks_created = 0;         // 总创建任务数
    int total_tasks_completed = 0;       // 总完成任务数
    int total_pomodoros = 0;             // 总番茄钟数
    int current_streak = 0;              // 当前连续打卡天数
    int longest_streak = 0;              // 最长连续打卡天数
    int total_xp = 0;                    // 总经验值
    int level = 1;                       // 当前等级
    std::string last_active_date;        // 最后活跃日期
    double completion_rate = 0.0;        // 平均完成率
    int achievements_unlocked = 0;       // 已解锁成就数
    
    UserStats() = default;
};

/**
 * @brief 用户设置实体 - 个性化配置
 * 
 * 负责人: Mao Jingqi (UI/设置系统)
 */
struct UserSettings : BaseEntity {
    int pomodoro_duration = 25;          // 番茄钟工作时间 (分钟)
    int short_break_duration = 5;        // 短休息时间 (分钟)
    int long_break_duration = 15;        // 长休息时间 (分钟)
    int pomodoros_until_long_break = 4;  // 长休息间隔
    bool sound_enabled = true;           // 是否启用声音
    bool notifications_enabled = true;   // 是否启用通知
    std::string theme = "default";       // 主题设置
    std::string language = "zh";         // 语言设置
    bool auto_start_pomodoros = false;   // 是否自动开始番茄钟
    
    UserSettings() = default;
};

/**
 * @brief 用户等级信息 - 游戏化系统
 */
struct UserLevelInfo {
    int userId = 0;
    int currentLevel = 1;
    int totalXP = 0;
    int xpForCurrentLevel = 0;
    int xpForNextLevel = 100;
    double progressToNextLevel = 0.0;
};

/**
 * @brief 等级定义 - 游戏化系统
 */
struct LevelDefinition {
    int level = 1;
    int requiredXP = 0;
    std::string title;
    std::string description;
};

/**
 * @brief 经验值记录 - 游戏化系统
 */
struct ExperienceRecord {
    int id = 0;
    int userId = 0;
    int amount = 0;
    std::string source;
    std::string description;
    std::string timestamp;
};

/**
 * @brief 用户排名 - 游戏化系统
 */
struct UserRanking {
    int rank = 0;
    int userId = 0;
    std::string username;
    int totalXP = 0;
    int level = 1;
};

/**
 * @brief 每日完成统计 - 统计系统
 */
struct DailyCompletionStats {
    std::string date;
    int tasksCompleted = 0;
    int tasksCreated = 0;
    double completionRate = 0.0;
    int pomodorosCompleted = 0;
};

/**
 * @brief 热力图数据 - 可视化系统
 */
struct HeatmapData {
    std::string date;
    int taskCount = 0;
};

/**
 * @brief 生产力报告 - 统计系统
 */
struct ProductivityReport {
    std::string startDate;
    std::string endDate;
    int totalTasks = 0;
    int completedTasks = 0;
    double completionRate = 0.0;
    double averageTasksPerDay = 0.0;
};

/**
 * @brief 番茄钟统计 - 统计系统
 */
struct PomodoroStatistics {
    int totalPomodoros = 0;
    int pomodorosToday = 0;
    int pomodorosThisWeek = 0;
    double averagePomodorosPerDay = 0.0;
};

/**
 * @brief 连续记录 - 统计系统
 */
struct StreakRecord {
    int id = 0;
    std::string startDate;
    std::string endDate;
    int durationDays = 0;
    bool isActive = false;
};

#endif // ENTITIES_H
