#include "ReminderDAO.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

class SQLiteReminderDAO : public ReminderDAO {
private:
    sqlite3* db;
    std::string dbPath;

    // SQL 语句常量
    static constexpr const char* CREATE_TABLE_SQL = 
        "CREATE TABLE IF NOT EXISTS reminders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "message TEXT NOT NULL,"
        "trigger_time TEXT NOT NULL,"           // 存储为 ISO8601 字符串
        "reminder_type INTEGER NOT NULL,"       // ReminderType 枚举值
        "status INTEGER NOT NULL,"              // ReminderStatus 枚举值
        "task_id INTEGER DEFAULT 0,"
        "recurrence_rule TEXT DEFAULT '',"      // 重复规则
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ");";

    static constexpr const char* INSERT_REMINDER_SQL =
        "INSERT INTO reminders (title, message, trigger_time, reminder_type, status, task_id, recurrence_rule) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    static constexpr const char* UPDATE_REMINDER_SQL =
        "UPDATE reminders SET title=?, message=?, trigger_time=?, reminder_type=?, "
        "status=?, task_id=?, recurrence_rule=?, updated_at=CURRENT_TIMESTAMP WHERE id=?;";

    static constexpr const char* DELETE_REMINDER_SQL =
        "DELETE FROM reminders WHERE id=?;";

    static constexpr const char* SELECT_BY_ID_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders WHERE id=?;";

    static constexpr const char* SELECT_ALL_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders ORDER BY trigger_time ASC;";

    static constexpr const char* SELECT_ACTIVE_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders WHERE status = ? ORDER BY trigger_time ASC;";

    static constexpr const char* SELECT_BY_TASK_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders WHERE task_id=? ORDER BY trigger_time ASC;";

    static constexpr const char* SELECT_BY_TYPE_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders WHERE reminder_type=? ORDER BY trigger_time ASC;";

    static constexpr const char* SELECT_DUE_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders WHERE trigger_time <= ? AND status = ? ORDER BY trigger_time ASC;";

    static constexpr const char* SELECT_DUE_TODAY_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders WHERE date(trigger_time) = date('now') AND status IN (?, ?) "
        "ORDER BY trigger_time ASC;";

    static constexpr const char* SELECT_DUE_WEEK_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders WHERE date(trigger_time) BETWEEN date('now') AND date('now', '+6 days') "
        "AND status IN (?, ?) ORDER BY trigger_time ASC;";

    static constexpr const char* SELECT_BY_DATE_RANGE_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders WHERE trigger_time BETWEEN ? AND ? ORDER BY trigger_time ASC;";

    static constexpr const char* MARK_TRIGGERED_SQL =
        "UPDATE reminders SET status=?, updated_at=CURRENT_TIMESTAMP WHERE id=?;";

    static constexpr const char* MARK_COMPLETED_SQL =
        "UPDATE reminders SET status=?, updated_at=CURRENT_TIMESTAMP WHERE id=?;";

    static constexpr const char* RESCHEDULE_SQL =
        "UPDATE reminders SET trigger_time=?, updated_at=CURRENT_TIMESTAMP WHERE id=?;";

    static constexpr const char* SELECT_RECURRING_SQL =
        "SELECT id, title, message, trigger_time, reminder_type, status, task_id, recurrence_rule "
        "FROM reminders WHERE recurrence_rule != '' AND status != ? ORDER BY trigger_time ASC;";

    static constexpr const char* DELETE_EXPIRED_SQL =
        "DELETE FROM reminders WHERE status = ? AND trigger_time < datetime('now', '-30 days');";

    static constexpr const char* CLEANUP_COMPLETED_SQL =
        "DELETE FROM reminders WHERE status = ?;";

    static constexpr const char* COUNT_BY_STATUS_SQL =
        "SELECT COUNT(*) FROM reminders WHERE status=?;";

    static constexpr const char* COUNT_OVERDUE_SQL =
        "SELECT COUNT(*) FROM reminders WHERE trigger_time < datetime('now') AND status = ?;";

public:
    SQLiteReminderDAO(const std::string& databasePath = "reminders.db") 
        : db(nullptr), dbPath(databasePath) {}

    ~SQLiteReminderDAO() override {
        if (db) {
            sqlite3_close(db);
        }
    }

    bool initialize() {
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "无法打开数据库: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        char* errMsg = nullptr;
        rc = sqlite3_exec(db, CREATE_TABLE_SQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "创建表失败: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }

        std::cout << "ReminderDAO 初始化成功" << std::endl;
        return true;
    }

    // 时间转换工具函数
    static std::string timePointToString(const std::chrono::system_clock::time_point& tp) {
        auto time_t = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    static std::chrono::system_clock::time_point stringToTimePoint(const std::string& timeStr) {
        std::tm tm = {};
        std::stringstream ss(timeStr);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        auto time_t = std::mktime(&tm);
        return std::chrono::system_clock::from_time_t(time_t);
    }

    // 基础CRUD操作
    bool insertReminder(Reminder& reminder) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, INSERT_REMINDER_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "准备插入语句失败: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        std::string triggerTimeStr = timePointToString(reminder.triggerTime);

        sqlite3_bind_text(stmt, 1, reminder.title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, reminder.message.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, triggerTimeStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, static_cast<int>(reminder.type));
        sqlite3_bind_int(stmt, 5, static_cast<int>(reminder.status));
        sqlite3_bind_int(stmt, 6, reminder.taskId);
        sqlite3_bind_text(stmt, 7, reminder.recurrenceRule.c_str(), -1, SQLITE_TRANSIENT);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        if (success) {
            reminder.id = sqlite3_last_insert_rowid(db);
        }
        sqlite3_finalize(stmt);
        
        return success;
    }

    bool updateReminder(const Reminder& reminder) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, UPDATE_REMINDER_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "准备更新语句失败: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        std::string triggerTimeStr = timePointToString(reminder.triggerTime);

        sqlite3_bind_text(stmt, 1, reminder.title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, reminder.message.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, triggerTimeStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, static_cast<int>(reminder.type));
        sqlite3_bind_int(stmt, 5, static_cast<int>(reminder.status));
        sqlite3_bind_int(stmt, 6, reminder.taskId);
        sqlite3_bind_text(stmt, 7, reminder.recurrenceRule.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 8, reminder.id);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    bool deleteReminder(int reminderId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, DELETE_REMINDER_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "准备删除语句失败: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        sqlite3_bind_int(stmt, 1, reminderId);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    // 查询操作
    std::optional<Reminder> getReminderById(int reminderId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_BY_ID_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return std::nullopt;
        }

        sqlite3_bind_int(stmt, 1, reminderId);
        
        std::optional<Reminder> reminder = std::nullopt;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            reminder = extractReminderFromStatement(stmt);
        }

        sqlite3_finalize(stmt);
        return reminder;
    }

    std::vector<Reminder> getAllReminders() override {
        return executeQuery(SELECT_ALL_SQL);
    }

    std::vector<Reminder> getActiveReminders() override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_ACTIVE_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        sqlite3_bind_int(stmt, 1, static_cast<int>(ReminderStatus::PENDING));
        auto reminders = extractRemindersFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return reminders;
    }

    std::vector<Reminder> getRemindersByTask(int taskId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_BY_TASK_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        sqlite3_bind_int(stmt, 1, taskId);
        auto reminders = extractRemindersFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return reminders;
    }

    std::vector<Reminder> getRemindersByType(ReminderType type) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_BY_TYPE_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        sqlite3_bind_int(stmt, 1, static_cast<int>(type));
        auto reminders = extractRemindersFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return reminders;
    }

    std::vector<Reminder> getDueReminders(
        const std::chrono::system_clock::time_point& currentTime) override {
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_DUE_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        std::string currentTimeStr = timePointToString(currentTime);
        sqlite3_bind_text(stmt, 1, currentTimeStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, static_cast<int>(ReminderStatus::PENDING));

        auto reminders = extractRemindersFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return reminders;
    }

    // 时间相关查询
    std::vector<Reminder> getRemindersDueToday() override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_DUE_TODAY_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        sqlite3_bind_int(stmt, 1, static_cast<int>(ReminderStatus::PENDING));
        sqlite3_bind_int(stmt, 2, static_cast<int>(ReminderStatus::OVERDUE));

        auto reminders = extractRemindersFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return reminders;
    }

    std::vector<Reminder> getRemindersDueThisWeek() override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_DUE_WEEK_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        sqlite3_bind_int(stmt, 1, static_cast<int>(ReminderStatus::PENDING));
        sqlite3_bind_int(stmt, 2, static_cast<int>(ReminderStatus::OVERDUE));

        auto reminders = extractRemindersFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return reminders;
    }

    std::vector<Reminder> getRemindersByDateRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) override {
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_BY_DATE_RANGE_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        std::string startStr = timePointToString(start);
        std::string endStr = timePointToString(end);

        sqlite3_bind_text(stmt, 1, startStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, endStr.c_str(), -1, SQLITE_TRANSIENT);

        auto reminders = extractRemindersFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return reminders;
    }

    // 状态管理
    bool markReminderAsTriggered(int reminderId) override {
        return updateReminderStatus(reminderId, ReminderStatus::TRIGGERED);
    }

    bool markReminderAsCompleted(int reminderId) override {
        return updateReminderStatus(reminderId, ReminderStatus::COMPLETED);
    }

    bool rescheduleReminder(int reminderId, 
        const std::chrono::system_clock::time_point& newTime) override {
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, RESCHEDULE_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        std::string newTimeStr = timePointToString(newTime);
        sqlite3_bind_text(stmt, 1, newTimeStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, reminderId);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    // 重复提醒管理
    bool createNextRecurringReminder(int originalReminderId) override {
        auto original = getReminderById(originalReminderId);
        if (!original) {
            return false;
        }

        // 这里需要根据 recurrenceRule 计算下一次触发时间
        // 这是一个简化的实现，实际需要解析重复规则
        auto nextTime = original->triggerTime + std::chrono::hours(24); // 默认每天重复

        Reminder newReminder = *original;
        newReminder.id = 0; // 新记录
        newReminder.triggerTime = nextTime;
        newReminder.status = ReminderStatus::PENDING;

        return insertReminder(newReminder);
    }

    std::vector<Reminder> getRecurringReminders() override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_RECURRING_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        sqlite3_bind_int(stmt, 1, static_cast<int>(ReminderStatus::COMPLETED));
        auto reminders = extractRemindersFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return reminders;
    }

    // 批量操作
    bool deleteExpiredReminders() override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, DELETE_EXPIRED_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int(stmt, 1, static_cast<int>(ReminderStatus::COMPLETED));
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    bool cleanUpCompletedReminders() override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, CLEANUP_COMPLETED_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int(stmt, 1, static_cast<int>(ReminderStatus::COMPLETED));
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    // 统计查询
    int getReminderCountByStatus(ReminderStatus status) override {
        return executeCountQuery(COUNT_BY_STATUS_SQL, static_cast<int>(status));
    }

    int getOverdueReminderCount() override {
        return executeCountQuery(COUNT_OVERDUE_SQL, static_cast<int>(ReminderStatus::PENDING));
    }

private:
    std::vector<Reminder> executeQuery(const char* sql) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        auto reminders = extractRemindersFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return reminders;
    }

    std::vector<Reminder> extractRemindersFromStatement(sqlite3_stmt* stmt) {
        std::vector<Reminder> reminders;
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            auto reminder = extractReminderFromStatement(stmt);
            if (reminder) {
                reminders.push_back(*reminder);
            }
        }
        
        return reminders;
    }

    std::optional<Reminder> extractReminderFromStatement(sqlite3_stmt* stmt) {
        Reminder reminder;
        reminder.id = sqlite3_column_int(stmt, 0);
        reminder.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        reminder.message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        
        std::string triggerTimeStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        reminder.triggerTime = stringToTimePoint(triggerTimeStr);
        
        reminder.type = static_cast<ReminderType>(sqlite3_column_int(stmt, 4));
        reminder.status = static_cast<ReminderStatus>(sqlite3_column_int(stmt, 5));
        reminder.taskId = sqlite3_column_int(stmt, 6);
        reminder.recurrenceRule = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        
        return reminder;
    }

    bool updateReminderStatus(int reminderId, ReminderStatus status) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, MARK_TRIGGERED_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int(stmt, 1, static_cast<int>(status));
        sqlite3_bind_int(stmt, 2, reminderId);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    int executeCountQuery(const char* sql, int status) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return -1;
        }

        sqlite3_bind_int(stmt, 1, status);
        int count = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count;
    }
};

// 工厂函数
std::unique_ptr<ReminderDAO> createReminderDAO(const std::string& dbPath = "reminders.db") {
    auto dao = std::make_unique<SQLiteReminderDAO>(dbPath);
    if (dao->initialize()) {
        return dao;
    }
    return nullptr;
}
