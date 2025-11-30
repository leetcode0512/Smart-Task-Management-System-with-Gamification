#ifndef STATISTICS_ANALYZER_H
#define STATISTICS_ANALYZER_H

#include <string>
#include <vector>
#include <map>
#include "../database/DatabaseManager.h"

using namespace std;

/**
 * @brief 统计分析引擎 - 提供全面的任务和用户数据统计分析
 * 
 * 负责人: Mao Jingqi (成员E)
 * 功能: 任务统计、时间分析、生产力评估、报告生成
 */
class StatisticsAnalyzer {
private:
    DatabaseManager* dbManager;
    
    // 辅助方法
    int queryInt(const string& sql);
    double queryDouble(const string& sql);
    string getCurrentDate();
    string getWeekStartDate();
    string getMonthStartDate();
    
public:
    StatisticsAnalyzer();
    ~StatisticsAnalyzer();
    
    // === 任务统计 ===
    
    /**
     * @brief 获取总完成任务数
     */
    int getTotalTasksCompleted();
    
    /**
     * @brief 获取总创建任务数
     */
    int getTotalTasksCreated();
    
    /**
     * @brief 获取任务完成率
     * @return 完成率 (0.0 - 1.0)
     */
    double getCompletionRate();
    
    // === 时间维度统计 ===
    
    /**
     * @brief 获取今天完成的任务数
     */
    int getTasksCompletedToday();
    
    /**
     * @brief 获取本周完成的任务数
     */
    int getTasksCompletedThisWeek();
    
    /**
     * @brief 获取本月完成的任务数
     */
    int getTasksCompletedThisMonth();
    
    // === 生产力分析 ===
    
    /**
     * @brief 计算平均每天完成的任务数
     */
    double getAverageTasksPerDay();
    
    /**
     * @brief 获取最近几周的任务完成趋势
     * @param weeks 周数
     * @return 每周完成任务数的向量
     */
    vector<int> getWeeklyTrends(int weeks = 4);
    
    // === 连续打卡统计 ===
    
    /**
     * @brief 获取当前连续打卡天数
     */
    int getCurrentStreak();
    
    /**
     * @brief 获取历史最长连续打卡天数
     */
    int getLongestStreak();
    
    /**
     * @brief 更新连续打卡记录
     */
    void updateStreak();
    
    // === 番茄钟统计 ===
    
    /**
     * @brief 获取总番茄钟数
     */
    int getTotalPomodoros();
    
    /**
     * @brief 获取今天完成的番茄钟数
     */
    int getPomodorosToday();
    
    // === 项目统计 ===
    
    /**
     * @brief 获取项目总数
     */
    int getTotalProjects();
    
    /**
     * @brief 获取平均项目进度
     * @return 平均进度 (0.0 - 1.0)
     */
    double getAverageProjectProgress();
    
    /**
     * @brief 获取已完成的项目数
     */
    int getCompletedProjects();
    
    // === 游戏化统计 ===
    
    /**
     * @brief 获取已解锁的成就数
     */
    int getAchievementsUnlocked();
    
    /**
     * @brief 获取已完成的挑战数
     */
    int getChallengesCompleted();
    
    // === 报告生成 ===
    
    /**
     * @brief 生成每日统计报告
     */
    string generateDailyReport();
    
    /**
     * @brief 生成每周统计报告
     */
    string generateWeeklyReport();
    
    /**
     * @brief 生成每月统计报告
     */
    string generateMonthlyReport();
    
    /**
     * @brief 生成综合统计摘要
     */
    string generateSummary();
    
    // === 热力图数据支持 ===
    
    /**
     * @brief 获取指定天数的任务完成数据
     * @param days 天数
     * @return 日期->完成数映射
     */
    map<string, int> getTaskCompletionData(int days = 90);
};

#endif // STATISTICS_ANALYZER_H
