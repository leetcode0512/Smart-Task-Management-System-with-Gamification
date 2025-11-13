#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include "../database/DAO/ReminderDAO.h"  // 包含队友的DAO头文件

// 注意：现在使用 Entities.h 中的 Reminder 结构体，不需要重复定义
// 假设 Entities.h 中已经有 Reminder 的定义

class ReminderSystem {
private:
    std::vector<Reminder> reminders;
    std::unique_ptr<ReminderDAO> reminderDAO;  // 使用智能指针管理DAO
    
public:
    // 构造函数现在接收 ReminderDAO
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
    
    // 新增方法：获取不同类型的提醒
    std::vector<Reminder> getActiveReminders();
    std::vector<Reminder> getRemindersByTask(int taskId);
    std::vector<Reminder> getDueRemindersForToday();
    
    // 状态管理方法
    bool markReminderAsTriggered(int reminderId);
    bool rescheduleReminder(int reminderId, const std::string& newTime);
};
