#include "project/Project.h"
#include <ctime>
#include <sstream>

// 构造函数
Project::Project() {
    id = 0;
    name = "";
    description = "";
    color_label = "";
    progress = 0.0;
    total_tasks = 0;
    completed_tasks = 0;
    target_date = "";
    archived = false;
    
    // 获取当前时间
    time_t now = time(0);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << (1900 + ltm->tm_year) << "-" 
       << (1 + ltm->tm_mon) << "-" 
       << ltm->tm_mday;
    created_date = ss.str();
}

Project::Project(string name, string desc, string color) 
    : Project() {
    this->name = name;
    this->description = desc;
    this->color_label = color;
}

// Getter实现
int Project::getId() const { return id; }
string Project::getName() const { return name; }
string Project::getDescription() const { return description; }
string Project::getColorLabel() const { return color_label; }
double Project::getProgress() const { return progress; }
int Project::getTotalTasks() const { return total_tasks; }
int Project::getCompletedTasks() const { return completed_tasks; }
string Project::getTargetDate() const { return target_date; }
bool Project::isArchived() const { return archived; }
string Project::getCreatedDate() const { return created_date; }

// Setter实现
void Project::setId(int id) { this->id = id; }
void Project::setName(string name) { this->name = name; }
void Project::setDescription(string desc) { this->description = desc; }
void Project::setColorLabel(string color) { this->color_label = color; }
void Project::setProgress(double progress) { this->progress = progress; }
void Project::setTotalTasks(int total) { this->total_tasks = total; }
void Project::setCompletedTasks(int completed) { this->completed_tasks = completed; }
void Project::setTargetDate(string date) { this->target_date = date; }
void Project::setArchived(bool archived) { this->archived = archived; }
void Project::setCreatedDate(string date) { this->created_date = date; }

// 业务方法
void Project::updateProgress() {
    if (total_tasks > 0) {
        progress = (double)completed_tasks / total_tasks;
    } else {
        progress = 0.0;
    }
}

bool Project::isCompleted() const {
    return total_tasks > 0 && completed_tasks >= total_tasks;
}