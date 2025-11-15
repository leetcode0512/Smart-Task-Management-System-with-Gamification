#ifndef TASK_DAO_H
#define TASK_DAO_H

#include "common/Entities.h"
#include <vector>
#include <optional>
#include <map>

class TaskDAO {
public:
    virtual ~TaskDAO() = default;
    
    // 基础CRUD操作
    virtual bool insertTask(Task& task) = 0;
    virtual bool updateTask(const Task& task) = 0;
    virtual bool deleteTask(int taskId) = 0;
    virtual bool softDeleteTask(int taskId) = 0;
    
    // 查询操作
    virtual std::optional<Task> getTaskById(int taskId) = 0;
    virtual std::vector<Task> getAllTasks() = 0;
    virtual std::vector<Task> getTasksByStatus(TaskStatus status) = 0;
    virtual std::vector<Task> getTasksByPriority(Priority priority) = 0;
    virtual std::vector<Task> getTasksByDueDateRange(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end) = 0;
    virtual std::vector<Task> getOverdueTasks() = 0;
    virtual std::vector<Task> getTasksDueToday() = 0;
    virtual std::vector<Task> getTasksDueThisWeek() = 0;
    
    // 高级查询
    virtual std::vector<Task> getTasksByProject(int projectId) = 0;
    virtual std::vector<Task> getTasksByTag(const std::string& tag) = 0;
    virtual std::vector<Task> searchTasks(const std::string& keyword) = 0;
    virtual std::vector<Task> getTasksWithPomodoroSessions(int minSessions = 1) = 0;
    
    // 批量操作
    virtual bool updateTasksStatus(const std::vector<int>& taskIds, TaskStatus status) = 0;
    virtual bool updateTasksPriority(const std::vector<int>& taskIds, Priority priority) = 0;
    virtual bool deleteCompletedTasks() = 0;
    
    // 统计查询
    virtual int getTaskCountByStatus(TaskStatus status) = 0;
    virtual int getTaskCountByPriority(Priority priority) = 0;
    virtual int getCompletedTaskCountToday() = 0;
    virtual int getCompletedTaskCountThisWeek() = 0;
    virtual double getAverageCompletionTime() = 0;
    
    // Pomodoro相关
    virtual bool incrementPomodoroCount(int taskId) = 0;
    virtual bool setPomodoroCount(int taskId, int count) = 0;
    virtual int getTotalPomodoroCount() = 0;
    
    // 标签管理
    virtual bool addTagToTask(int taskId, const std::string& tag) = 0;
    virtual bool removeTagFromTask(int taskId, const std::string& tag) = 0;
    virtual std::vector<std::string> getTagsForTask(int taskId) = 0;
    virtual std::vector<std::string> getAllUniqueTags() = 0;
    
    // 数据验证
    virtual bool taskExists(int taskId) = 0;
    virtual bool isTaskTitleUnique(const std::string& title) = 0;
};

#endif // TASK_DAO_H

