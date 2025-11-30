#include "project/ProjectManager.h"
#include <iostream>

ProjectManager::ProjectManager() {
    dao = new ProjectDAO();
}

ProjectManager::ProjectManager(string dbPath) {
    dao = new ProjectDAO(dbPath);
}

ProjectManager::~ProjectManager() {
    delete dao;
}

bool ProjectManager::initialize() {
    return dao->createTable();
}

int ProjectManager::createProject(const Project& project) {
    int id = dao->insert(project);
    
    if (id > 0) {
        cout << "Project created successfully! ID: " << id << endl;
    } else {
        cout << "Project creation failed!" << endl;
    }
    
    return id;
}

Project* ProjectManager::getProject(int id) {
    return dao->selectById(id);
}

vector<Project*> ProjectManager::getAllProjects() {
    return dao->selectAll();
}

vector<Project*> ProjectManager::getAllProjectsIncludingArchived() {
    return dao->selectAllIncludingArchived();
}

bool ProjectManager::updateProject(const Project& project) {
    bool success = dao->update(project);
    
    if (success) {
        cout << "Project updated successfully!" << endl;
    } else {
        cout << "Project update failed!" << endl;
    }
    
    return success;
}

bool ProjectManager::deleteProject(int id) {
    bool success = dao->deleteById(id);
    
    if (success) {
        cout << "Project archived successfully!" << endl;
    } else {
        cout << "Project archive failed!" << endl;
    }
    
    return success;
}

double ProjectManager::calculateProgress(int project_id) {
    Project* p = getProject(project_id);
    if (p != nullptr) {
        return p->getProgress();
    }
    return 0.0;
}

void ProjectManager::updateProjectProgress(int project_id) {
    Project* p = getProject(project_id);
    if (p != nullptr) {
        p->updateProgress();
        dao->update(*p);
        cout << "Project progress updated: " << (p->getProgress() * 100) << "%" << endl;
        delete p;
    }
}

int ProjectManager::getProjectCount() {
    return dao->count();
}

int ProjectManager::getActiveProjectCount() {
    return dao->countActive();
}
