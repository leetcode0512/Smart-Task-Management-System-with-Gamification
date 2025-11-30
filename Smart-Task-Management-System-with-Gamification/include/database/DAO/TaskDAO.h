#ifndef TASKDAO_H
#define TASKDAO_H
#include <sqlite3.h>
#include <vector>
#include <optional>
#include <string>
#include "task/task.h"

class TaskDAO {

public:
    virtual ~TaskDAO() = default;
    
    // 表管理
    virtual bool createTable() = 0;
    
    // CRUD 操作
    virtual int insertTask(const Task& task) = 0;
    virtual std::optional<Task> getTaskById(int id) = 0;
    virtual std::vector<Task> getAllTasks() = 0;
    virtual bool updateTask(const Task& task) = 0;
    virtual bool deleteTask(int id) = 0;
    
    // 查询操作
    virtual std::vector<Task> getTasksByStatus(bool completed) = 0;
    virtual std::vector<Task> getTasksByProject(int projectId) = 0;
    virtual std::vector<Task> getOverdueTasks() = 0;
    virtual std::vector<Task> getTodayTasks() = 0;
    
    // 统计操作
    virtual int countAllTasks() = 0;
    virtual int countCompletedTasks() = 0;
    
    // 项目分配
    virtual bool assignTaskToProject(int taskId, int projectId) = 0;
    
    // 番茄钟
    virtual bool incrementPomodoro(int taskId) = 0;
    virtual int getPomodoroCount(int taskId) = 0;
};

// SQLite具体实现类
class TaskDAOImpl : public TaskDAO {
private:
    std::string databasePath;
    
    // 数据库连接辅助方法
    sqlite3* getDatabaseConnection();
    bool executeSQL(const std::string& sql);
    
public:
    TaskDAOImpl(const std::string& dbPath = "task_manager.db");
    virtual ~TaskDAOImpl() = default;
    
    bool createTable() override;
    int insertTask(const Task& task) override;
    std::optional<Task> getTaskById(int id) override;
    std::vector<Task> getAllTasks() override;
    bool updateTask(const Task& task) override;
    bool deleteTask(int id) override;
    
    std::vector<Task> getTasksByStatus(bool completed) override;
    std::vector<Task> getTasksByProject(int projectId) override;
    std::vector<Task> getOverdueTasks() override;
    std::vector<Task> getTodayTasks() override;
    
    int countAllTasks() override;
    int countCompletedTasks() override;
    
    bool assignTaskToProject(int taskId, int projectId) override;
    
    bool incrementPomodoro(int taskId) override;
    int getPomodoroCount(int taskId) override;
};

#endif // TASKDAO_H

