#ifndef REMINDER_DAO_H
#define REMINDER_DAO_H

#include "entities.h"
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

    // 由于 ReminderType 已被移除，这里改为按 recurrence 字段查询
    // recurrence 示例: "once", "daily", "weekly", "monthly"
    virtual std::vector<Reminder> getRemindersByRecurrence(const std::string& recurrence) = 0;

    virtual std::vector<Reminder> getDueReminders(
        const std::chrono::system_clock::time_point& currentTime) = 0;
    
    // 时间相关查询
    virtual std::vector<Reminder> getRemindersDueToday() = 0;
    virtual std::vector<Reminder> getRemindersDueThisWeek() = 0;
};

#endif // REMINDER_DAO_H


