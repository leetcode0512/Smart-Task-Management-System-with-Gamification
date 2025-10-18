#ifndef PROJECT_DAO_H
#define PROJECT_DAO_H

#include "Entities.h"
#include <vector>
#include <optional>

struct ProjectStats {
    int totalTasks;
    int completedTasks;
    int pendingTasks;
    double completionRate;
    int totalPomodoros;
    std::chrono::system_clock::time_point createdDate;
    std::chrono::system_clock::time_point lastActivity;
    
    ProjectStats();
};

class ProjectDAO {
public:
    virtual ~ProjectDAO() = default;
    
    // 基础CRUD操作
    virtual bool insertProject(Project& project) = 0;
    virtual bool updateProject(const Project& project) = 0;
    virtual bool deleteProject(int projectId) = 0;
    virtual bool softDeleteProject(int projectId) = 0;
    
    // 查询操作
    virtual std::optional<Project> getProjectById(int projectId) = 0;
    virtual std::vector<Project> getAllProjects() = 0;
    virtual std::vector<Project> getActiveProjects() = 0;
    virtual std::vector<Project> getCompletedProjects() = 0;
    virtual std::vector<Project> getProjectsByColor(const std::string& colorCode) = 0;
    
    // 进度管理
    virtual bool updateProjectProgress(int projectId, double progress) = 0;
    virtual double calculateProjectProgress(int projectId) = 0;
    
    // 任务关联管理
    virtual bool addTaskToProject(int taskId, int projectId) = 0;
    virtual bool removeTaskFromProject(int taskId, int projectId) = 0;
    virtual bool moveTaskToProject(int taskId, int newProjectId) = 0;
    virtual std::vector<int> getTaskIdsInProject(int projectId) = 0;
    virtual int getTaskCountInProject(int projectId) = 0;
    virtual int getCompletedTaskCountInProject(int projectId) = 0;
    
    // 统计查询
    virtual std::vector<Project> getProjectsByCompletionRate(double minRate, double maxRate) = 0;
    virtual ProjectStats getProjectStatistics(int projectId) = 0;
    
    // 数据验证
    virtual bool projectExists(int projectId) = 0;
    virtual bool isProjectNameUnique(const std::string& name) = 0;
};

#endif // PROJECT_DAO_H
