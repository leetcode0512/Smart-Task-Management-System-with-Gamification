#include "project/ProjectManager.h"
#include <iostream>

ProjectManager::ProjectManager() {
    nextId = 1;
}

ProjectManager::~ProjectManager() {
    for (auto& pair : projects) {
        delete pair.second;
    }
    projects.clear();
}

int ProjectManager::createProject(const Project& project) {
    Project* newProject = new Project(project);
    newProject->setId(nextId);
    projects[nextId] = newProject;
    
    cout << "✅ 项目创建成功！ID: " << nextId << endl;
    return nextId++;
}

Project* ProjectManager::getProject(int id) {
    if (projects.find(id) != projects.end()) {
        return projects[id];
    }
    return nullptr;
}

vector<Project*> ProjectManager::getAllProjects() {
    vector<Project*> result;
    for (auto& pair : projects) {
        if (!pair.second->isArchived()) {
            result.push_back(pair.second);
        }
    }
    return result;
}

bool ProjectManager::updateProject(const Project& project) {
    int id = project.getId();
    if (projects.find(id) != projects.end()) {
        *projects[id] = project;
        cout << "✅ 项目更新成功！" << endl;
        return true;
    }
    cout << "❌ 项目不存在！" << endl;
    return false;
}

bool ProjectManager::deleteProject(int id) {
    if (projects.find(id) != projects.end()) {
        delete projects[id];
        projects.erase(id);
        cout << "✅ 项目删除成功！" << endl;
        return true;
    }
    cout << "❌ 项目不存在！" << endl;
    return false;
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
        cout << "📊 项目进度已更新：" << (p->getProgress() * 100) << "%" << endl;
    }
}

int ProjectManager::getProjectCount() {
    return projects.size();
}