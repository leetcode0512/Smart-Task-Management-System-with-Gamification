#include "database/DAO/TaskDAO.h"
#include "database/DatabaseManager.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include <optional>

// =====================
// 构造函数
// =====================
TaskDAOImpl::TaskDAOImpl(const std::string& dbPath) {
    databasePath = dbPath.empty() ? "task_manager.db" : dbPath;

    // 确保与全局 DatabaseManager 使用同一个数据库文件，避免 TaskManager 与统计/成就使用不同数据源
    auto& dbManager = DatabaseManager::getInstance();
    if (!dbManager.isOpen()) {
        dbManager.initialize(databasePath);
    }

    createTable(); // 自动创建表
}

sqlite3* TaskDAOImpl::getDatabaseConnection() {
    auto& dbManager = DatabaseManager::getInstance();
    if (!dbManager.isOpen() && !dbManager.initialize(databasePath)) {
        std::cerr << "无法初始化数据库: " << databasePath << std::endl;
        return nullptr;
    }

    return dbManager.getRawConnection();
}

bool TaskDAOImpl::executeSQL(const std::string& sql) {
    auto& dbManager = DatabaseManager::getInstance();
    if (!dbManager.isOpen() && !dbManager.initialize(databasePath)) {
        return false;
    }

    bool success = dbManager.execute(sql);

    if (!success) {
        std::cerr << "SQL执行错误: " << dbManager.getLastErrorMessage() << std::endl;
    }

    return success;
}

bool TaskDAOImpl::createTable() {
    // 复用 DatabaseManager 中的统一 schema，确保与 StatisticsAnalyzer / AchievementManager 读取的数据一致
    auto& dbManager = DatabaseManager::getInstance();
    if (!dbManager.isOpen() && !dbManager.initialize(databasePath)) {
        return false;
    }

    // DatabaseManager::initialize 会调用 createTables()，此处简单检查任务表是否存在
    return dbManager.tableExists("tasks") || dbManager.createTables();
}

int TaskDAOImpl::insertTask(const Task& task) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return -1;

    const char* sql = "INSERT INTO tasks (title, description, completed, project_id) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, task.getName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, task.getDescription().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, task.isCompleted() ? 1 : 0);
    sqlite3_bind_int(stmt, 4, task.getProjectId());

    int id = -1;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        id = static_cast<int>(sqlite3_last_insert_rowid(db));
    } else {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    return id;
}

std::optional<Task> TaskDAOImpl::getTaskById(int id) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return std::nullopt;

    const char* sql = "SELECT id, title, description, completed, project_id FROM tasks WHERE id = ? AND deleted = 0";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    std::optional<Task> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Task task;
        task.setId(sqlite3_column_int(stmt, 0));
        task.setName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        task.setDescription(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        task.setCompleted(sqlite3_column_int(stmt, 3) != 0);
        task.setProjectId(sqlite3_column_int(stmt, 4));
        result = task;
    }

    sqlite3_finalize(stmt);

    return result;
}

std::vector<Task> TaskDAOImpl::getAllTasks() {
    sqlite3* db = getDatabaseConnection();
    std::vector<Task> tasks;
    if (!db) return tasks;

    const char* sql = "SELECT id, title, description, completed, project_id FROM tasks WHERE deleted = 0 ORDER BY created_date DESC";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return tasks;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Task task;
        task.setId(sqlite3_column_int(stmt, 0));
        task.setName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        task.setDescription(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        task.setCompleted(sqlite3_column_int(stmt, 3) != 0);
        task.setProjectId(sqlite3_column_int(stmt, 4));
        tasks.push_back(task);
    }

    sqlite3_finalize(stmt);

    return tasks;
}

bool TaskDAOImpl::updateTask(const Task& task) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return false;

    const char* sql = R"(
        UPDATE tasks
        SET title = ?,
            description = ?,
            completed = ?,
            project_id = ?,
            updated_date = datetime('now'),
            completed_date = CASE WHEN ? = 1 THEN COALESCE(completed_date, datetime('now')) ELSE completed_date END
        WHERE id = ?
    )";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, task.getName().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, task.getDescription().c_str(), -1, SQLITE_STATIC);
    const int completed = task.isCompleted() ? 1 : 0;
    sqlite3_bind_int(stmt, 3, completed);
    sqlite3_bind_int(stmt, 4, task.getProjectId());
    sqlite3_bind_int(stmt, 5, completed);
    sqlite3_bind_int(stmt, 6, task.getId());

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    return success;
}

bool TaskDAOImpl::deleteTask(int id) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return false;

    const char* sql = "UPDATE tasks SET deleted = 1, updated_date = datetime('now') WHERE id = ?";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    return success;
}

std::vector<Task> TaskDAOImpl::getTasksByStatus(bool completed) {
    sqlite3* db = getDatabaseConnection();
    std::vector<Task> tasks;
    if (!db) return tasks;

    const char* sql = "SELECT id, title, description, completed, project_id FROM tasks WHERE completed = ? AND deleted = 0 ORDER BY created_date DESC";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return tasks;
    }

    sqlite3_bind_int(stmt, 1, completed ? 1 : 0);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Task task;
        task.setId(sqlite3_column_int(stmt, 0));
        task.setName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        task.setDescription(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        task.setCompleted(sqlite3_column_int(stmt, 3) != 0);
        task.setProjectId(sqlite3_column_int(stmt, 4));
        tasks.push_back(task);
    }

    sqlite3_finalize(stmt);

    return tasks;
}

std::vector<Task> TaskDAOImpl::getTasksByProject(int projectId) {
    sqlite3* db = getDatabaseConnection();
    std::vector<Task> tasks;
    if (!db) return tasks;

    const char* sql = "SELECT id, title, description, completed, project_id FROM tasks WHERE project_id = ? AND deleted = 0 ORDER BY created_date DESC";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return tasks;
    }

    sqlite3_bind_int(stmt, 1, projectId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Task task;
        task.setId(sqlite3_column_int(stmt, 0));
        task.setName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        task.setDescription(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        task.setCompleted(sqlite3_column_int(stmt, 3) != 0);
        task.setProjectId(sqlite3_column_int(stmt, 4));
        tasks.push_back(task);
    }

    sqlite3_finalize(stmt);

    return tasks;
}

std::vector<Task> TaskDAOImpl::getOverdueTasks() {
    sqlite3* db = getDatabaseConnection();
    std::vector<Task> tasks;
    if (!db) return tasks;

    const char* sql = "SELECT id, title, description, completed, project_id FROM tasks WHERE due_date < date('now') AND completed = 0 AND deleted = 0 ORDER BY due_date ASC";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return tasks;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Task task;
        task.setId(sqlite3_column_int(stmt, 0));
        task.setName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        task.setDescription(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        task.setCompleted(sqlite3_column_int(stmt, 3) != 0);
        task.setProjectId(sqlite3_column_int(stmt, 4));
        tasks.push_back(task);
    }

    sqlite3_finalize(stmt);

    return tasks;
}

std::vector<Task> TaskDAOImpl::getTodayTasks() {
    sqlite3* db = getDatabaseConnection();
    std::vector<Task> tasks;
    if (!db) return tasks;

    const char* sql = "SELECT id, title, description, completed, project_id FROM tasks WHERE due_date = date('now') AND deleted = 0 ORDER BY created_date DESC";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return tasks;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Task task;
        task.setId(sqlite3_column_int(stmt, 0));
        task.setName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        task.setDescription(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        task.setCompleted(sqlite3_column_int(stmt, 3) != 0);
        task.setProjectId(sqlite3_column_int(stmt, 4));
        tasks.push_back(task);
    }

    sqlite3_finalize(stmt);

    return tasks;
}

int TaskDAOImpl::countAllTasks() {
    sqlite3* db = getDatabaseConnection();
    if (!db) return 0;

    const char* sql = "SELECT COUNT(*) FROM tasks WHERE deleted = 0";
    sqlite3_stmt* stmt;
    int count = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return count;
}

int TaskDAOImpl::countCompletedTasks() {
    sqlite3* db = getDatabaseConnection();
    if (!db) return 0;

    const char* sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1 AND deleted = 0";
    sqlite3_stmt* stmt;
    int count = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return count;
}

bool TaskDAOImpl::assignTaskToProject(int taskId, int projectId) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return false;

    const char* sql = "UPDATE tasks SET project_id = ?, updated_date = datetime('now') WHERE id = ?";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, projectId);
    sqlite3_bind_int(stmt, 2, taskId);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    return success;
}

bool TaskDAOImpl::incrementPomodoro(int taskId) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return false;

    const char* sql = "UPDATE tasks SET pomodoro_count = pomodoro_count + 1, updated_date = datetime('now') WHERE id = ?";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, taskId);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    return success;
}

int TaskDAOImpl::getPomodoroCount(int taskId) {
    sqlite3* db = getDatabaseConnection();
    if (!db) return 0;

    const char* sql = "SELECT pomodoro_count FROM tasks WHERE id = ? AND deleted = 0";
    sqlite3_stmt* stmt;
    int count = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }

    sqlite3_bind_int(stmt, 1, taskId);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return count;
}
