#ifndef REMINDER_DAO_H
#define REMINDER_DAO_H

include "common/entities.h"  // 包含实体定义
#include <vector>
#include <optional>
#include <chrono>
#include <string>

class ReminderDAO {
public:
    virtual ~ReminderDAO() = default;
    
    // 基础CRUD操作
    virtual bool insertReminder(Reminder& reminder) = 0;
    virtual bool updateReminder(const Reminder& reminder) = 0;
    virtual bool deleteReminder(int reminderId) = 0;
    
    // 查询操作
    virtual std::optional<Reminder> getReminderById(int reminderId) = 0;
    virtual std::vector<Reminder> getAllReminders() = 0;
    virtual std::vector<Reminder> getActiveReminders() = 0;
    virtual std::vector<Reminder> getRemindersByTask(int taskId) = 0;
    virtual std::vector<Reminder> getRemindersByType(ReminderType type) = 0;

    // 由于 ReminderType 已被移除，这里改为按 recurrence 字段查询
    // recurrence 示例: "once", "daily", "weekly", "monthly"
    virtual std::vector<Reminder> getRemindersByRecurrence(const std::string& recurrence) = 0;

    virtual std::vector<Reminder> getDueReminders(
        const std::chrono::system_clock::time_point& currentTime) = 0;
    
    // 时间相关查询
    virtual std::vector<Reminder> getRemindersDueToday() = 0;
    virtual std::vector<Reminder> getRemindersDueThisWeek() = 0;

    // 范围查询
    virtual std::vector<Reminder> getRemindersByDateRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) = 0;

    // 状态管理
    virtual bool markReminderAsTriggered(int reminderId) = 0;
    virtual bool markReminderAsCompleted(int reminderId) = 0;
    virtual bool rescheduleReminder(int reminderId,
        const std::chrono::system_clock::time_point& newTime) = 0;

    // 重复提醒
    virtual bool createNextRecurringReminder(int originalReminderId) = 0;
    virtual std::vector<Reminder> getRecurringReminders() = 0;

    // 清理与统计
    virtual bool deleteExpiredReminders() = 0;
    virtual bool cleanUpCompletedReminders() = 0;
    virtual int getReminderCountByStatus(ReminderStatus status) = 0;
    virtual int getOverdueReminderCount() = 0;
};

#endif // REMINDER_DAO_H



