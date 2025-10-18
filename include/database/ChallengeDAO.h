#ifndef CHALLENGE_DAO_H
#define CHALLENGE_DAO_H

#include "Entities.h"
#include <vector>
#include <optional>
#include <map>

struct ChallengeStats {
    int totalChallenges;
    int completedChallenges;
    int failedChallenges;
    double completionRate;
    int totalExperienceEarned;
    
    ChallengeStats();
};

class ChallengeDAO {
public:
    virtual ~ChallengeDAO() = default;
    
    // 挑战模板管理
    virtual bool insertChallengeTemplate(const ChallengeTemplate& templateObj) = 0;
    virtual bool updateChallengeTemplate(const ChallengeTemplate& templateObj) = 0;
    virtual bool deleteChallengeTemplate(const std::string& templateId) = 0;
    
    // 活跃挑战管理
    virtual bool createChallengeFromTemplate(const std::string& templateId, Challenge& challenge) = 0;
    virtual bool updateChallenge(const Challenge& challenge) = 0;
    virtual bool deleteChallenge(int challengeId) = 0;
    
    // 查询操作
    virtual std::optional<Challenge> getChallengeById(int challengeId) = 0;
    virtual std::vector<Challenge> getAllActiveChallenges() = 0;
    virtual std::vector<Challenge> getCompletedChallenges() = 0;
    virtual std::vector<Challenge> getChallengesByType(ChallengeType type) = 0;
    virtual std::vector<Challenge> getDailyChallenges() = 0;
    virtual std::vector<Challenge> getWeeklyChallenges() = 0;
    virtual std::vector<ChallengeTemplate> getAllChallengeTemplates() = 0;
    
    // 进度管理
    virtual bool updateChallengeProgress(int challengeId, int newProgress) = 0;
    virtual bool incrementChallengeProgress(int challengeId, int increment = 1) = 0;
    virtual bool completeChallenge(int challengeId) = 0;
    virtual bool resetChallengeProgress(int challengeId) = 0;
    
    // 时间管理
    virtual bool expireOldChallenges() = 0;
    virtual bool generateDailyChallenges() = 0;
    virtual bool generateWeeklyChallenges() = 0;
    
    // 奖励管理
    virtual bool awardChallengeReward(int challengeId) = 0;
    virtual std::vector<Challenge> getChallengeRewards(int challengeId) = 0;
    
    // 统计查询
    virtual int getCompletedChallengeCountToday() = 0;
    virtual int getCompletedChallengeCountThisWeek() = 0;
    virtual double getChallengeCompletionRate() = 0;
    virtual ChallengeStats getChallengeStatistics() = 0;
    
    // 模板查询
    virtual std::optional<ChallengeTemplate> getRandomDailyTemplate() = 0;
    virtual std::optional<ChallengeTemplate> getRandomWeeklyTemplate() = 0;
};

#endif // CHALLENGE_DAO_H