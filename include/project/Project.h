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

public:
    // 构造函数
    Project();
    Project(string name, string desc, string color);
    
    // Getter
    int getId() const;
    string getName() const;
    string getDescription() const;
    string getColorLabel() const;
    double getProgress() const;
    int getTotalTasks() const;
    int getCompletedTasks() const;
    string getTargetDate() const;
    bool isArchived() const;
    string getCreatedDate() const;
    
    // Setter
    void setId(int id);
    void setName(string name);
    void setDescription(string desc);
    void setColorLabel(string color);
    void setProgress(double progress);
    void setTotalTasks(int total);
    void setCompletedTasks(int completed);
    void setTargetDate(string date);
    void setArchived(bool archived);
    void setCreatedDate(string date);
    
    // 业务方法
    void updateProgress();
    bool isCompleted() const;
};

#endif