#include "task.h"

Task::Task(const std::string &n, const std::string &desc)
    : name(n), description(desc), completed(false) {}

void Task::markCompleted() {
    completed = true;
}

bool Task::isCompleted() const {
    return completed;
}

std::string Task::getName() const {
    return name;
}

std::string Task::getDescription() const {
    return description;
}
