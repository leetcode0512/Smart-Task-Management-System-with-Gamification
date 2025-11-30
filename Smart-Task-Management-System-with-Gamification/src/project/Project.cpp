#include "project/Project.h"
#include <ctime>
#include <sstream>
#include <iomanip>

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
    
    time_t now = time(0);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << (1900 + ltm->tm_year) << "-" 
       << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-" 
       << setfill('0') << setw(2) << ltm->tm_mday;
    created_date = ss.str();
    updated_date = ss.str();
}

Project::Project(string name, string desc, string color) 
    : Project() {
    this->name = name;
    this->description = desc;
    this->color_label = color;
}

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
