#include "AchievementManager.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

// 构造函数接收 AchievementDAO 和用户ID
AchievementManager::AchievementManager(std::unique_ptr<AchievementDAO> dao, int userId) 
    : achievementDAO(std::move(dao)), currentUserId(userId) {
    initialize();
}

void AchievementManager::initialize() {
    std::cout << "成就系统初始化 (用户ID: " << currentUserId << ")\n";
    
    if (loadAchievementDefinitions() && loadUserAchievements()) {
        std::cout << "成就系统初始化完成，加载了 " 
                  << achievementDefinitions.size() << " 个成就定义\n";
    } else {
        std::cout << "成就系统初始化失败\n";
    }
}

bool AchievementManager::loadAchievementDefinitions() {
    if (!achievementDAO) {
        std::cerr << "AchievementDAO 未初始化\n";
        return false;
    }
    
    try {
        achievementDefinitions = achievementDAO->getAllAchievementDefinitions();
        std::cout << "从数据库加载了 " << achievementDefinitions.size() << " 个成就定义\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载成就定义失败: " << e.what() << "\n";
        return false;
    }
}

bool AchievementManager::loadUserAchievements() {
    if (!achievementDAO) {
        std::cerr << "AchievementDAO 未初始化\n";
        return false;
    }
    
    try {
        auto userAchievementsList = achievementDAO->getUserAchievements(currentUserId);
        userAchievements.clear();
        
        for (const auto& ua : userAchievementsList) {
            userAchievements[ua.achievement_id] = ua;
        }
        
        std::cout << "加载了用户 " << currentUserId << " 的 " 
                  << userAchievements.size() << " 个成就记录\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载用户成就失败: " << e.what() << "\n";
        return false;
    }
}

void AchievementManager::checkAllAchievements() {
    std::cout << "=== 开始检查所有成就 (用户ID: " << currentUserId << ") ===\n";
    
    checkFirstTaskAchievement();
    checkSevenDayStreakAchievement();
    checkTimeManagementAchievement();
    checkPomodoroMasterAchievement();
    
    std::cout << "=== 成就检查完成 ===\n\n";
}

void AchievementManager::checkFirstTaskAchievement() {
    int completedTasks = getCompletedTaskCount();
    std::string achievementId = "first_task"; // 假设成就ID
    
    // 检查是否已解锁
    if (achievementDAO->isAchievementUnlocked(currentUserId, achievementId)) {
        return; // 已解锁，无需检查
    }
    
    // 获取成就定义
    auto definition = achievementDAO->getAchievementDefinition(achievementId);
    if (!definition) {
        std::cerr << "成就定义不存在: " << achievementId << "\n";
        return;
    }
    
    // 计算进度
    int progress = (completedTasks >= 1) ? 100 : (completedTasks * 100);
    
    // 更新进度
    if (progress > 0) {
        achievementDAO->updateAchievementProgress(currentUserId, achievementId, progress);
        
        // 如果达到100%，解锁成就
        if (progress >= 100) {
            unlockAchievement(achievementId);
        }
    }
}

void AchievementManager::checkSevenDayStreakAchievement() {
    int currentStreak = getCurrentStreak();
    std::string achievementId = "seven_day_streak"; // 假设成就ID
    
    if (achievementDAO->isAchievementUnlocked(currentUserId, achievementId)) {
        return;
    }
    
    auto definition = achievementDAO->getAchievementDefinition(achievementId);
    if (!definition) {
        std::cerr << "成就定义不存在: " << achievementId << "\n";
        return;
    }
    
    int progress = (currentStreak >= 7) ? 100 : ((currentStreak * 100) / 7);
    
    if (progress > 0) {
        achievementDAO->updateAchievementProgress(currentUserId, achievementId, progress);
        
        if (progress >= 100) {
            unlockAchievement(achievementId);
        }
    }
}

void AchievementManager::checkTimeManagementAchievement() {
    int dailyTasks = getDailyTaskCount("today");
    std::string achievementId = "time_management_master";
    
    if (achievementDAO->isAchievementUnlocked(currentUserId, achievementId)) {
        return;
    }
    
    auto definition = achievementDAO->getAchievementDefinition(achievementId);
    if (!definition) {
        std::cerr << "成就定义不存在: " << achievementId << "\n";
        return;
    }
    
    int progress = (dailyTasks >= 10) ? 100 : ((dailyTasks * 100) / 10);
    
    if (progress > 0) {
        achievementDAO->updateAchievementProgress(currentUserId, achievementId, progress);
        
        if (progress >= 100) {
            unlockAchievement(achievementId);
        }
    }
}

void AchievementManager::checkPomodoroMasterAchievement() {
    int totalPomodoros = getTotalPomodoroCount();
    std::string achievementId = "pomodoro_master";
    
    if (achievementDAO->isAchievementUnlocked(currentUserId, achievementId)) {
        return;
    }
    
    auto definition = achievementDAO->getAchievementDefinition(achievementId);
    if (!definition) {
        std::cerr << "成就定义不存在: " << achievementId << "\n";
        return;
    }
    
    int progress = (totalPomodoros >= 20) ? 100 : ((totalPomodoros * 100) / 20);
    
    if (progress > 0) {
        achievementDAO->updateAchievementProgress(currentUserId, achievementId, progress);
        
        if (progress >= 100) {
            unlockAchievement(achievementId);
        }
    }
}

void AchievementManager::unlockAchievement(const std::string& achievementId) {
    if (!achievementDAO) {
        std::cerr << "AchievementDAO 未初始化\n";
        return;
    }
    
    try {
        if (achievementDAO->unlockAchievement(currentUserId, achievementId)) {
            // 重新加载用户成就以更新缓存
            loadUserAchievements();
            
            // 获取成就定义以显示信息
            auto definition = achievementDAO->getAchievementDefinition(achievementId);
            if (definition) {
                std::cout << "🎉 成就解锁: " << definition->name << "!\n";
                std::cout << "   " << definition->description << "\n";
                std::cout << "   +" << definition->reward_xp << " XP\n\n";
            }
        } else {
            std::cerr << "解锁成就失败: " << achievementId << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "解锁成就异常: " << e.what() << "\n";
    }
}

void AchievementManager::updateAchievementProgress(const std::string& achievementId, int progress) {
    if (!achievementDAO) {
        std::cerr << "AchievementDAO 未初始化\n";
        return;
    }
    
    try {
        achievementDAO->updateAchievementProgress(currentUserId, achievementId, progress);
    } catch (const std::exception& e) {
        std::cerr << "更新成就进度失败: " << e.what() << "\n";
    }
}

void AchievementManager::displayUnlockedAchievements() {
    if (!achievementDAO) {
        std::cerr << "AchievementDAO 未初始化\n";
        return;
    }
    
    try {
        auto unlocked = achievementDAO->getUnlockedAchievements(currentUserId);
        
        std::cout << "=== 已解锁成就 (" << unlocked.size() << "个) ===\n";
        for (const auto& userAchievement : unlocked) {
            auto definition = achievementDAO->getAchievementDefinition(userAchievement.achievement_id);
            if (definition) {
                printAchievement(*definition, &userAchievement);
            }
        }
        
        if (unlocked.empty()) {
            std::cout << "暂无已解锁成就\n";
        }
        std::cout << "==================\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "显示已解锁成就失败: " << e.what() << "\n";
    }
}

void AchievementManager::displayAllAchievements() {
    std::cout << "=== 所有成就 (" << achievementDefinitions.size() << "个) ===\n";
    
    for (const auto& definition : achievementDefinitions) {
        // 查找用户成就记录
        UserAchievement* userAchievement = nullptr;
        auto it = userAchievements.find(definition.id);
        if (it != userAchievements.end()) {
            userAchievement = &it->second;
        }
        
        printAchievement(definition, userAchievement);
    }
    
    std::cout << "================\n\n";
}

void AchievementManager::displayAchievementStatistics() {
    if (!achievementDAO) {
        std::cerr << "AchievementDAO 未初始化\n";
        return;
    }
    
    try {
        auto stats = achievementDAO->getAchievementStatistics();
        double completionRate = achievementDAO->getAchievementCompletionRate(currentUserId);
        int unlockedCount = achievementDAO->getUnlockedAchievementCount(currentUserId);
        int totalCount = achievementDAO->getTotalAchievementCount();
        
        std::cout << "=== 成就统计 ===\n";
        std::cout << "用户ID: " << currentUserId << "\n";
        std::cout << "解锁成就: " << unlockedCount << " / " << totalCount << "\n";
        std::cout << "完成率: " << std::fixed << std::setprecision(1) << (completionRate * 100) << "%\n";
        std::cout << "总成就数: " << stats.totalAchievements << "\n";
        std::cout << "已解锁: " << stats.unlockedAchievements << "\n";
        std::cout << "未解锁: " << stats.lockedAchievements << "\n";
        std::cout << "全局解锁率: " << std::fixed << std::setprecision(1) << (stats.unlockRate * 100) << "%\n";
        
        std::cout << "按类别分布:\n";
        for (const auto& [category, count] : stats.achievementsByCategory) {
            std::cout << "  " << category << ": " << count << " 个\n";
        }
        
        std::cout << "================\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "显示成就统计失败: " << e.what() << "\n";
    }
}

void AchievementManager::printAchievement(const AchievementDefinition& definition, 
                                        const UserAchievement* userAchievement) const {
    bool unlocked = userAchievement && userAchievement->unlocked;
    int progress = userAchievement ? userAchievement->progress : 0;
    
    std::cout << (unlocked ? "✅ " : "🔒 ");
    std::cout << definition.name << " - " << definition.description;
    std::cout << " [" << progress << "%]";
    
    if (unlocked && userAchievement && !userAchievement->unlocked_date.empty()) {
        std::cout << " (解锁于: " << userAchievement->unlocked_date << ")";
    }
    
    if (definition.reward_xp > 0) {
        std::cout << " +" << definition.reward_xp << "XP";
    }
    
    std::cout << "\n";
}

// 统计信息获取方法（需要队友实现其他DAO）
int AchievementManager::getCompletedTaskCount() const {
    // TODO: 需要队友实现 TaskDAO 或类似的接口
    // 临时返回模拟数据
    return 5; // 模拟5个已完成任务
}

int AchievementManager::getCurrentStreak() const {
    // TODO: 需要队友实现 UserStatsDAO 或类似的接口
    return 3; // 模拟3天连续记录
}

int AchievementManager::getDailyTaskCount(const std::string& date) const {
    // TODO: 需要队友实现 TaskDAO 的按日期统计方法
    return 2; // 模拟今日完成2个任务
}

int AchievementManager::getTotalPomodoroCount() const {
    // TODO: 需要队友实现 PomodoroDAO 或类似的接口
    return 15; // 模拟15个番茄钟
}

void AchievementManager::setCurrentUserId(int userId) {
    currentUserId = userId;
    // 切换用户时重新加载成就
    loadUserAchievements();
}

int AchievementManager::getCurrentUserId() const {
    return currentUserId;
}
