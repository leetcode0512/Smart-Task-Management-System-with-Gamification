#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <vector>
#include <optional>
#include <string>

#include "task.h"
#include "database/DAO/TaskDAO.h"

class TaskManager {
private:
    TaskDAO* dao;          // 使用已完成的 TaskDAO
    bool ownDAO = false;   // 是否需要析构 DAO（防止重复 delete）

public:
    // 构造 & 析构
    TaskManager();
    explicit TaskManager(TaskDAO* externalDao);
    ~TaskManager();

    // 初始化（创建表）
    bool initialize();

    // ===== CRUD =====
    int createTask(const Task& task);
    std::optional<Task> getTask(int id);
    std::vector<Task> getAllTasks();

    bool updateTask(const Task& task);
    bool deleteTask(int id);

    // ===== 状态处理 =====
    bool completeTask(int id);
    std::vector<Task> getTasksByCompletion(bool completed);

    // ===== 项目功能 =====
    std::vector<Task> getTasksByProject(int projectId);
    bool assignTaskToProject(int taskId, int projectId);

    // ===== 查询功能 =====
    std::vector<Task> getOverdueTasks();
    std::vector<Task> getTodayTasks();

    // ===== 统计 =====
    int getTaskCount();
    int getCompletedTaskCount();
    double getCompletionRate();

    // ===== 番茄钟 =====
    bool addPomodoro(int taskId);
    int getPomodoroCount(int taskId);
};

#endif // TASK_MANAGER_H
