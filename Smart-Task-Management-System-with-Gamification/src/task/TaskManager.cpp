#include "task/TaskManager.h"
#include <iostream>

TaskManager::TaskManager() {
    dao = new TaskDAOImpl();
    ownDAO = true;
}

TaskManager::TaskManager(TaskDAO* externalDao) {
    dao = externalDao;
}

TaskManager::~TaskManager() {
    if (ownDAO && dao) delete dao;
}

bool TaskManager::initialize() {
    return dao->createTable();
}

int TaskManager::createTask(const Task& task) {
    return dao->insertTask(task);
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

bool TaskManager::completeTask(int id) {
    auto t = dao->getTaskById(id);
    if (!t.has_value()) return false;

    Task task = t.value();
    task.markCompleted();

    return dao->updateTask(task);
}

// Query & stats & pomodoro
std::vector<Task> TaskManager::getTasksByCompletion(bool completed) {
    return dao->getTasksByStatus(completed);
}

std::vector<Task> TaskManager::getTasksByProject(int projectId) {
    return dao->getTasksByProject(projectId);
}

bool TaskManager::assignTaskToProject(int taskId, int projectId) {
    return dao->assignTaskToProject(taskId, projectId);
}

std::vector<Task> TaskManager::getOverdueTasks() {
    return dao->getOverdueTasks();
}

std::vector<Task> TaskManager::getTodayTasks() {
    return dao->getTodayTasks();
}

int TaskManager::getTaskCount() { return dao->countAllTasks(); }
int TaskManager::getCompletedTaskCount() { return dao->countCompletedTasks(); }

double TaskManager::getCompletionRate() {
    int total = getTaskCount();
    return total == 0 ? 0 : (double)getCompletedTaskCount() / total;
}

bool TaskManager::addPomodoro(int taskId) {
    return dao->incrementPomodoro(taskId);
}

int TaskManager::getPomodoroCount(int taskId) {
    return dao->getPomodoroCount(taskId);
}
