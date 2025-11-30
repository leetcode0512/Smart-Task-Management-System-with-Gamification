#ifndef DAO_FACTORY_H
#define DAO_FACTORY_H

#include "TaskDAO.h"
#include "ProjectDAO.h"
#include "ReminderDAO.h"
#include "AchievementDAO.h"
#include "SettingsDAO.h"
#include "ExperienceDAO.h"
#include "StatisticsDAO.h"
#include <memory>

class DAOFactory {
public:
    virtual ~DAOFactory() = default;
    
    // DAO创建方法
    virtual std::unique_ptr<TaskDAO> createTaskDAO() = 0;
    virtual std::unique_ptr<ProjectDAO> createProjectDAO() = 0;
    virtual std::unique_ptr<ReminderDAO> createReminderDAO() = 0;
    virtual std::unique_ptr<AchievementDAO> createAchievementDAO() = 0;
    virtual std::unique_ptr<SettingsDAO> createSettingsDAO() = 0;
    virtual std::unique_ptr<ExperienceDAO> createExperienceDAO() = 0;
    virtual std::unique_ptr<StatisticsDAO> createStatisticsDAO() = 0;
    
    // 数据库状态
    virtual bool isDatabaseReady() = 0;
    virtual bool initializeDatabase() = 0;
    
    // 工厂实例
    static std::unique_ptr<DAOFactory> createFactory();
};

#endif // DAO_FACTORY_H
