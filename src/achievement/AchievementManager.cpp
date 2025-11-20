#include "achievement/AchievementManager.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

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
        achievementDAO->loadAchievementDefinitions();
        achievementDefinitions = achievementDAO->getAllAchievementDefinitions();

        if (achievementDefinitions.empty()) {
            std::cerr << "未找到任何成就定义\n";
            return false;
        }

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
        achievementDAO->loadUserAchievements(currentUserId);
        auto userAchievementsList = achievementDAO->getUserAchievements(currentUserId);
        refreshUserAchievementCache(userAchievementsList);

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
    const std::string achievementId = "first_task";
    const auto* definition = findAchievementDefinition(achievementId);
    if (!definition) {
        std::cerr << "成就定义不存在: " << achievementId << "\n";
        return;
    }

    if (isAchievementUnlocked(achievementId)) {
        return;
    }

    const int completedTasks = getCompletedTaskCount();
    const int progressValue = std::min(definition->target_value, completedTasks);

    if (progressValue <= 0) {
        return;
    }

    if (achievementDAO->updateAchievementProgress(currentUserId, achievementId, progressValue)) {
        loadUserAchievements();
        if (progressValue >= definition->target_value) {
            unlockAchievement(achievementId);
        }
    }
}

void AchievementManager::checkSevenDayStreakAchievement() {
    const std::string achievementId = "seven_day_streak";
    const auto* definition = findAchievementDefinition(achievementId);
    if (!definition) {
        std::cerr << "成就定义不存在: " << achievementId << "\n";
        return;
    }

    if (isAchievementUnlocked(achievementId)) {
        return;
    }

    const int currentStreak = getCurrentStreak();
    const int progressValue = std::min(definition->target_value, currentStreak);

    if (progressValue <= 0) {
        return;
    }

    if (achievementDAO->updateAchievementProgress(currentUserId, achievementId, progressValue)) {
        loadUserAchievements();
        if (progressValue >= definition->target_value) {
            unlockAchievement(achievementId);
        }
    }
}

void AchievementManager::checkTimeManagementAchievement() {
    const std::string achievementId = "time_management_master";
    const auto* definition = findAchievementDefinition(achievementId);
    if (!definition) {
        std::cerr << "成就定义不存在: " << achievementId << "\n";
        return;
    }

    if (isAchievementUnlocked(achievementId)) {
        return;
    }

    const int dailyTasks = getDailyTaskCount("today");
    const int progressValue = std::min(definition->target_value, dailyTasks);

    if (progressValue <= 0) {
        return;
    }

    if (achievementDAO->updateAchievementProgress(currentUserId, achievementId, progressValue)) {
        loadUserAchievements();
        if (progressValue >= definition->target_value) {
            unlockAchievement(achievementId);
        }
    }
}

void AchievementManager::checkPomodoroMasterAchievement() {
    const std::string achievementId = "pomodoro_master";
    const auto* definition = findAchievementDefinition(achievementId);
    if (!definition) {
        std::cerr << "成就定义不存在: " << achievementId << "\n";
        return;
    }

    if (isAchievementUnlocked(achievementId)) {
        return;
    }

    const int totalPomodoros = getTotalPomodoroCount();
    const int progressValue = std::min(definition->target_value, totalPomodoros);

    if (progressValue <= 0) {
        return;
    }

    if (achievementDAO->updateAchievementProgress(currentUserId, achievementId, progressValue)) {
        loadUserAchievements();
        if (progressValue >= definition->target_value) {
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
        const auto* definition = findAchievementDefinition(achievementId);
        if (!definition) {
            std::cerr << "成就定义不存在: " << achievementId << "\n";
            return;
        }

        if (achievementDAO->unlockAchievement(currentUserId, achievementId)) {
            // 重新加载用户成就以更新缓存
            loadUserAchievements();

            std::cout << "🎉 成就解锁: " << definition->name << "!\n";
            std::cout << "   " << definition->description << "\n";
            std::cout << "   +" << definition->reward_xp << " XP\n\n";
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
        if (achievementDAO->updateAchievementProgress(currentUserId, achievementId, progress)) {
            loadUserAchievements();
        }
    } catch (const std::exception& e) {
        std::cerr << "更新成就进度失败: " << e.what() << "\n";
    }
}

void AchievementManager::updateAchievementProgress(int userId, int achievementId, int newValue) {
    if (!achievementDAO) {
        std::cerr << "AchievementDAO 未初始化\n";
        return;
    }

    try {
        const std::string key = getAchievementKeyById(achievementId);
        if (key.empty()) {
            std::cerr << "无法找到成就ID: " << achievementId << "\n";
            return;
        }

        if (achievementDAO->updateAchievementProgress(userId, key, newValue)) {
            if (userId == currentUserId) {
                loadUserAchievements();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "更新成就进度失败 (userId=" << userId
                  << ", achievementId=" << achievementId
                  << "): " << e.what() << "\n";
    }
}

void AchievementManager::incrementAchievementProgress(int userId, int achievementId, int increment) {
    if (!achievementDAO) {
        std::cerr << "AchievementDAO 未初始化\n";
        return;
    }

    if (increment <= 0) {
        // 非正增量不做任何事，避免产生奇怪的进度变化
        return;
    }

    try {
        const std::string key = getAchievementKeyById(achievementId);
        if (key.empty()) {
            std::cerr << "无法找到成就ID: " << achievementId << "\n";
            return;
        }

        int baseProgress = 0;
        if (userId == currentUserId) {
            if (auto* achievement = findUserAchievement(key)) {
                baseProgress = achievement->progress;
            }
        }

        updateAchievementProgress(userId, achievementId, baseProgress + increment);
    } catch (const std::exception& e) {
        std::cerr << "增加成就进度失败 (userId=" << userId
                  << ", achievementId=" << achievementId
                  << ", increment=" << increment
                  << "): " << e.what() << "\n";
    }
}

void AchievementManager::displayUnlockedAchievements() {
    std::cout << "=== 已解锁成就 ===\n";

    int unlockedCount = 0;
    for (const auto& [key, achievement] : userAchievements) {
        if (!achievement.unlocked) {
            continue;
        }

        const auto* definition = findAchievementDefinition(key);
        if (definition) {
            printAchievement(*definition, &achievement);
            unlockedCount++;
        }
    }

    if (unlockedCount == 0) {
        std::cout << "暂无已解锁成就\n";
    } else {
        std::cout << "共 " << unlockedCount << " 个成就\n";
    }

    std::cout << "==================\n\n";
}

void AchievementManager::displayAllAchievements() {
    std::cout << "=== 所有成就 (" << achievementDefinitions.size() << "个) ===\n";
    
    for (const auto& definition : achievementDefinitions) {
        // 查找用户成就记录
        Achievement* userAchievement = nullptr;
        auto it = userAchievements.find(definition.unlock_condition);
        if (it != userAchievements.end()) {
            userAchievement = &it->second;
        }
        
        printAchievement(definition, userAchievement);
    }
    
    std::cout << "================\n\n";
}

void AchievementManager::displayAchievementStatistics() {
    AchievementStats stats;
    stats.totalAchievements = static_cast<int>(achievementDefinitions.size());

    int unlockedCount = 0;
    int totalTarget = 0;
    int totalProgress = 0;

    for (const auto& definition : achievementDefinitions) {
        stats.achievementsByCategory[definition.category]++;
        totalTarget += definition.target_value;

        auto it = userAchievements.find(definition.unlock_condition);
        if (it != userAchievements.end()) {
            const auto& achievement = it->second;
            if (achievement.unlocked) {
                unlockedCount++;
            }
            totalProgress += std::min(achievement.progress, definition.target_value);
        }
    }

    stats.unlockedAchievements = unlockedCount;
    stats.lockedAchievements = stats.totalAchievements - unlockedCount;
    stats.unlockRate = stats.totalAchievements > 0
        ? static_cast<double>(unlockedCount) / stats.totalAchievements
        : 0.0;

    double completionRate = 0.0;
    if (totalTarget > 0) {
        completionRate = static_cast<double>(totalProgress) / totalTarget;
    }

    std::cout << "=== 成就统计 ===\n";
    std::cout << "用户ID: " << currentUserId << "\n";
    std::cout << "解锁成就: " << unlockedCount << " / " << stats.totalAchievements << "\n";
    std::cout << "完成率: " << std::fixed << std::setprecision(1) << (completionRate * 100) << "%\n";
    std::cout << "已解锁: " << stats.unlockedAchievements << "\n";
    std::cout << "未解锁: " << stats.lockedAchievements << "\n";
    std::cout << "全局解锁率: " << std::fixed << std::setprecision(1) << (stats.unlockRate * 100) << "%\n";

    std::cout << "按类别分布:\n";
    for (const auto& [category, count] : stats.achievementsByCategory) {
        std::cout << "  " << category << ": " << count << " 个\n";
    }

    std::cout << "================\n\n";
}

void AchievementManager::printAchievement(const Achievement& definition,
                                         const Achievement* userAchievement) const {
    const bool unlocked = userAchievement && userAchievement->unlocked;
    const int currentValue = userAchievement ? userAchievement->progress : 0;
    const int targetValue = std::max(1, definition.target_value);
    const double percent = std::min(100.0,
        static_cast<double>(currentValue) * 100.0 / targetValue);

    const auto previousFlags = std::cout.flags();
    const auto previousPrecision = std::cout.precision();

    std::cout << (unlocked ? "✅ " : "🔒 ");
    std::cout << definition.name << " - " << definition.description;
    std::cout << " [" << std::fixed << std::setprecision(0) << percent << "%]";

    if (unlocked && userAchievement && !userAchievement->unlocked_date.empty()) {
        std::cout << " (解锁于: " << userAchievement->unlocked_date << ")";
    }

    if (definition.reward_xp > 0) {
        std::cout << " +" << definition.reward_xp << "XP";
    }

    std::cout.flags(previousFlags);
    std::cout.precision(previousPrecision);
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
    (void)date; // 避免未使用参数警告
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

std::vector<AchievementProgress> AchievementManager::getAchievementProgress(int userId) {
    std::vector<AchievementProgress> result;

    if (!achievementDAO) {
        std::cerr << "AchievementDAO 未初始化\n";
        return result;
    }

    try {
        achievementDAO->loadUserAchievements(userId);
        auto entries = achievementDAO->getUserAchievements(userId);

        for (const auto& entry : entries) {
            AchievementProgress progress{};
            progress.achievementId = entry.id;
            progress.currentProgress = entry.progress;
            progress.targetProgress = entry.target_value;
            progress.progressPercent = entry.target_value > 0
                ? static_cast<double>(entry.progress) * 100.0 / entry.target_value
                : 0.0;
            result.push_back(progress);
        }

        if (userId == currentUserId) {
            refreshUserAchievementCache(entries);
        } else {
            achievementDAO->loadUserAchievements(currentUserId);
            refreshUserAchievementCache(achievementDAO->getUserAchievements(currentUserId));
        }
    } catch (const std::exception& e) {
        std::cerr << "获取成就进度失败 (userId=" << userId << "): " << e.what() << "\n";
    }

    return result;
}

const Achievement* AchievementManager::findAchievementDefinition(const std::string& key) const {
    auto it = std::find_if(achievementDefinitions.begin(), achievementDefinitions.end(),
        [&key](const Achievement& definition) {
            return definition.unlock_condition == key;
        });

    if (it != achievementDefinitions.end()) {
        return &(*it);
    }
    return nullptr;
}

Achievement* AchievementManager::findUserAchievement(const std::string& key) {
    auto it = userAchievements.find(key);
    if (it != userAchievements.end()) {
        return &it->second;
    }
    return nullptr;
}

bool AchievementManager::isAchievementUnlocked(const std::string& key) const {
    auto it = userAchievements.find(key);
    return it != userAchievements.end() && it->second.unlocked;
}

std::string AchievementManager::getAchievementKeyById(int achievementId) const {
    for (const auto& definition : achievementDefinitions) {
        if (definition.id == achievementId) {
            return definition.unlock_condition;
        }
    }
    return {};
}

void AchievementManager::refreshUserAchievementCache(const std::vector<Achievement>& entries) {
    userAchievements.clear();
    for (const auto& entry : entries) {
        userAchievements[entry.unlock_condition] = entry;
    }
}
