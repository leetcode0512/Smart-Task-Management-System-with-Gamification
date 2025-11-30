#ifndef PROJECT_H
#define PROJECT_H

#include <string>
using namespace std;

class Project {
private:
    int id;
    string name;
    string description;
    string color_label;
    double progress;
    int total_tasks;
    int completed_tasks;
    string target_date;
    bool archived;
    string created_date;
    string updated_date;

public:
    Project();
    Project(string name, string desc, string color);
    
    // get methods
    int getId() const { return id; }
    string getName() const { return name; }
    string getDescription() const { return description; }
    string getColorLabel() const { return color_label; }
    double getProgress() const { return progress; }
    int getTotalTasks() const { return total_tasks; }
    int getCompletedTasks() const { return completed_tasks; }
    string getTargetDate() const { return target_date; }
    bool isArchived() const { return archived; }
    string getCreatedDate() const { return created_date; }
    string getUpdatedDate() const { return updated_date; }
    
    // set methods
    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setDescription(string desc) { this->description = desc; }
    void setColorLabel(string color) { this->color_label = color; }
    void setProgress(double progress) { this->progress = progress; }
    void setTotalTasks(int total) { this->total_tasks = total; }
    void setCompletedTasks(int completed) { this->completed_tasks = completed; }
    void setTargetDate(string date) { this->target_date = date; }
    void setArchived(bool archived) { this->archived = archived; }
    void setCreatedDate(string date) { this->created_date = date; }
    void setUpdatedDate(string date) { this->updated_date = date; }
    
    void updateProgress();
    bool isCompleted() const;
};

#endif
