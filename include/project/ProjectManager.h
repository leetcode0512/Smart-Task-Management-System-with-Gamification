#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include "Project.h"
#include "../database/DAO/ProjectDAO.h"
#include <vector>
#include <string>

using namespace std;

class ProjectManager {
private:
    ProjectDAO* dao;

public:
    ProjectManager();
    ProjectManager(string dbPath);
    ~ProjectManager();
    
    bool initialize();
    
    int createProject(const Project& project);
    Project* getProject(int id);
    vector<Project*> getAllProjects();
    vector<Project*> getAllProjectsIncludingArchived();
    bool updateProject(const Project& project);
    bool deleteProject(int id);
    
    double calculateProgress(int project_id);
    void updateProjectProgress(int project_id);
    int getProjectCount();
    int getActiveProjectCount();
};

#endif
