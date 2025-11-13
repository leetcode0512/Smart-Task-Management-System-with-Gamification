#include "ProjectDAO.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

class SQLiteProjectDAO : public ProjectDAO {
private:
    sqlite3* db;
    std::string dbPath;

    // SQL 语句常量
    static constexpr const char* CREATE_PROJECTS_TABLE_SQL = 
        "CREATE TABLE IF NOT EXISTS projects ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "description TEXT,"
        "color_label TEXT,"
        "progress REAL DEFAULT 0.0,"
        "total_tasks INTEGER DEFAULT 0,"
        "completed_tasks INTEGER DEFAULT 0,"
        "target_date TEXT,"
        "archived BOOLEAN DEFAULT 0,"
        "created_date TEXT NOT NULL,"
        "updated_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ");";

    static constexpr const char* CREATE_TASKS_TABLE_SQL = 
        "CREATE TABLE IF NOT EXISTS tasks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "project_id INTEGER,"
        "title TEXT NOT NULL,"
        "description TEXT,"
        "status INTEGER DEFAULT 0,"  // 0: pending, 1: completed
        "created_date TEXT NOT NULL,"
        "due_date TEXT,"
        "FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE SET NULL"
        ");";

    static constexpr const char* INSERT_PROJECT_SQL =
        "INSERT INTO projects (name, description, color_label, target_date, created_date) "
        "VALUES (?, ?, ?, ?, ?);";

    static constexpr const char* UPDATE_PROJECT_SQL =
        "UPDATE projects SET name=?, description=?, color_label=?, progress=?, "
        "total_tasks=?, completed_tasks=?, target_date=?, archived=?, updated_at=CURRENT_TIMESTAMP "
        "WHERE id=?;";

    static constexpr const char* DELETE_PROJECT_SQL =
        "DELETE FROM projects WHERE id=?;";

    static constexpr const char* SOFT_DELETE_PROJECT_SQL =
        "UPDATE projects SET archived=1, updated_at=CURRENT_TIMESTAMP WHERE id=?;";

    static constexpr const char* SELECT_PROJECT_BY_ID_SQL =
        "SELECT id, name, description, color_label, progress, total_tasks, "
        "completed_tasks, target_date, archived, created_date "
        "FROM projects WHERE id=?;";

    static constexpr const char* SELECT_ALL_PROJECTS_SQL =
        "SELECT id, name, description, color_label, progress, total_tasks, "
        "completed_tasks, target_date, archived, created_date "
        "FROM projects ORDER BY created_date DESC;";

    static constexpr const char* SELECT_ACTIVE_PROJECTS_SQL =
        "SELECT id, name, description, color_label, progress, total_tasks, "
        "completed_tasks, target_date, archived, created_date "
        "FROM projects WHERE archived=0 ORDER BY created_date DESC;";

    static constexpr const char* SELECT_ARCHIVED_PROJECTS_SQL =
        "SELECT id, name, description, color_label, progress, total_tasks, "
        "completed_tasks, target_date, archived, created_date "
        "FROM projects WHERE archived=1 ORDER BY created_date DESC;";

    static constexpr const char* SELECT_COMPLETED_PROJECTS_SQL =
        "SELECT id, name, description, color_label, progress, total_tasks, "
        "completed_tasks, target_date, archived, created_date "
        "FROM projects WHERE progress >= 1.0 AND archived=0 ORDER BY created_date DESC;";

    static constexpr const char* SELECT_PROJECTS_BY_COLOR_SQL =
        "SELECT id, name, description, color_label, progress, total_tasks, "
        "completed_tasks, target_date, archived, created_date "
        "FROM projects WHERE color_label=? AND archived=0 ORDER BY created_date DESC;";

    static constexpr const char* UPDATE_PROJECT_PROGRESS_SQL =
        "UPDATE projects SET progress=?, updated_at=CURRENT_TIMESTAMP WHERE id=?;";

    static constexpr const char* UPDATE_TASK_COUNTS_SQL =
        "UPDATE projects SET total_tasks=?, completed_tasks=?, progress=?, "
        "updated_at=CURRENT_TIMESTAMP WHERE id=?;";

    static constexpr const char* ADD_TASK_TO_PROJECT_SQL =
        "UPDATE tasks SET project_id=? WHERE id=?;";

    static constexpr const char* REMOVE_TASK_FROM_PROJECT_SQL =
        "UPDATE tasks SET project_id=NULL WHERE id=? AND project_id=?;";

    static constexpr const char* MOVE_TASK_TO_PROJECT_SQL =
        "UPDATE tasks SET project_id=? WHERE id=?;";

    static constexpr const char* SELECT_TASK_IDS_IN_PROJECT_SQL =
        "SELECT id FROM tasks WHERE project_id=?;";

    static constexpr const char* SELECT_TASK_COUNT_SQL =
        "SELECT COUNT(*) FROM tasks WHERE project_id=?;";

    static constexpr const char* SELECT_COMPLETED_TASK_COUNT_SQL =
        "SELECT COUNT(*) FROM tasks WHERE project_id=? AND status=1;";

    static constexpr const char* SELECT_PROJECTS_BY_COMPLETION_RATE_SQL =
        "SELECT id, name, description, color_label, progress, total_tasks, "
        "completed_tasks, target_date, archived, created_date "
        "FROM projects WHERE progress BETWEEN ? AND ? AND archived=0 ORDER BY progress DESC;";

    static constexpr const char* SELECT_PROJECT_STATS_SQL =
        "SELECT "
        "COUNT(*) as total_tasks, "
        "SUM(CASE WHEN status=1 THEN 1 ELSE 0 END) as completed_tasks, "
        "SUM(CASE WHEN status=0 THEN 1 ELSE 0 END) as pending_tasks, "
        "CAST(SUM(CASE WHEN status=1 THEN 1 ELSE 0 END) AS REAL) / COUNT(*) as completion_rate, "
        "0 as total_pomodoros, "  // 假设番茄钟数据在另一个表中
        "MIN(t.created_date) as created_date, "
        "MAX(t.created_date) as last_activity "
        "FROM tasks t WHERE t.project_id=?;";

    static constexpr const char* COUNT_ALL_PROJECTS_SQL =
        "SELECT COUNT(*) FROM projects;";

    static constexpr const char* COUNT_ACTIVE_PROJECTS_SQL =
        "SELECT COUNT(*) FROM projects WHERE archived=0;";

    static constexpr const char* CHECK_PROJECT_EXISTS_SQL =
        "SELECT COUNT(*) FROM projects WHERE id=?;";

    static constexpr const char* CHECK_PROJECT_NAME_UNIQUE_SQL =
        "SELECT COUNT(*) FROM projects WHERE name=? AND id!=?;";

public:
    SQLiteProjectDAO(const std::string& databasePath = "projects.db") 
        : db(nullptr), dbPath(databasePath) {}

    ~SQLiteProjectDAO() override {
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

        // 创建项目表
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, CREATE_PROJECTS_TABLE_SQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "创建项目表失败: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }

        // 创建任务表
        rc = sqlite3_exec(db, CREATE_TASKS_TABLE_SQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "创建任务表失败: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }

        std::cout << "ProjectDAO 初始化成功" << std::endl;
        return true;
    }

    // 基础CRUD操作
    bool insertProject(Project& project) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, INSERT_PROJECT_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "准备插入语句失败: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        sqlite3_bind_text(stmt, 1, project.getName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, project.getDescription().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, project.getColorLabel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, project.getTargetDate().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, project.getCreatedDate().c_str(), -1, SQLITE_TRANSIENT);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        if (success) {
            project.setId(sqlite3_last_insert_rowid(db));
        }
        sqlite3_finalize(stmt);
        
        return success;
    }

    bool updateProject(const Project& project) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, UPDATE_PROJECT_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "准备更新语句失败: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        sqlite3_bind_text(stmt, 1, project.getName().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, project.getDescription().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, project.getColorLabel().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, project.getProgress());
        sqlite3_bind_int(stmt, 5, project.getTotalTasks());
        sqlite3_bind_int(stmt, 6, project.getCompletedTasks());
        sqlite3_bind_text(stmt, 7, project.getTargetDate().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 8, project.isArchived() ? 1 : 0);
        sqlite3_bind_int(stmt, 9, project.getId());

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    bool deleteProject(int projectId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, DELETE_PROJECT_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "准备删除语句失败: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        sqlite3_bind_int(stmt, 1, projectId);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    bool softDeleteProject(int projectId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SOFT_DELETE_PROJECT_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "准备软删除语句失败: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        sqlite3_bind_int(stmt, 1, projectId);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    // 查询操作
    std::optional<Project> getProjectById(int projectId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_PROJECT_BY_ID_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return std::nullopt;
        }

        sqlite3_bind_int(stmt, 1, projectId);
        
        std::optional<Project> project = std::nullopt;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            project = extractProjectFromStatement(stmt);
        }

        sqlite3_finalize(stmt);
        return project;
    }

    std::vector<Project> getAllProjects() override {
        return executeProjectQuery(SELECT_ALL_PROJECTS_SQL);
    }

    std::vector<Project> getActiveProjects() override {
        return executeProjectQuery(SELECT_ACTIVE_PROJECTS_SQL);
    }

    std::vector<Project> getArchivedProjects() override {
        return executeProjectQuery(SELECT_ARCHIVED_PROJECTS_SQL);
    }

    std::vector<Project> getCompletedProjects() override {
        return executeProjectQuery(SELECT_COMPLETED_PROJECTS_SQL);
    }

    std::vector<Project> getProjectsByColor(const std::string& colorCode) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_PROJECTS_BY_COLOR_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        sqlite3_bind_text(stmt, 1, colorCode.c_str(), -1, SQLITE_TRANSIENT);
        auto projects = extractProjectsFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return projects;
    }

    // 进度管理
    bool updateProjectProgress(int projectId) override {
        auto project = getProjectById(projectId);
        if (!project) {
            return false;
        }

        double progress = 0.0;
        if (project->getTotalTasks() > 0) {
            progress = static_cast<double>(project->getCompletedTasks()) / project->getTotalTasks();
        }

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, UPDATE_PROJECT_PROGRESS_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_double(stmt, 1, progress);
        sqlite3_bind_int(stmt, 2, projectId);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    bool updateTaskCounts(int projectId, int totalTasks, int completedTasks) override {
        double progress = 0.0;
        if (totalTasks > 0) {
            progress = static_cast<double>(completedTasks) / totalTasks;
        }

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, UPDATE_TASK_COUNTS_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int(stmt, 1, totalTasks);
        sqlite3_bind_int(stmt, 2, completedTasks);
        sqlite3_bind_double(stmt, 3, progress);
        sqlite3_bind_int(stmt, 4, projectId);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        return success;
    }

    double calculateProjectProgress(int projectId) override {
        auto project = getProjectById(projectId);
        if (!project) {
            return 0.0;
        }
        return project->getProgress();
    }

    // 任务关联管理
    bool addTaskToProject(int taskId, int projectId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, ADD_TASK_TO_PROJECT_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int(stmt, 1, projectId);
        sqlite3_bind_int(stmt, 2, taskId);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        if (success) {
            // 更新任务计数
            updateProjectTaskCounts(projectId);
        }
        
        return success;
    }

    bool removeTaskFromProject(int taskId, int projectId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, REMOVE_TASK_FROM_PROJECT_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int(stmt, 1, taskId);
        sqlite3_bind_int(stmt, 2, projectId);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        if (success) {
            // 更新任务计数
            updateProjectTaskCounts(projectId);
        }
        
        return success;
    }

    bool moveTaskToProject(int taskId, int newProjectId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, MOVE_TASK_TO_PROJECT_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_int(stmt, 1, newProjectId);
        sqlite3_bind_int(stmt, 2, taskId);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        
        if (success) {
            // 更新两个项目的任务计数
            updateProjectTaskCounts(newProjectId);
            // 注意：这里需要找到原来的项目ID来更新，但当前接口没有提供
            // 实际实现中需要额外的逻辑来处理
        }
        
        return success;
    }

    std::vector<int> getTaskIdsInProject(int projectId) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_TASK_IDS_IN_PROJECT_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        sqlite3_bind_int(stmt, 1, projectId);
        
        std::vector<int> taskIds;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            taskIds.push_back(sqlite3_column_int(stmt, 0));
        }

        sqlite3_finalize(stmt);
        return taskIds;
    }

    int getTaskCountInProject(int projectId) override {
        return executeCountQuery(SELECT_TASK_COUNT_SQL, projectId);
    }

    int getCompletedTaskCountInProject(int projectId) override {
        return executeCountQuery(SELECT_COMPLETED_TASK_COUNT_SQL, projectId);
    }

    // 统计查询
    std::vector<Project> getProjectsByCompletionRate(double minRate, double maxRate) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_PROJECTS_BY_COMPLETION_RATE_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        sqlite3_bind_double(stmt, 1, minRate);
        sqlite3_bind_double(stmt, 2, maxRate);

        auto projects = extractProjectsFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return projects;
    }

    ProjectStats getProjectStatistics(int projectId) override {
        ProjectStats stats;
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, SELECT_PROJECT_STATS_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return stats;
        }

        sqlite3_bind_int(stmt, 1, projectId);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalTasks = sqlite3_column_int(stmt, 0);
            stats.completedTasks = sqlite3_column_int(stmt, 1);
            stats.pendingTasks = sqlite3_column_int(stmt, 2);
            stats.completionRate = sqlite3_column_double(stmt, 3);
            stats.totalPomodoros = sqlite3_column_int(stmt, 4);
            
            const char* createdDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            const char* lastActivity = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            
            if (createdDate) stats.createdDate = createdDate;
            if (lastActivity) stats.lastActivity = lastActivity;
        }

        sqlite3_finalize(stmt);
        return stats;
    }

    int getTotalProjectCount() override {
        return executeCountQuery(COUNT_ALL_PROJECTS_SQL);
    }

    int getActiveProjectCount() override {
        return executeCountQuery(COUNT_ACTIVE_PROJECTS_SQL);
    }

    // 数据验证
    bool projectExists(int projectId) override {
        return executeCountQuery(CHECK_PROJECT_EXISTS_SQL, projectId) > 0;
    }

    bool isProjectNameUnique(const std::string& name, int excludeId = 0) override {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, CHECK_PROJECT_NAME_UNIQUE_SQL, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, excludeId);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count == 0;
    }

private:
    std::vector<Project> executeProjectQuery(const char* sql) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return {};
        }

        auto projects = extractProjectsFromStatement(stmt);
        sqlite3_finalize(stmt);
        
        return projects;
    }

    std::vector<Project> extractProjectsFromStatement(sqlite3_stmt* stmt) {
        std::vector<Project> projects;
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            auto project = extractProjectFromStatement(stmt);
            if (project) {
                projects.push_back(*project);
            }
        }
        
        return projects;
    }

    std::optional<Project> extractProjectFromStatement(sqlite3_stmt* stmt) {
        Project project;
        project.setId(sqlite3_column_int(stmt, 0));
        project.setName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        project.setDescription(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        project.setColorLabel(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        project.setProgress(sqlite3_column_double(stmt, 4));
        project.setTotalTasks(sqlite3_column_int(stmt, 5));
        project.setCompletedTasks(sqlite3_column_int(stmt, 6));
        
        const char* targetDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        if (targetDate) project.setTargetDate(targetDate);
        
        project.setArchived(sqlite3_column_int(stmt, 8) != 0);
        project.setCreatedDate(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)));
        
        return project;
    }

    int executeCountQuery(const char* sql, int param = 0) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return -1;
        }

        if (param != 0) {
            sqlite3_bind_int(stmt, 1, param);
        }

        int count = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count;
    }

    void updateProjectTaskCounts(int projectId) {
        int totalTasks = getTaskCountInProject(projectId);
        int completedTasks = getCompletedTaskCountInProject(projectId);
        updateTaskCounts(projectId, totalTasks, completedTasks);
    }
};

// 工厂函数
std::unique_ptr<ProjectDAO> createProjectDAO(const std::string& dbPath = "projects.db") {
    auto dao = std::make_unique<SQLiteProjectDAO>(dbPath);
    if (dao->initialize()) {
        return dao;
    }
    return nullptr;
}
