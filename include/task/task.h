#ifndef TASK_H
#define TASK_H

#include <string>

class Task {
private:
    int id;                    // ⭐ 添加 ID 字段
    std::string name;
    std::string description;
    bool completed;
    int projectId;

public:
    // 构造函数
    Task(const std::string &name, const std::string &desc, int projectId = 0);
    Task(int id, const std::string &name, const std::string &desc, bool completed, int projectId = 0);  // ⭐ 新增

    // ID 相关方法 ⭐ 新增
    int getId() const;
    void setId(int id);

    // 状态管理
    void markCompleted();
    bool isCompleted() const;

    // Getters
    std::string getName() const;
    std::string getDescription() const;
    int getProjectId() const;  // ⭐ 新增

    // Setters ⭐ 新增
    void setName(const std::string& name);
    void setDescription(const std::string& desc);
    void setProjectId(int projectId);
};

#endif // TASK_H
