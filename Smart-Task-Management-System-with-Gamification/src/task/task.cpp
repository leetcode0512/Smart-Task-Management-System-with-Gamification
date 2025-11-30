#include "task/task.h"

// Default constructor
Task::Task()
    : id(-1), name(""), description(""), projectId(std::nullopt), completed(false),
      priority(1), dueDate(""), tags(""),
      pomodoroCount(0), estimatedPomodoros(0),
      reminderTime("") {}

// Simple create-task constructor
Task::Task(const std::string &name, const std::string &desc, std::optional<int> projectId)
    : id(-1), name(name), description(desc), projectId(projectId), completed(false),
      priority(1), dueDate(""), tags(""),
      pomodoroCount(0), estimatedPomodoros(0),
      reminderTime("") {}

// Full constructor (database load)
Task::Task(int id, const std::string &name, const std::string &desc, bool completed,
           std::optional<int> projectId, int priority, const std::string& dueDate,
           const std::string& tags, int pomodoroCount,
           int estimatedPomodoros, const std::string& reminderTime)
    : id(id), name(name), description(desc),
      projectId(projectId), completed(completed),
      priority(priority), dueDate(dueDate), tags(tags),
      pomodoroCount(pomodoroCount), estimatedPomodoros(estimatedPomodoros),
      reminderTime(reminderTime) {}

// ID
int Task::getId() const { return id; }
void Task::setId(int id) { this->id = id; }

// Basic
std::string Task::getName() const { return name; }
void Task::setName(const std::string& name) { this->name = name; }

std::string Task::getDescription() const { return description; }
void Task::setDescription(const std::string& desc) { this->description = desc; }

std::optional<int> Task::getProjectId() const { return projectId; }
void Task::setProjectId(std::optional<int> projectId) { this->projectId = projectId; }

bool Task::isCompleted() const { return completed; }
void Task::setCompleted(bool completed) { this->completed = completed; }
void Task::markCompleted() { this->completed = true; }

// Extended
int Task::getPriority() const { return priority; }
void Task::setPriority(int p) { priority = p; }

std::string Task::getDueDate() const { return dueDate; }
void Task::setDueDate(const std::string& d) { dueDate = d; }

std::string Task::getTags() const { return tags; }
void Task::setTags(const std::string& t) { tags = t; }

int Task::getPomodoroCount() const { return pomodoroCount; }
void Task::setPomodoroCount(int c) { pomodoroCount = c; }

int Task::getEstimatedPomodoros() const { return estimatedPomodoros; }
void Task::setEstimatedPomodoros(int e) { estimatedPomodoros = e; }

std::string Task::getReminderTime() const { return reminderTime; }
void Task::setReminderTime(const std::string& r) { reminderTime = r; }
