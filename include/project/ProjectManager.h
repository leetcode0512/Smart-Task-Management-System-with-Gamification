#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include "Project.h"
#include <vector>
#include <map>

class ProjectManager {
private:
    // 暂时用map存储，后续替换成数据库
    map<int, Project*> projects;
    int nextId;

public:
    ProjectManager();
    ~ProjectManager();
    
    // CRUD操作
    int createProject(const Project& project);
    Project* getProject(int id);
    vector<Project*> getAllProjects();
    bool updateProject(const Project& project);
    bool deleteProject(int id);
    
    // 业务功能
    double calculateProgress(int project_id);
    void updateProjectProgress(int project_id);
    int getProjectCount();
};

#endif