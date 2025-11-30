#ifndef STATISTICS_DAO_H
#define STATISTICS_DAO_H

#include "common/entities.h"
#include <vector>
#include <map>
#include <chrono>

struct CompletionTrend {
    std::vector<std::pair<std::chrono::system_clock::time_point, int>> dailyCompletions;
    double trendSlope;
    bool isImproving;
    
    CompletionTrend();
};

struct ProductivityTrend {
    std::vector<std::pair<std::chrono::system_clock::time_point, double>> dailyProductivity;
    double averageProductivity;
    bool isImproving;
    
    ProductivityTrend();
};

struct ChallengeCompletionStats {
    int dailyChallengesCompleted;
    int weeklyChallengesCompleted;
    int totalChallengesCompleted;
    double dailyChallengeCompletionRate;
    double weeklyChallengeCompletionRate;
    
    ChallengeCompletionStats();
};

struct AchievementUnlockStats {
    int totalUnlocked;
    int totalLocked;
    double unlockRate;
    std::map<std::string, int> unlocksByCategory;
    
    AchievementUnlockStats();
};

class StatisticsDAO {
public:
    virtual ~StatisticsDAO() = default;
    
    // 任务完成统计
    virtual DailyCompletionStats getDailyCompletionStats(
        const std::chrono::system_clock::time_point& date) = 0;
    virtual std::vector<DailyCompletionStats> getWeeklyCompletionStats(
        const std::chrono::system_clock::time_point& startDate) = 0;
    virtual std::vector<DailyCompletionStats> getMonthlyCompletionStats(int year, int month) = 0;
    
    // 热力图数据
    virtual std::vector<HeatmapData> getHeatmapData(int days = 90) = 0;
    virtual std::map<std::chrono::system_clock::time_point, int> getCompletionCountByDate(
        const std::chrono::system_clock::time_point& startDate,
        const std::chrono::system_clock::time_point& endDate) = 0;
    
    // 生产力分析
    virtual ProductivityReport generateProductivityReport(
        const std::chrono::system_clock::time_point& startDate,
        const std::chrono::system_clock::time_point& endDate) = 0;
    
    // Pomodoro统计
    virtual PomodoroStatistics getPomodoroStatistics(
        const std::chrono::system_clock::time_point& startDate,
        const std::chrono::system_clock::time_point& endDate) = 0;
    
    // 趋势分析
    virtual CompletionTrend getCompletionTrend(int days = 30) = 0;
    virtual ProductivityTrend getProductivityTrend(int days = 30) = 0;
    
    // streaks 统计
    virtual int getCurrentCompletionStreak() = 0;
    virtual int getLongestCompletionStreak() = 0;
    virtual std::vector<StreakRecord> getStreakHistory() = 0;
    
    // 挑战和成就统计
    virtual ChallengeCompletionStats getChallengeCompletionStats() = 0;
    virtual AchievementUnlockStats getAchievementUnlockStats() = 0;
    
    // 综合统计
    virtual std::map<std::string, int> getOverallStatistics() = 0;
};

#endif // STATISTICS_DAO_H


