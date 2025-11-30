#ifndef EXPERIENCE_DAO_H
#define EXPERIENCE_DAO_H

#include "common/entities.h"
#include <vector>
#include <map>

class ExperienceDAO {
public:
    virtual ~ExperienceDAO() = default;
    
    // 经验值管理
    virtual bool awardExperience(int userId, int amount, const std::string& source, const std::string& description = "") = 0;
    virtual bool deductExperience(int userId, int amount, const std::string& reason) = 0;
    
    // 等级管理
    virtual bool levelUpUser(int userId) = 0;
    
    // 查询操作
    virtual int getCurrentExperience(int userId) = 0;
    virtual int getCurrentLevel(int userId) = 0;
    virtual UserLevelInfo getLevelInfo(int userId) = 0;
    virtual LevelDefinition getLevelDefinition(int level) = 0;
    
    // 历史记录
    virtual bool addExperienceRecord(const ExperienceRecord& record) = 0;
    virtual std::vector<ExperienceRecord> getExperienceHistory(int userId, int limit = 50) = 0;
    virtual std::vector<ExperienceRecord> getRecentExperienceHistory(int userId, int hours = 24) = 0;
    
    // 统计查询
    virtual int getTotalExperienceEarned(int userId) = 0;
    virtual int getExperienceEarnedToday(int userId) = 0;
    virtual int getExperienceEarnedThisWeek(int userId) = 0;
    virtual std::map<std::string, int> getExperienceBySource(int userId) = 0;
    
    // 排行榜
    virtual std::vector<UserRanking> getExperienceLeaderboard(int limit = 10) = 0;
    virtual std::vector<UserRanking> getLevelLeaderboard(int limit = 10) = 0;
    virtual int getUserRankByExperience(int userId) = 0;
    virtual int getUserRankByLevel(int userId) = 0;
    
    // 等级定义管理
    virtual std::vector<LevelDefinition> getAllLevelDefinitions() = 0;
    virtual bool updateLevelDefinition(const LevelDefinition& definition) = 0;
};

#endif // EXPERIENCE_DAO_H

