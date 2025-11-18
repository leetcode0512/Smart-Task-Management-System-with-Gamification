#include "reminder/ReminderSystem.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

// 构造函数接收 ReminderDAO
ReminderSystem::ReminderSystem(std::unique_ptr<ReminderDAO> dao) 
    : reminderDAO(std::move(dao)) {
    initialize();
}

void ReminderSystem::initialize() {
    if (loadRemindersFromDB()) {
        std::cout << "提醒系统初始化完成，共加载 " << reminders.size() << " 个提醒\n";
    } else {
        std::cout << "提醒系统初始化失败\n";
    }
}

bool ReminderSystem::loadRemindersFromDB() {
    if (!reminderDAO) {
        std::cerr << "ReminderDAO 未初始化\n";
        return false;
    }
    
    try {
        // 使用DAO获取所有提醒
        reminders = reminderDAO->getAllReminders();
        std::cout << "从数据库加载了 " << reminders.size() << " 个提醒\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载提醒失败: " << e.what() << "\n";
        return false;
    }
}

void ReminderSystem::checkDueReminders() {
    if (!reminderDAO) {
        std::cerr << "ReminderDAO 未初始化\n";
        return;
    }
    
    auto currentTime = std::chrono::system_clock::now();
    
    std::cout << "=== 检查到期提醒 (" << getCurrentTime() << ") ===\n";
    
    try {
        // 使用DAO获取到期的提醒
        auto dueReminders = reminderDAO->getDueReminders(currentTime);
        int triggeredCount = 0;
        
        for (auto& reminder : dueReminders) {
            // 触发提醒
            std::cout << "🔔 提醒: " << reminder.title << "\n";
            std::cout << "   " << reminder.message << "\n";
            if (reminder.task_id > 0) {
                std::cout << "   关联任务ID: " << reminder.task_id << "\n";
            }
            std::cout << "   触发时间: " << reminder.trigger_time << "\n\n";
            
            // 标记为已触发
            if (markReminderAsTriggered(reminder.id)) {
                triggeredCount++;
                
                // 处理重复提醒
                if (reminder.recurrence != "once") {
                    processRecurringReminder(reminder);
                }
            }
        }
        
        if (triggeredCount == 0) {
            std::cout << "暂无到期提醒\n";
        } else {
            std::cout << "共触发 " << triggeredCount << " 个提醒\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "检查到期提醒失败: " << e.what() << "\n";
    }
    
    std::cout << "===================\n\n";
}

bool ReminderSystem::isReminderDue(const Reminder& reminder) const {
    // 解析提醒时间
    std::time_t reminderTime = parseTimeString(reminder.trigger_time);
    if (reminderTime == -1) return false;
    
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    return reminderTime <= now_time;
}

void ReminderSystem::processRecurringReminder(const Reminder& reminder) {
    std::string nextTime = calculateNextTriggerTime(reminder);
    
    // 创建新的提醒记录
    Reminder newReminder;
    newReminder.title = reminder.title;
    newReminder.message = reminder.message;
    newReminder.trigger_time = nextTime;
    newReminder.recurrence = reminder.recurrence;
    newReminder.task_id = reminder.task_id;
    newReminder.enabled = true;
    newReminder.triggered = false;
    
    // 使用DAO保存新提醒
    if (reminderDAO->insertReminder(newReminder)) {
        std::cout << "已创建下一次提醒，时间: " << nextTime << "\n";
    } else {
        std::cerr << "创建重复提醒失败\n";
    }
}

std::string ReminderSystem::calculateNextTriggerTime(const Reminder& reminder) const {
    std::time_t currentTime = parseTimeString(reminder.trigger_time);
    if (currentTime == -1) return "";
    
    std::time_t nextTime = currentTime;
    
    if (reminder.recurrence == "daily") {
        nextTime += 24 * 60 * 60; // 增加1天
    } else if (reminder.recurrence == "weekly") {
        nextTime += 7 * 24 * 60 * 60; // 增加1周
    } else if (reminder.recurrence == "monthly") {
        // 简单实现：增加30天
        nextTime += 30 * 24 * 60 * 60;
    }
    
    return formatTime(nextTime);
}

void ReminderSystem::addReminder(const std::string& title, const std::string& message,
                                const std::string& time, const std::string& rule,
                                int task_id) {
    if (!reminderDAO) {
        std::cerr << "ReminderDAO 未初始化\n";
        return;
    }
    
    Reminder newReminder;
    newReminder.title = title;
    newReminder.message = message;
    newReminder.trigger_time = time;
    newReminder.recurrence = rule;
    newReminder.task_id = task_id;
    newReminder.enabled = true;
    newReminder.triggered = false;
    
    if (reminderDAO->insertReminder(newReminder)) {
        std::cout << "✅ 已添加提醒: " << title << " (时间: " << time << ", 重复: " << rule << ")\n";
        // 重新加载提醒列表以包含新提醒
        loadRemindersFromDB();
    } else {
        std::cerr << "添加提醒失败\n";
    }
}

void ReminderSystem::displayAllReminders() {
    std::cout << "=== 所有提醒 (" << reminders.size() << "个) ===\n";
    for (const auto& reminder : reminders) {
        std::cout << (reminder.triggered ? "✅ " : "⏰ ");
        std::cout << "ID: " << reminder.id;
        std::cout << " | 时间: " << reminder.trigger_time;
        std::cout << " | 重复: " << reminder.recurrence;
        std::cout << " | 状态: " << (reminder.enabled ? "启用" : "禁用") << "\n";
        std::cout << "   标题: " << reminder.title << "\n";
        std::cout << "   内容: " << reminder.message;
        if (reminder.task_id > 0) {
            std::cout << " (任务ID: " << reminder.task_id << ")";
        }
        std::cout << "\n";
        if (reminder.triggered && !reminder.last_triggered.empty()) {
            std::cout << "   上次触发: " << reminder.last_triggered << "\n";
        }
        std::cout << "\n";
    }
    std::cout << "================\n\n";
}

void ReminderSystem::displayPendingReminders() {
    if (!reminderDAO) {
        std::cerr << "ReminderDAO 未初始化\n";
        return;
    }
    
    try {
        auto activeReminders = reminderDAO->getActiveReminders();
        std::cout << "=== 待处理提醒 ===\n";
        
        for (const auto& reminder : activeReminders) {
            std::cout << "⏰ ID: " << reminder.id;
            std::cout << " | 时间: " << reminder.trigger_time;
            std::cout << " | 重复: " << reminder.recurrence << "\n";
            std::cout << "   标题: " << reminder.title << "\n";
        }
        
        if (activeReminders.empty()) {
            std::cout << "暂无待处理提醒\n";
        } else {
            std::cout << "共 " << activeReminders.size() << " 个待处理提醒\n";
        }
        std::cout << "==================\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "获取待处理提醒失败: " << e.what() << "\n";
    }
}

// 新增方法实现
std::vector<Reminder> ReminderSystem::getActiveReminders() {
    if (reminderDAO) {
        return reminderDAO->getActiveReminders();
    }
    return {};
}

std::vector<Reminder> ReminderSystem::getRemindersByTask(int taskId) {
    if (reminderDAO) {
        return reminderDAO->getRemindersByTask(taskId);
    }
    return {};
}

std::vector<Reminder> ReminderSystem::getDueRemindersForToday() {
    if (reminderDAO) {
        return reminderDAO->getRemindersDueToday();
    }
    return {};
}

bool ReminderSystem::markReminderAsTriggered(int reminderId) {
    if (reminderDAO) {
        return reminderDAO->markReminderAsTriggered(reminderId);
    }
    return false;
}

bool ReminderSystem::rescheduleReminder(int reminderId, const std::string& newTime) {
    if (reminderDAO) {
        auto timePoint = stringToTimePoint(newTime);
        return reminderDAO->rescheduleReminder(reminderId, timePoint);
    }
    return false;
}

// 时间工具方法
std::string ReminderSystem::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    return formatTime(now_time);
}

std::time_t ReminderSystem::parseTimeString(const std::string& timeStr) const {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        return -1;
    }
    return std::mktime(&tm);
}

std::string ReminderSystem::formatTime(std::time_t time) const {
    std::tm* timeinfo = std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::chrono::system_clock::time_point ReminderSystem::stringToTimePoint(const std::string& timeStr) const {
    std::time_t time = parseTimeString(timeStr);
    if (time == -1) {
        return std::chrono::system_clock::time_point{};
    }
    return std::chrono::system_clock::from_time_t(time);
}
