#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include "../database/DAO/ReminderDAO.h"  // 包含队友的DAO头文件
#include "Entities.h"  // 包含实体定义

class ReminderSystem {
private:
    std::vector<Reminder> reminders;
    std::unique_ptr<ReminderDAO> reminderDAO;
    
public:
    ReminderSystem(std::unique_ptr<ReminderDAO> dao);
    
    // 核心方法
    void initialize();
    void checkDueReminders();
    void addReminder(const std::string& title, const std::string& message,
                    const std::string& time, const std::string& rule = "once",
                    int task_id = 0);
    void displayAllReminders();
    void displayPendingReminders();
    
    // 工具方法
    bool isReminderDue(const Reminder& reminder) const;
    void processRecurringReminder(const Reminder& reminder);
    std::string calculateNextTriggerTime(const Reminder& reminder) const;
    bool loadRemindersFromDB();
    
    // 时间工具方法
    std::string getCurrentTime() const;
    std::time_t parseTimeString(const std::string& timeStr) const;
    std::string formatTime(std::time_t time) const;
    std::chrono::system_clock::time_point stringToTimePoint(const std::string& timeStr) const;
    
    // 新增方法
    std::vector<Reminder> getActiveReminders();
    std::vector<Reminder> getRemindersByTask(int taskId);
    std::vector<Reminder> getDueRemindersForToday();
    bool markReminderAsTriggered(int reminderId);
    bool rescheduleReminder(int reminderId, const std::string& newTime);
};
