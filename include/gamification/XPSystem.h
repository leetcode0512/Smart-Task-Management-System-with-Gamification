#ifndef XP_SYSTEM_H
#define XP_SYSTEM_H

#include <string>
#include <map>
#include "../database/DatabaseManager.h"

using namespace std;

/**
 * @brief 经验值和等级系统 - 游戏化核心组件
 * 
 * 负责人: Mao Jingqi (成员E)
 * 功能: 经验值管理、等级计算、奖励发放
 */
class XPSystem {
private:
    DatabaseManager* dbManager;
    
    // 等级配置：等级 -> 所需总经验值
    map<int, int> levelThresholds;
    
    // 等级称号
    map<int, string> levelTitles;
    
    /**
     * @brief 根据总经验值计算等级
     */
    int calculateLevel(int totalXP);
    
    /**
     * @brief 初始化等级系统配置
     */
    void initializeLevelSystem();
    
    /**
     * @brief 更新用户统计表中的等级和经验值
     */
    void updateUserStats(int totalXP, int level);
    
public:
    XPSystem();
    ~XPSystem();
    
    // === 经验值管理 ===
    
    /**
     * @brief 奖励经验值
     * @param amount 经验值数量
     * @param source 来源描述
     * @return 是否成功
     */
    bool awardXP(int amount, const string& source);
    
    /**
     * @brief 获取当前经验值（当前等级进度）
     */
    int getCurrentXP();
    
    /**
     * @brief 获取总经验值
     */
    int getTotalXP();
    
    // === 等级管理 ===
    
    /**
     * @brief 获取当前等级
     */
    int getCurrentLevel();
    
    /**
     * @brief 获取下一等级所需经验值
     */
    int getXPForNextLevel();
    
    /**
     * @brief 获取到下一等级还需要的经验值
     */
    int getXPProgressToNextLevel();
    
    /**
     * @brief 获取当前等级进度百分比
     * @return 进度 (0.0 - 1.0)
     */
    double getLevelProgress();
    
    /**
     * @brief 获取等级称号
     */
    string getLevelTitle(int level);
    
    /**
     * @brief 获取当前等级称号
     */
    string getCurrentLevelTitle();
    
    // === 经验值奖励标准 ===
    
    /**
     * @brief 根据任务优先级获取奖励经验值
     * @param priority 优先级 (0:低, 1:中, 2:高)
     */
    int getXPForTaskCompletion(int priority);
    
    /**
     * @brief 完成番茄钟获得的经验值
     */
    int getXPForPomodoro();
    
    /**
     * @brief 连续打卡奖励
     * @param days 连续天数
     */
    int getXPForStreak(int days);
    
    /**
     * @brief 完成挑战获得的经验值
     * @param rewardXP 挑战定义的奖励
     */
    int getXPForChallenge(int rewardXP);
    
    /**
     * @brief 解锁成就获得的经验值
     * @param rewardXP 成就定义的奖励
     */
    int getXPForAchievement(int rewardXP);
    
    // === 显示方法 ===
    
    /**
     * @brief 显示等级和经验值信息
     */
    string displayLevelInfo();
    
    /**
     * @brief 显示经验值进度条（ASCII艺术）
     * @param width 进度条宽度
     */
    string displayXPBar(int width = 30);
    
    /**
     * @brief 获取等级徽章emoji
     */
    string getLevelBadge(int level);
};

#endif // XP_SYSTEM_H
