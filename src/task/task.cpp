#include "task/task.h"

// Default constructor
Task::Task()
    : id(-1), name(""), description(""), projectId(0), completed(false) {}

// Main constructor
Task::Task(const std::string &name, const std::string &desc, int projectId)
    : id(-1), name(name), description(desc), projectId(projectId), completed(false) {}

// Full constructor (used when reading from database)
Task::Task(int id, const std::string &name, const std::string &desc, bool completed, int projectId)
    : id(id), name(name), description(desc), projectId(projectId), completed(completed) {}

// ID 相关方法 ⭐ 新增
int Task::getId() const {
    return id;
}

void Task::setId(int id) {
    this->id = id;
}

// 状态管理
void Task::markCompleted() {
    completed = true;
}

bool Task::isCompleted() const {
    return completed;
}

// ⭐ 新增 setCompleted
void Task::setCompleted(bool completed) {
    this->completed = completed;
}

// Getters
std::string Task::getName() const {
    return name;
}

std::string Task::getDescription() const {
    return description;
}

int Task::getProjectId() const {  // ⭐ 新增
    return projectId;
}

// Setters ⭐ 新增
void Task::setName(const std::string& name) {
    this->name = name;
}

void Task::setDescription(const std::string& desc) {
    this->description = desc;
}

void Task::setProjectId(int projectId) {
    this->projectId = projectId;
}
