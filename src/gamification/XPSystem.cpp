#include "../../include/gamification/XPSystem.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <sqlite3.h>

XPSystem::XPSystem() {
    dbManager = DatabaseManager::getInstance();
    if (!dbManager->isOpen()) {
        cerr << "⚠️  警告: 数据库未打开，XPSystem可能无法正常工作" << endl;
    }
    
    initializeLevelSystem();
}

XPSystem::~XPSystem() {
    // DatabaseManager是单例，不需要在这里删除
}

void XPSystem::initializeLevelSystem() {
    // 等级阈值配置（指数增长）
    levelThresholds[1] = 0;
    levelThresholds[2] = 100;
    levelThresholds[3] = 250;
    levelThresholds[4] = 500;
    levelThresholds[5] = 1000;
    levelThresholds[6] = 1750;
    levelThresholds[7] = 2750;
    levelThresholds[8] = 4000;
    levelThresholds[9] = 5500;
    levelThresholds[10] = 7500;
    levelThresholds[11] = 10000;
    levelThresholds[12] = 13000;
    levelThresholds[13] = 16500;
    levelThresholds[14] = 20500;
    levelThresholds[15] = 25000;
    levelThresholds[16] = 30000;
    levelThresholds[17] = 36000;
    levelThresholds[18] = 43000;
    levelThresholds[19] = 51000;
    levelThresholds[20] = 60000;
    
    // 等级称号
    levelTitles[1] = "新手";
    levelTitles[2] = "初学者";
    levelTitles[3] = "学徒";
    levelTitles[4] = "实践者";
    levelTitles[5] = "熟练者";
    levelTitles[6] = "资深者";
    levelTitles[7] = "精英";
    levelTitles[8] = "专家";
    levelTitles[9] = "大师";
    levelTitles[10] = "宗师";
    levelTitles[11] = "传奇";
    levelTitles[12] = "史诗";
    levelTitles[13] = "神话";
    levelTitles[14] = "不朽";
    levelTitles[15] = "永恒";
    levelTitles[16] = "至尊";
    levelTitles[17] = "主宰";
    levelTitles[18] = "神圣";
    levelTitles[19] = "超凡";
    levelTitles[20] = "传说";
}

int XPSystem::calculateLevel(int totalXP) {
    int level = 1;
    
    // 从高到低查找对应等级
    for (int l = 20; l >= 1; l--) {
        if (totalXP >= levelThresholds[l]) {
            level = l;
            break;
        }
    }
    
    return level;
}

void XPSystem::updateUserStats(int totalXP, int level) {
    if (!dbManager->isOpen()) return;
    
    stringstream sql;
    sql << "UPDATE user_stats SET "
        << "total_xp = " << totalXP << ", "
        << "level = " << level << " "
        << "WHERE id = 1;";
    
    dbManager->execute(sql.str());
}

// === 经验值管理 ===

bool XPSystem::awardXP(int amount, const string& source) {
    if (!dbManager->isOpen() || amount <= 0) return false;
    
    // 获取当前总经验值
    int currentTotal = getTotalXP();
    int oldLevel = getCurrentLevel();
    
    // 增加经验值
    int newTotal = currentTotal + amount;
    int newLevel = calculateLevel(newTotal);
    
    // 更新数据库
    updateUserStats(newTotal, newLevel);
    
    // 显示获得经验值的消息
    cout << "\n✨ 获得 " << amount << " 经验值! ";
    cout << "(" << source << ")\n";
    
    // 检查是否升级
    if (newLevel > oldLevel) {
        cout << "\n";
        cout << "🎉🎉🎉 恭喜升级！🎉🎉🎉\n";
        cout << "等级: " << oldLevel << " (" << getLevelTitle(oldLevel) << ") "
             << "→ " << newLevel << " (" << getLevelTitle(newLevel) << ")\n";
        cout << "继续加油！\n\n";
    }
    
    return true;
}

int XPSystem::getCurrentXP() {
    int totalXP = getTotalXP();
    int level = getCurrentLevel();
    
    // 当前等级的起始经验值
    int levelStartXP = levelThresholds[level];
    
    // 当前等级内的经验值
    return totalXP - levelStartXP;
}

int XPSystem::getTotalXP() {
    if (!dbManager->isOpen()) return 0;
    
    string sql = "SELECT total_xp FROM user_stats WHERE id = 1;";
    sqlite3* db = dbManager->getRawConnection();
    sqlite3_stmt* stmt;
    int result = 0;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    return result;
}

// === 等级管理 ===

int XPSystem::getCurrentLevel() {
    if (!dbManager->isOpen()) return 1;
    
    string sql = "SELECT level FROM user_stats WHERE id = 1;";
    sqlite3* db = dbManager->getRawConnection();
    sqlite3_stmt* stmt;
    int result = 1;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    return result;
}

int XPSystem::getXPForNextLevel() {
    int level = getCurrentLevel();
    
    if (level >= 20) {
        return levelThresholds[20]; // 已满级
    }
    
    return levelThresholds[level + 1];
}

int XPSystem::getXPProgressToNextLevel() {
    int totalXP = getTotalXP();
    int nextLevelXP = getXPForNextLevel();
    
    return nextLevelXP - totalXP;
}

double XPSystem::getLevelProgress() {
    int level = getCurrentLevel();
    
    if (level >= 20) {
        return 1.0; // 已满级
    }
    
    int totalXP = getTotalXP();
    int currentLevelXP = levelThresholds[level];
    int nextLevelXP = levelThresholds[level + 1];
    
    int xpInLevel = totalXP - currentLevelXP;
    int xpNeeded = nextLevelXP - currentLevelXP;
    
    if (xpNeeded == 0) return 1.0;
    
    return (double)xpInLevel / xpNeeded;
}

string XPSystem::getLevelTitle(int level) {
    if (levelTitles.find(level) != levelTitles.end()) {
        return levelTitles[level];
    }
    return "未知";
}

string XPSystem::getCurrentLevelTitle() {
    return getLevelTitle(getCurrentLevel());
}

// === 经验值奖励标准 ===

int XPSystem::getXPForTaskCompletion(int priority) {
    switch (priority) {
        case 0: return 10;  // 低优先级
        case 1: return 20;  // 中优先级
        case 2: return 50;  // 高优先级
        default: return 10;
    }
}

int XPSystem::getXPForPomodoro() {
    return 5; // 每个番茄钟5点经验值
}

int XPSystem::getXPForStreak(int days) {
    // 连续打卡奖励：天数 × 10
    return days * 10;
}

int XPSystem::getXPForChallenge(int rewardXP) {
    // 直接返回挑战定义的奖励
    return rewardXP;
}

int XPSystem::getXPForAchievement(int rewardXP) {
    // 直接返回成就定义的奖励
    return rewardXP;
}

// === 显示方法 ===

string XPSystem::displayLevelInfo() {
    stringstream info;
    
    int level = getCurrentLevel();
    int totalXP = getTotalXP();
    int currentXP = getCurrentXP();
    string title = getCurrentLevelTitle();
    
    info << "\n";
    info << "╔═══════════════════════════════════════════════════╗\n";
    info << "║          " << getLevelBadge(level) << " 等级信息 " << getLevelBadge(level) << "                           ║\n";
    info << "╚═══════════════════════════════════════════════════╝\n\n";
    
    info << "等级: " << level << " (" << title << ")\n";
    info << "总经验值: " << totalXP << " XP\n";
    
    if (level < 20) {
        int nextLevelXP = getXPForNextLevel();
        int needed = getXPProgressToNextLevel();
        double progress = getLevelProgress() * 100;
        
        info << "当前进度: " << currentXP << " / " << (nextLevelXP - levelThresholds[level]) << " XP\n";
        info << "距离下级: " << needed << " XP\n";
        info << "进度: " << fixed << setprecision(1) << progress << "%\n";
        
        info << "\n" << displayXPBar(30) << "\n";
    } else {
        info << "\n🏆 恭喜！您已达到最高等级！\n";
    }
    
    info << "\n═══════════════════════════════════════════════════\n\n";
    
    return info.str();
}

string XPSystem::displayXPBar(int width) {
    double progress = getLevelProgress();
    int filled = (int)(progress * width);
    
    stringstream bar;
    bar << "[";
    
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            bar << "█";
        } else {
            bar << "░";
        }
    }
    
    bar << "] " << fixed << setprecision(1) << (progress * 100) << "%";
    
    return bar.str();
}

string XPSystem::getLevelBadge(int level) {
    if (level >= 20) return "👑";
    if (level >= 15) return "💎";
    if (level >= 10) return "🏆";
    if (level >= 5) return "⭐";
    return "🌟";
}
