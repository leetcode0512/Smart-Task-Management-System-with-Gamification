#ifndef ACHIEVEMENT_DAO_H
#define ACHIEVEMENT_DAO_H

#include "Entities.h"
#include <vector>
#include <optional>
#include <map>
#include <string>

/**
 * @brief 成就统计信息结构
 * 包含用户的成就完成情况统计
 */
struct AchievementStats {
    int userId = 0;                              // 用户ID（明确统计归属）
    int totalAchievements = 0;                   // 总成就数量
    int unlockedAchievements = 0;                // 已解锁成就数量
    int lockedAchievements = 0;                  // 未解锁成就数量
    int inProgressAchievements = 0;              // 进行中的成就数量
    double unlockRate = 0.0;                     // 解锁率（0.0-1.0）
    std::map<std::string, int> achievementsByCategory;  // 各类别成就数量
    std::map<std::string, int> unlockedByCategory;      // 各类别已解锁数量
    
    // 默认构造函数
    AchievementStats() = default;
    
    // 带参数构造函数
    AchievementStats(int uid) : userId(uid) {}
};

/**
 * @brief 成就批量操作结果
 */
struct BatchOperationResult {
    int successCount = 0;                        // 成功操作数量
    int failedCount = 0;                         // 失败操作数量
    std::vector<std::string> failedIds;          // 失败的成就ID列表
    std::string errorMessage;                    // 错误信息
    
    bool isSuccess() const { return failedCount == 0; }
};

/**
 * @brief 成就数据访问对象接口
 * 提供成就系统的数据库操作抽象
 */
class AchievementDAO {
public:
    virtual ~AchievementDAO() = default;
    
    // ==================== 成就定义管理 ====================
    
    /**
     * @brief 插入新的成就定义
     * @param definition 成就定义对象
     * @return 成功返回 true，失败返回 false
     */
    virtual bool insertAchievementDefinition(const AchievementDefinition& definition) = 0;
    
    /**
     * @brief 更新已存在的成就定义
     * @param definition 成就定义对象（必须包含有效的 achievementId）
     * @return 成功返回 true，失败返回 false
     */
    virtual bool updateAchievementDefinition(const AchievementDefinition& definition) = 0;
    
    /**
     * @brief 删除成就定义
     * @param achievementId 成就ID
     * @return 成功返回 true，失败返回 false
     * @note 删除成就定义时应同时删除或处理相关的用户成就记录
     */
    virtual bool deleteAchievementDefinition(const std::string& achievementId) = 0;
    
    /**
     * @brief 批量插入成就定义
     * @param definitions 成就定义列表
     * @return 批量操作结果
     */
    virtual BatchOperationResult insertAchievementDefinitions(
        const std::vector<AchievementDefinition>& definitions) = 0;
    
    // ==================== 用户成就管理 ====================
    
    /**
     * @brief 解锁用户成就
     * @param userId 用户ID
     * @param achievementId 成就ID
     * @return 成功返回 true，失败返回 false
     * @note 如果成就已解锁，应返回 true
     */
    virtual bool unlockAchievement(int userId, const std::string& achievementId) = 0;
    
    /**
     * @brief 更新成就进度
     * @param userId 用户ID
     * @param achievementId 成就ID
     * @param progress 当前进度值
     * @return 成功返回 true，失败返回 false
     * @note 如果进度达到目标值，应自动解锁成就
     */
    virtual bool updateAchievementProgress(int userId, const std::string& achievementId, int progress) = 0;
    
    /**
     * @brief 批量解锁成就
     * @param userId 用户ID
     * @param achievementIds 成就ID列表
     * @return 批量操作结果
     */
    virtual BatchOperationResult unlockMultipleAchievements(
        int userId, 
        const std::vector<std::string>& achievementIds) = 0;
    
    /**
     * @brief 重置用户成就进度
     * @param userId 用户ID
     * @param achievementId 成就ID
     * @return 成功返回 true，失败返回 false
     */
    virtual bool resetAchievementProgress(int userId, const std::string& achievementId) = 0;
    
    /**
     * @brief 重置用户所有成就
     * @param userId 用户ID
     * @return 成功返回 true，失败返回 false
     */
    virtual bool resetAllAchievements(int userId) = 0;
    
    // ==================== 查询操作 ====================
    
    /**
     * @brief 获取成就定义
     * @param achievementId 成就ID
     * @return 成就定义对象，如果不存在返回 std::nullopt
     */
    virtual std::optional<AchievementDefinition> getAchievementDefinition(
        const std::string& achievementId) = 0;
    
    /**
     * @brief 获取所有成就定义
     * @return 成就定义列表
     */
    virtual std::vector<AchievementDefinition> getAllAchievementDefinitions() = 0;
    
    /**
     * @brief 获取用户的所有成就（包括已解锁和未解锁）
     * @param userId 用户ID
     * @return 用户成就列表
     */
    virtual std::vector<UserAchievement> getUserAchievements(int userId) = 0;
    
    /**
     * @brief 获取用户已解锁的成就
     * @param userId 用户ID
     * @return 已解锁成就列表
     */
    virtual std::vector<UserAchievement> getUnlockedAchievements(int userId) = 0;
    
    /**
     * @brief 获取用户未解锁的成就
     * @param userId 用户ID
     * @return 未解锁成就列表
     */
    virtual std::vector<UserAchievement> getLockedAchievements(int userId) = 0;
    
    /**
     * @brief 获取用户进行中的成就（有进度但未解锁）
     * @param userId 用户ID
     * @return 进行中的成就列表
     */
    virtual std::vector<UserAchievement> getInProgressAchievements(int userId) = 0;
    
    /**
     * @brief 分页获取用户成就
     * @param userId 用户ID
     * @param limit 每页数量
     * @param offset 偏移量
     * @return 用户成就列表
     */
    virtual std::vector<UserAchievement> getUserAchievementsPaginated(
        int userId, int limit, int offset) = 0;
    
    /**
     * @brief 获取最近解锁的成就
     * @param userId 用户ID
     * @param limit 返回数量限制
     * @return 最近解锁的成就列表（按解锁时间倒序）
     */
    virtual std::vector<UserAchievement> getRecentlyUnlockedAchievements(
        int userId, int limit = 10) = 0;
    
    // ==================== 进度查询 ====================
    
    /**
     * @brief 获取成就当前进度
     * @param userId 用户ID
     * @param achievementId 成就ID
     * @return 进度值，如果成就不存在返回 std::nullopt
     */
    virtual std::optional<int> getAchievementProgress(
        int userId, const std::string& achievementId) = 0;
    
    /**
     * @brief 检查成就是否已解锁
     * @param userId 用户ID
     * @param achievementId 成就ID
     * @return 已解锁返回 true，否则返回 false
     */
    virtual bool isAchievementUnlocked(int userId, const std::string& achievementId) = 0;
    
    /**
     * @brief 获取成就完成百分比
     * @param userId 用户ID
     * @param achievementId 成就ID
     * @return 完成百分比（0.0-1.0），如果成就不存在返回 std::nullopt
     */
    virtual std::optional<double> getAchievementCompletionPercentage(
        int userId, const std::string& achievementId) = 0;
    
    // ==================== 统计查询 ====================
    
    /**
     * @brief 获取用户已解锁成就数量
     * @param userId 用户ID
     * @return 已解锁成就数量
     */
    virtual int getUnlockedAchievementCount(int userId) = 0;
    
    /**
     * @brief 获取系统总成就数量
     * @return 总成就数量
     */
    virtual int getTotalAchievementCount() = 0;
    
    /**
     * @brief 获取用户成就完成率
     * @param userId 用户ID
     * @return 完成率（0.0-1.0）
     */
    virtual double getAchievementCompletionRate(int userId) = 0;
    
    /**
     * @brief 获取用户成就统计信息
     * @param userId 用户ID
     * @return 成就统计结构
     */
    virtual AchievementStats getAchievementStatistics(int userId) = 0;
    
    /**
     * @brief 获取全局成就统计（所有用户的汇总）
     * @return 全局统计信息
     */
    virtual std::map<std::string, int> getGlobalAchievementStatistics() = 0;
    
    /**
     * @brief 获取成就解锁排行榜
     * @param limit 返回数量限制
     * @return 用户ID和解锁数量的映射（按解锁数降序）
     */
    virtual std::vector<std::pair<int, int>> getAchievementLeaderboard(int limit = 10) = 0;
    
    // ==================== 类别管理 ====================
    
    /**
     * @brief 获取所有成就类别
     * @return 类别名称列表
     */
    virtual std::vector<std::string> getAchievementCategories() = 0;
    
    /**
     * @brief 根据类别获取成就定义
     * @param category 类别名称
     * @return 该类别的成就定义列表
     */
    virtual std::vector<AchievementDefinition> getAchievementsByCategory(
        const std::string& category) = 0;
    
    /**
     * @brief 获取用户在特定类别中的成就
     * @param userId 用户ID
     * @param category 类别名称
     * @return 该类别的用户成就列表
     */
    virtual std::vector<UserAchievement> getUserAchievementsByCategory(
        int userId, const std::string& category) = 0;
    
    /**
     * @brief 获取类别完成率
     * @param userId 用户ID
     * @param category 类别名称
     * @return 该类别的完成率（0.0-1.0）
     */
    virtual double getCategoryCompletionRate(int userId, const std::string& category) = 0;
    
    // ==================== 搜索和过滤 ====================
    
    /**
     * @brief 按难度等级搜索成就
     * @param difficulty 难度等级
     * @return 对应难度的成就列表
     */
    virtual std::vector<AchievementDefinition> getAchievementsByDifficulty(
        const std::string& difficulty) = 0;
    
    /**
     * @brief 搜索成就（按名称或描述）
     * @param keyword 搜索关键词
     * @return 匹配的成就列表
     */
    virtual std::vector<AchievementDefinition> searchAchievements(
        const std::string& keyword) = 0;
    
    // ==================== 数据验证 ====================
    
    /**
     * @brief 检查成就定义是否存在
     * @param achievementId 成就ID
     * @return 存在返回 true，否则返回 false
     */
    virtual bool achievementExists(const std::string& achievementId) = 0;
    
    /**
     * @brief 验证用户成就数据完整性
     * @param userId 用户ID
     * @return 数据完整返回 true，否则返回 false
     */
    virtual bool validateUserAchievementData(int userId) = 0;
};

#endif // ACHIEVEMENT_DAO_H
