#include "task/TaskManager.h"
#include <iostream>
#include <ctime>

// =====================
// 构造 & 析构
// =====================

TaskManager::TaskManager() {
    dao = new TaskDAOImpl();
    ownDAO = true;
}

TaskManager::TaskManager(TaskDAO* externalDao) {
    dao = externalDao;
}

TaskManager::~TaskManager() {
    if (ownDAO && dao) {
        delete dao;
    }
}

bool TaskManager::initialize() {
    return dao->createTable();
}

// =====================
// CRUD
// =====================

int TaskManager::createTask(const Task& task) {
    auto id = dao->insertTask(task);
    return id; // 若失败，DAO 会返回 -1
}

std::optional<Task> TaskManager::getTask(int id) {
    return dao->getTaskById(id);
}

std::vector<Task> TaskManager::getAllTasks() {
    return dao->getAllTasks();
}

bool TaskManager::updateTask(const Task& task) {
    return dao->updateTask(task);
}

bool TaskManager::deleteTask(int id) {
    return dao->deleteTask(id);
}

// =====================
// 任务完成逻辑
// =====================

bool TaskManager::completeTask(int id) {
    auto taskOpt = dao->getTaskById(id);
    if (!taskOpt.has_value()) return false;

    Task task = taskOpt.value();
    task.markCompleted();

    bool ok = dao->updateTask(task);

    // === 奖励 XP（完成任务）===
    if (ok) {
        // XPSystem::getInstance()->awardXP(20, "Task Completed!");
        std::cout << "Task " << id << " completed successfully.\n";
    }
    return ok;
}

std::vector<Task> TaskManager::getTasksByCompletion(bool completed) {
    return dao->getTasksByStatus(completed);
}

// =====================
// 项目相关
// =====================

std::vector<Task> TaskManager::getTasksByProject(int projectId) {
    return dao->getTasksByProject(projectId);
}

bool TaskManager::assignTaskToProject(int taskId, int projectId) {
    return dao->assignTaskToProject(taskId, projectId);
}

// =====================
// 查询功能
// =====================

std::vector<Task> TaskManager::getOverdueTasks() {
    return dao->getOverdueTasks();
}

std::vector<Task> TaskManager::getTodayTasks() {
    return dao->getTodayTasks();
}

// =====================
// 统计功能
// =====================

int TaskManager::getTaskCount() {
    return dao->countAllTasks();
}

int TaskManager::getCompletedTaskCount() {
    return dao->countCompletedTasks();
}

double TaskManager::getCompletionRate() {
    int total = getTaskCount();
    if (total == 0) return 0.0;

    int completed = getCompletedTaskCount();
    return (completed * 1.0) / total;
}

// =====================
// 番茄钟
// =====================

bool TaskManager::addPomodoro(int taskId) {
    return dao->incrementPomodoro(taskId);
}

int TaskManager::getPomodoroCount(int taskId) {
    return dao->getPomodoroCount(taskId);
}
