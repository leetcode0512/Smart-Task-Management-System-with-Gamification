#include "TaskDAO.h"
#include "common/Entities.h"
#include <sqlite3.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

class TaskDAOImpl : public TaskDAO {
private:
    sqlite3* db_;
    
    // 日期时间辅助函数
    std::string getCurrentDateTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
    
    std::string timePointToString(const std::chrono::system_clock::time_point& tp) {
        auto time_t = std::chrono::system_clock::to_time_t(tp);
        std::tm tm = *std::localtime(&time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
    
    std::chrono::system_clock::time_point stringToTimePoint(const std::string& str) {
        std::tm tm = {};
        std::istringstream iss(str);
        iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        auto time_t = std::mktime(&tm);
        return std::chrono::system_clock::from_time_t(time_t);
    }
    
    // 执行SQL语句
    bool executeSQL(const std::string& sql) {
        char* errorMessage = nullptr;
        int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMessage);
        if (result != SQLITE_OK) {
            if (errorMessage) {
                sqlite3_free(errorMessage);
            }
            return false;
        }
        return true;
    }
    
    // 从SQL结果构建Task对象
    Task buildTaskFromRow(sqlite3_stmt* stmt) {
        Task task;
        task.id = sqlite3_column_int(stmt, 0);
        task.created_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        task.updated_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        task.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        task.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        task.priority = sqlite3_column_int(stmt, 5);
        task.due_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        task.completed = sqlite3_column_int(stmt, 7) != 0;
        task.tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        task.project_id = sqlite3_column_int(stmt, 9);
        task.pomodoro_count = sqlite3_column_int(stmt, 10);
        task.estimated_pomodoros = sqlite3_column_int(stmt, 11);
        task.completed_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        task.reminder_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        return task;
    }
    
    // 标签处理辅助函数
    std::vector<std::string> splitTags(const std::string& tags) {
        std::vector<std::string> result;
        if (tags.empty()) return result;
        
        std::stringstream ss(tags);
        std::string tag;
        while (std::getline(ss, tag, ',')) {
            // 去除前后空格
            tag.erase(0, tag.find_first_not_of(' '));
            tag.erase(tag.find_last_not_of(' ') + 1);
            if (!tag.empty()) {
                result.push_back(tag);
            }
        }
        return result;
    }
    
    std::string joinTags(const std::vector<std::string>& tags) {
        std::ostringstream oss;
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << tags[i];
        }
        return oss.str();
    }

public:
    TaskDAOImpl(sqlite3* database) : db_(database) {}
    
    ~TaskDAOImpl() override {
        // 注意：我们不关闭数据库连接，由外部管理
    }

    // 基础CRUD操作
    bool insertTask(Task& task) override {
        std::string currentTime = getCurrentDateTime();
        task.created_date = currentTime;
        task.updated_date = currentTime;
        
        const char* sql = R"(
            INSERT INTO tasks (created_date, updated_date, title, description, priority, 
                             due_date, completed, tags, project_id, pomodoro_count, 
                             estimated_pomodoros, completed_date, reminder_time)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, task.created_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, task.updated_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, task.title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, task.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, task.priority);
        sqlite3_bind_text(stmt, 6, task.due_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 7, task.completed ? 1 : 0);
        sqlite3_bind_text(stmt, 8, task.tags.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 9, task.project_id);
        sqlite3_bind_int(stmt, 10, task.pomodoro_count);
        sqlite3_bind_int(stmt, 11, task.estimated_pomodoros);
        sqlite3_bind_text(stmt, 12, task.completed_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 13, task.reminder_time.c_str(), -1, SQLITE_TRANSIENT);
        
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        if (success) {
            task.id = sqlite3_last_insert_rowid(db_);
        }
        
        sqlite3_finalize(stmt);
        return success;
    }
    
    bool updateTask(const Task& task) override {
        std::string currentTime = getCurrentDateTime();
        
        const char* sql = R"(
            UPDATE tasks SET updated_date = ?, title = ?, description = ?, priority = ?,
                           due_date = ?, completed = ?, tags = ?, project_id = ?, 
                           pomodoro_count = ?, estimated_pomodoros = ?, completed_date = ?,
                           reminder_time = ?
            WHERE id = ?
        )";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, currentTime.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, task.title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, task.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, task.priority);
        sqlite3_bind_text(stmt, 5, task.due_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, task.completed ? 1 : 0);
        sqlite3_bind_text(stmt, 7, task.tags.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 8, task.project_id);
        sqlite3_bind_int(stmt, 9, task.pomodoro_count);
        sqlite3_bind_int(stmt, 10, task.estimated_pomodoros);
        sqlite3_bind_text(stmt, 11, task.completed_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 12, task.reminder_time.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 13, task.id);
        
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }
    
    bool deleteTask(int taskId) override {
        const char* sql = "DELETE FROM tasks WHERE id = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_int(stmt, 1, taskId);
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }
    
    bool softDeleteTask(int taskId) override {
        // 在您的实体中没有软删除字段，这里假设有一个deleted字段
        // 如果需要软删除，需要在Task实体中添加bool deleted字段
        const char* sql = "UPDATE tasks SET deleted = 1 WHERE id = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_int(stmt, 1, taskId);
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }

    // 查询操作
    std::optional<Task> getTaskById(int taskId) override {
        const char* sql = "SELECT * FROM tasks WHERE id = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return std::nullopt;
        }
        
        sqlite3_bind_int(stmt, 1, taskId);
        
        std::optional<Task> result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = buildTaskFromRow(stmt);
        }
        
        sqlite3_finalize(stmt);
        return result;
    }
    
    std::vector<Task> getAllTasks() override {
        const char* sql = "SELECT * FROM tasks ORDER BY created_date DESC";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }
    
    std::vector<Task> getTasksByStatus(bool completed) override {
        const char* sql = "SELECT * FROM tasks WHERE completed = ? ORDER BY created_date DESC";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_int(stmt, 1, completed ? 1 : 0);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }
    
    std::vector<Task> getTasksByPriority(int priority) override {
        const char* sql = "SELECT * FROM tasks WHERE priority = ? ORDER BY created_date DESC";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_int(stmt, 1, priority);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }
    
    std::vector<Task> getTasksByDueDateRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) override {
        
        std::string startStr = timePointToString(start);
        std::string endStr = timePointToString(end);
        
        const char* sql = "SELECT * FROM tasks WHERE due_date BETWEEN ? AND ? ORDER BY due_date";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_text(stmt, 1, startStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, endStr.c_str(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }
    
    std::vector<Task> getOverdueTasks() override {
        std::string currentDate = getCurrentDateTime().substr(0, 10); // 只取日期部分
        
        const char* sql = "SELECT * FROM tasks WHERE due_date < ? AND completed = 0 ORDER BY due_date";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_text(stmt, 1, currentDate.c_str(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }
    
    std::vector<Task> getTasksDueToday() override {
        std::string today = getCurrentDateTime().substr(0, 10); // YYYY-MM-DD
        
        const char* sql = "SELECT * FROM tasks WHERE due_date = ? AND completed = 0";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_text(stmt, 1, today.c_str(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }
    
    std::vector<Task> getTasksDueThisWeek() override {
        // 计算本周的开始和结束日期
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time_t);
        
        // 计算本周的开始（周一）和结束（周日）
        int daysSinceMonday = tm.tm_wday == 0 ? 6 : tm.tm_wday - 1;
        tm.tm_mday -= daysSinceMonday;
        std::mktime(&tm);
        std::string weekStart = formatDate(tm);
        
        tm.tm_mday += 6;
        std::mktime(&tm);
        std::string weekEnd = formatDate(tm);
        
        const char* sql = "SELECT * FROM tasks WHERE due_date BETWEEN ? AND ? AND completed = 0 ORDER BY due_date";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_text(stmt, 1, weekStart.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, weekEnd.c_str(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }

    // 高级查询
    std::vector<Task> getTasksByProject(int projectId) override {
        const char* sql = "SELECT * FROM tasks WHERE project_id = ? ORDER BY created_date DESC";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_int(stmt, 1, projectId);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }
    
    std::vector<Task> getTasksByTag(const std::string& tag) override {
        std::string searchPattern = "%" + tag + "%";
        const char* sql = "SELECT * FROM tasks WHERE tags LIKE ? ORDER BY created_date DESC";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_text(stmt, 1, searchPattern.c_str(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }
    
    std::vector<Task> searchTasks(const std::string& keyword) override {
        std::string searchPattern = "%" + keyword + "%";
        const char* sql = "SELECT * FROM tasks WHERE title LIKE ? OR description LIKE ? ORDER BY created_date DESC";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_text(stmt, 1, searchPattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, searchPattern.c_str(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }
    
    std::vector<Task> getTasksWithPomodoroSessions(int minSessions) override {
        const char* sql = "SELECT * FROM tasks WHERE pomodoro_count >= ? ORDER BY pomodoro_count DESC";
        sqlite3_stmt* stmt;
        std::vector<Task> tasks;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return tasks;
        }
        
        sqlite3_bind_int(stmt, 1, minSessions);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            tasks.push_back(buildTaskFromRow(stmt));
        }
        
        sqlite3_finalize(stmt);
        return tasks;
    }

    // 批量操作
    bool updateTasksStatus(const std::vector<int>& taskIds, bool completed) override {
        if (taskIds.empty()) return true;
        
        // 构建IN子句的参数占位符
        std::string placeholders;
        for (size_t i = 0; i < taskIds.size(); ++i) {
            placeholders += "?";
            if (i < taskIds.size() - 1) placeholders += ",";
        }
        
        std::string sql = "UPDATE tasks SET completed = ?, updated_date = ? WHERE id IN (" + placeholders + ")";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        std::string currentTime = getCurrentDateTime();
        sqlite3_bind_int(stmt, 1, completed ? 1 : 0);
        sqlite3_bind_text(stmt, 2, currentTime.c_str(), -1, SQLITE_TRANSIENT);
        
        for (size_t i = 0; i < taskIds.size(); ++i) {
            sqlite3_bind_int(stmt, 3 + i, taskIds[i]);
        }
        
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }
    
    bool updateTasksPriority(const std::vector<int>& taskIds, int priority) override {
        if (taskIds.empty()) return true;
        
        std::string placeholders;
        for (size_t i = 0; i < taskIds.size(); ++i) {
            placeholders += "?";
            if (i < taskIds.size() - 1) placeholders += ",";
        }
        
        std::string sql = "UPDATE tasks SET priority = ?, updated_date = ? WHERE id IN (" + placeholders + ")";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        std::string currentTime = getCurrentDateTime();
        sqlite3_bind_int(stmt, 1, priority);
        sqlite3_bind_text(stmt, 2, currentTime.c_str(), -1, SQLITE_TRANSIENT);
        
        for (size_t i = 0; i < taskIds.size(); ++i) {
            sqlite3_bind_int(stmt, 3 + i, taskIds[i]);
        }
        
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }
    
    bool deleteCompletedTasks() override {
        const char* sql = "DELETE FROM tasks WHERE completed = 1";
        return executeSQL(sql);
    }

    // 统计查询
    int getTaskCountByStatus(bool completed) override {
        const char* sql = "SELECT COUNT(*) FROM tasks WHERE completed = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0;
        }
        
        sqlite3_bind_int(stmt, 1, completed ? 1 : 0);
        int count = 0;
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count;
    }
    
    int getTaskCountByPriority(int priority) override {
        const char* sql = "SELECT COUNT(*) FROM tasks WHERE priority = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0;
        }
        
        sqlite3_bind_int(stmt, 1, priority);
        int count = 0;
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count;
    }
    
    int getCompletedTaskCountToday() override {
        std::string today = getCurrentDateTime().substr(0, 10);
        const char* sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1 AND completed_date LIKE ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0;
        }
        
        sqlite3_bind_text(stmt, 1, (today + "%").c_str(), -1, SQLITE_TRANSIENT);
        int count = 0;
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count;
    }
    
    int getCompletedTaskCountThisWeek() override {
        // 计算本周的开始日期
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time_t);
        
        int daysSinceMonday = tm.tm_wday == 0 ? 6 : tm.tm_wday - 1;
        tm.tm_mday -= daysSinceMonday;
        std::mktime(&tm);
        std::string weekStart = formatDate(tm);
        
        const char* sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1 AND completed_date >= ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0;
        }
        
        sqlite3_bind_text(stmt, 1, weekStart.c_str(), -1, SQLITE_TRANSIENT);
        int count = 0;
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count;
    }
    
    double getAverageCompletionTime() override {
        const char* sql = R"(
            SELECT AVG(julianday(completed_date) - julianday(created_date)) 
            FROM tasks 
            WHERE completed = 1 AND completed_date IS NOT NULL AND created_date IS NOT NULL
        )";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0.0;
        }
        
        double avgDays = 0.0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            avgDays = sqlite3_column_double(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return avgDays;
    }

    // Pomodoro相关
    bool incrementPomodoroCount(int taskId) override {
        const char* sql = "UPDATE tasks SET pomodoro_count = pomodoro_count + 1, updated_date = ? WHERE id = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        std::string currentTime = getCurrentDateTime();
        sqlite3_bind_text(stmt, 1, currentTime.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, taskId);
        
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }
    
    bool setPomodoroCount(int taskId, int count) override {
        const char* sql = "UPDATE tasks SET pomodoro_count = ?, updated_date = ? WHERE id = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        std::string currentTime = getCurrentDateTime();
        sqlite3_bind_int(stmt, 1, count);
        sqlite3_bind_text(stmt, 2, currentTime.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, taskId);
        
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }
    
    int getTotalPomodoroCount() override {
        const char* sql = "SELECT SUM(pomodoro_count) FROM tasks";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return 0;
        }
        
        int total = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return total;
    }

    // 标签管理
    bool addTagToTask(int taskId, const std::string& tag) override {
        auto taskOpt = getTaskById(taskId);
        if (!taskOpt) return false;
        
        Task task = *taskOpt;
        auto tags = splitTags(task.tags);
        
        // 检查标签是否已存在
        if (std::find(tags.begin(), tags.end(), tag) != tags.end()) {
            return true; // 标签已存在，视为成功
        }
        
        tags.push_back(tag);
        task.tags = joinTags(tags);
        return updateTask(task);
    }
    
    bool removeTagFromTask(int taskId, const std::string& tag) override {
        auto taskOpt = getTaskById(taskId);
        if (!taskOpt) return false;
        
        Task task = *taskOpt;
        auto tags = splitTags(task.tags);
        
        auto it = std::find(tags.begin(), tags.end(), tag);
        if (it == tags.end()) {
            return true; // 标签不存在，视为成功
        }
        
        tags.erase(it);
        task.tags = joinTags(tags);
        return updateTask(task);
    }
    
    std::vector<std::string> getTagsForTask(int taskId) override {
        auto taskOpt = getTaskById(taskId);
        if (!taskOpt) return {};
        
        return splitTags(taskOpt->tags);
    }
    
    std::vector<std::string> getAllUniqueTags() override {
        const char* sql = "SELECT DISTINCT tags FROM tasks WHERE tags IS NOT NULL AND tags != ''";
        sqlite3_stmt* stmt;
        std::vector<std::string> allTags;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return allTags;
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* tagsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (tagsStr) {
                auto taskTags = splitTags(tagsStr);
                allTags.insert(allTags.end(), taskTags.begin(), taskTags.end());
            }
        }
        
        sqlite3_finalize(stmt);
        
        // 去重
        std::sort(allTags.begin(), allTags.end());
        allTags.erase(std::unique(allTags.begin(), allTags.end()), allTags.end());
        
        return allTags;
    }

    // 数据验证
    bool taskExists(int taskId) override {
        return getTaskById(taskId).has_value();
    }
    
    bool isTaskTitleUnique(const std::string& title) override {
        const char* sql = "SELECT COUNT(*) FROM tasks WHERE title = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
        int count = 0;
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
        return count == 0;
    }

private:
    std::string formatDate(const std::tm& tm) {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(4) << (tm.tm_year + 1900) << "-"
            << std::setw(2) << (tm.tm_mon + 1) << "-"
            << std::setw(2) << tm.tm_mday;
        return oss.str();
    }
};

// 工厂函数
std::unique_ptr<TaskDAO> createTaskDAO(sqlite3* db) {
    return std::make_unique<TaskDAOImpl>(db);
}
