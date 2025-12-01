#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <thread>
#include <atomic>
#include "../database/DAO/ReminderDAO.h"  // 包含队友的DAO头文件
include "common/entities.h"  // 包含实体定义

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
    // 重复提醒计算：根据当前时间和提醒类型计算下一次提醒时间
    std::string calculateNextReminderTime(
        const std::string& currentTime,
        ReminderType type) const;
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
    
    // 当提醒触发时，通知UI显示
    void notifyUser(const Reminder& reminder);
};

class ReminderDaemon {
private:
    ReminderSystem& reminderSystem;
    std::thread worker;
    std::atomic<bool> running{false};

    void runLoop();  // 后台循环：定期调用checkPendingReminders

public:
    explicit ReminderDaemon(ReminderSystem& system);
    ~ReminderDaemon();

    // 启动定期检查
    void startChecking();

    // 检查待触发的提醒（供守护线程或手动调用）
    void checkPendingReminders();

    // 触发单个提醒
    void triggerReminder(int reminderId);
};
