#ifndef TASK_H
#define TASK_H

#include <string>
#include <optional>

class Task {
private:
    int id;
    std::string name;
    std::string description;
    std::optional<int> projectId;  // ✅ 改为 optional
    bool completed;

    int priority;
    std::string dueDate;
    std::string tags;
    int pomodoroCount;
    int estimatedPomodoros;
    std::string reminderTime;

public:
    // 默认构造函数
    Task();

    // 简单创建任务构造函数（可选 projectId）
    Task(const std::string &name, const std::string &desc, std::optional<int> projectId = std::nullopt);

    // 完整构造函数（数据库读取使用）
    Task(int id, const std::string &name, const std::string &desc, bool completed,
         std::optional<int> projectId, int priority, const std::string& dueDate,
         const std::string& tags, int pomodoroCount, int estimatedPomodoros,
         const std::string& reminderTime);

    // ID
    int getId() const;
    void setId(int id);

    // Basic info
    std::string getName() const;
    void setName(const std::string& name);

    std::string getDescription() const;
    void setDescription(const std::string& desc);

    std::optional<int> getProjectId() const;
    void setProjectId(std::optional<int> projectId);

    bool isCompleted() const;
    void setCompleted(bool completed);
    void markCompleted();

    // Extended fields
    int getPriority() const;
    void setPriority(int p);

    std::string getDueDate() const;
    void setDueDate(const std::string& d);

    std::string getTags() const;
    void setTags(const std::string& t);

    int getPomodoroCount() const;
    void setPomodoroCount(int c);

    int getEstimatedPomodoros() const;
    void setEstimatedPomodoros(int e);

    std::string getReminderTime() const;
    void setReminderTime(const std::string& r);
};

#endif // TASK_H
