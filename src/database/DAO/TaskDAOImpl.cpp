#include "database/DA0/TaskDAO.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <ctime>

// =====================
// 构造函数
// =====================
TaskDAOImpl::TaskDAOImpl(const std::string& dbPath) : databasePath(dbPath) {
    createTable(); // 自动创建表
}

// =====================
// 数据库连接辅助方法
// =====================
sqlite3* TaskDAOImpl::getDatabaseConnection() {
    sqlite3* db = nullptr;
    if (sqlite3_open(databasePath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "无法打开数据库: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    return db;
}

bool TaskDAOImpl::executeSQL(const std::string& sql) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return false;
    
    char* errMsg = nullptr;
    bool success = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) == SQLITE_OK;
    
    if (!success) {
        std::cerr << "SQL执行错误: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
    return success;
}

// =====================
// 表管理
// =====================
bool TaskDAOImpl::createTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT,
            completed BOOLEAN DEFAULT 0,
            project_id INTEGER DEFAULT 0,
            due_date TEXT,
            pomodoro_count INTEGER DEFAULT 0,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return executeSQL(sql);
}

// =====================
// CRUD 操作
// =====================
int TaskDAOImpl::insertTask(const Task& task) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return -1;
    
    const char* sql = "INSERT INTO tasks (name, description, completed) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "准备语句失败: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, task.getName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, task.getDescription().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, task.isCompleted() ? 1 : 0);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "插入任务失败: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }
    
    int id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return id;
}

std::optional<Task> TaskDAOImpl::getTaskById(int id) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return std::nullopt;
    
    const char* sql = "SELECT name, description, completed FROM tasks WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return std::nullopt;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    std::optional<Task> result = std::nullopt;
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bool completed = sqlite3_column_int(stmt, 2) == 1;
        
        Task task(name, description);
        if (completed) {
            task.markCompleted();
        }
        result = task;
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

std::vector<Task> TaskDAOImpl::getAllTasks() {
    std::vector<Task> tasks;
    sqlite3* db = getDatabaseConnection();
    if (!db) return tasks;
    
    const char* sql = "SELECT name, description, completed FROM tasks ORDER BY created_at DESC";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return tasks;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bool completed = sqlite3_column_int(stmt, 2) == 1;
        
        Task task(name, description);
        if (completed) {
            task.markCompleted();
        }
        tasks.push_back(task);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return tasks;
}

bool TaskDAOImpl::updateTask(const Task& task) {
    // 注意：需要扩展Task类以包含ID才能实现此方法
    std::cerr << "updateTask 需要Task类包含ID字段" << std::endl;
    return false;
}

bool TaskDAOImpl::deleteTask(int id) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return false;
    
    const char* sql = "DELETE FROM tasks WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}

// =====================
// 查询操作
// =====================
std::vector<Task> TaskDAOImpl::getTasksByStatus(bool completed) {
    std::vector<Task> tasks;
    sqlite3* db = getDatabaseConnection();
    if (!db) return tasks;
    
    const char* sql = "SELECT name, description, completed FROM tasks WHERE completed = ? ORDER BY created_at DESC";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return tasks;
    }
    
    sqlite3_bind_int(stmt, 1, completed ? 1 : 0);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bool taskCompleted = sqlite3_column_int(stmt, 2) == 1;
        
        Task task(name, description);
        if (taskCompleted) {
            task.markCompleted();
        }
        tasks.push_back(task);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return tasks;
}

std::vector<Task> TaskDAOImpl::getTasksByProject(int projectId) {
    std::vector<Task> tasks;
    sqlite3* db = getDatabaseConnection();
    if (!db) return tasks;
    
    const char* sql = "SELECT name, description, completed FROM tasks WHERE project_id = ? ORDER BY created_at DESC";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return tasks;
    }
    
    sqlite3_bind_int(stmt, 1, projectId);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bool completed = sqlite3_column_int(stmt, 2) == 1;
        
        Task task(name, description);
        if (completed) {
            task.markCompleted();
        }
        tasks.push_back(task);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return tasks;
}

std::vector<Task> TaskDAOImpl::getOverdueTasks() {
    std::vector<Task> tasks;
    sqlite3* db = getDatabaseConnection();
    if (!db) return tasks;
    
    const char* sql = "SELECT name, description, completed FROM tasks WHERE due_date < date('now') AND completed = 0 ORDER BY due_date ASC";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return tasks;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bool completed = sqlite3_column_int(stmt, 2) == 1;
        
        Task task(name, description);
        if (completed) {
            task.markCompleted();
        }
        tasks.push_back(task);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return tasks;
}

std::vector<Task> TaskDAOImpl::getTodayTasks() {
    std::vector<Task> tasks;
    sqlite3* db = getDatabaseConnection();
    if (!db) return tasks;
    
    const char* sql = "SELECT name, description, completed FROM tasks WHERE due_date = date('now') ORDER BY created_at DESC";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return tasks;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bool completed = sqlite3_column_int(stmt, 2) == 1;
        
        Task task(name, description);
        if (completed) {
            task.markCompleted();
        }
        tasks.push_back(task);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return tasks;
}

// =====================
// 统计操作
// =====================
int TaskDAOImpl::countAllTasks() {
    sqlite3* db = getDatabaseConnection();
    if (!db) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM tasks";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
}

int TaskDAOImpl::countCompletedTasks() {
    sqlite3* db = getDatabaseConnection();
    if (!db) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
}

// =====================
// 项目分配
// =====================
bool TaskDAOImpl::assignTaskToProject(int taskId, int projectId) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return false;
    
    const char* sql = "UPDATE tasks SET project_id = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, projectId);
    sqlite3_bind_int(stmt, 2, taskId);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}

// =====================
// 番茄钟
// =====================
bool TaskDAOImpl::incrementPomodoro(int taskId) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return false;
    
    const char* sql = "UPDATE tasks SET pomodoro_count = pomodoro_count + 1, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, taskId);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return success;
}

int TaskDAOImpl::getPomodoroCount(int taskId) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return 0;
    
    const char* sql = "SELECT pomodoro_count FROM tasks WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_int(stmt, 1, taskId);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
}
