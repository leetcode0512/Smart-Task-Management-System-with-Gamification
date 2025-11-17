#include "statistics/StatisticsAnalyzer.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <sqlite3.h>

StatisticsAnalyzer::StatisticsAnalyzer() {
    dbManager = &DatabaseManager::getInstance();
    if (!dbManager->isOpen()) {
        cerr << "⚠️  警告: 数据库未打开，StatisticsAnalyzer可能无法正常工作" << endl;
    }
}

StatisticsAnalyzer::~StatisticsAnalyzer() {
    // DatabaseManager是单例，不需要在这里删除
}

// === 辅助方法 ===

int StatisticsAnalyzer::queryInt(const string& sql) {
    if (!dbManager->isOpen()) return 0;
    
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

double StatisticsAnalyzer::queryDouble(const string& sql) {
    if (!dbManager->isOpen()) return 0.0;
    
    sqlite3* db = dbManager->getRawConnection();
    sqlite3_stmt* stmt;
    double result = 0.0;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    return result;
}

string StatisticsAnalyzer::getCurrentDate() {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << (1900 + ltm->tm_year) << "-"
       << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-"
       << setfill('0') << setw(2) << ltm->tm_mday;
    return ss.str();
}

string StatisticsAnalyzer::getWeekStartDate() {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    
    // 计算本周一的日期
    int daysToMonday = (ltm->tm_wday == 0) ? 6 : ltm->tm_wday - 1;
    time_t monday = now - (daysToMonday * 24 * 3600);
    ltm = localtime(&monday);
    
    stringstream ss;
    ss << (1900 + ltm->tm_year) << "-"
       << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-"
       << setfill('0') << setw(2) << ltm->tm_mday;
    return ss.str();
}

string StatisticsAnalyzer::getMonthStartDate() {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << (1900 + ltm->tm_year) << "-"
       << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-01";
    return ss.str();
}

// === 任务统计 ===

int StatisticsAnalyzer::getTotalTasksCompleted() {
    string sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1;";
    return queryInt(sql);
}

int StatisticsAnalyzer::getTotalTasksCreated() {
    string sql = "SELECT COUNT(*) FROM tasks;";
    return queryInt(sql);
}

double StatisticsAnalyzer::getCompletionRate() {
    int total = getTotalTasksCreated();
    if (total == 0) return 0.0;
    
    int completed = getTotalTasksCompleted();
    return (double)completed / total;
}

// === 时间维度统计 ===

int StatisticsAnalyzer::getTasksCompletedToday() {
    string today = getCurrentDate();
    string sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1 AND DATE(completed_date) = '" + today + "';";
    return queryInt(sql);
}

int StatisticsAnalyzer::getTasksCompletedThisWeek() {
    string weekStart = getWeekStartDate();
    string sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1 AND DATE(completed_date) >= '" + weekStart + "';";
    return queryInt(sql);
}

int StatisticsAnalyzer::getTasksCompletedThisMonth() {
    string monthStart = getMonthStartDate();
    string sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1 AND DATE(completed_date) >= '" + monthStart + "';";
    return queryInt(sql);
}

// === 生产力分析 ===

double StatisticsAnalyzer::getAverageTasksPerDay() {
    string sql = R"(
        SELECT COUNT(*) / (julianday('now') - julianday(MIN(created_date))) 
        FROM tasks 
        WHERE completed = 1;
    )";
    return queryDouble(sql);
}

vector<int> StatisticsAnalyzer::getWeeklyTrends(int weeks) {
    vector<int> trends;
    
    for (int i = 0; i < weeks; i++) {
        // 计算每周的起始和结束日期
        time_t now = time(nullptr);
        time_t weekStart = now - ((i + 1) * 7 * 24 * 3600);
        time_t weekEnd = now - (i * 7 * 24 * 3600);
        
        tm* startTm = localtime(&weekStart);
        tm* endTm = localtime(&weekEnd);
        
        stringstream startSs, endSs;
        startSs << (1900 + startTm->tm_year) << "-"
                << setfill('0') << setw(2) << (1 + startTm->tm_mon) << "-"
                << setfill('0') << setw(2) << startTm->tm_mday;
        
        endSs << (1900 + endTm->tm_year) << "-"
              << setfill('0') << setw(2) << (1 + endTm->tm_mon) << "-"
              << setfill('0') << setw(2) << endTm->tm_mday;
        
        string sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1 "
                    "AND DATE(completed_date) >= '" + startSs.str() + "' "
                    "AND DATE(completed_date) < '" + endSs.str() + "';";
        
        trends.push_back(queryInt(sql));
    }
    
    return trends;
}

// === 连续打卡统计 ===

int StatisticsAnalyzer::getCurrentStreak() {
    string sql = "SELECT current_streak FROM user_stats WHERE id = 1;";
    return queryInt(sql);
}

int StatisticsAnalyzer::getLongestStreak() {
    string sql = "SELECT longest_streak FROM user_stats WHERE id = 1;";
    return queryInt(sql);
}

void StatisticsAnalyzer::updateStreak() {
    if (!dbManager->isOpen()) return;
    
    string today = getCurrentDate();
    
    // 获取上次活跃日期
    string sql = "SELECT last_active_date FROM user_stats WHERE id = 1;";
    sqlite3* db = dbManager->getRawConnection();
    sqlite3_stmt* stmt;
    string lastActiveDate;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (date) lastActiveDate = date;
        }
        sqlite3_finalize(stmt);
    }
    
    // 如果今天已经更新过，直接返回
    if (lastActiveDate == today) return;
    
    int currentStreak = getCurrentStreak();
    int longestStreak = getLongestStreak();
    
    // 计算日期差
    time_t now = time(nullptr);
    tm ltm = {};
    strptime(lastActiveDate.c_str(), "%Y-%m-%d", &ltm);
    time_t lastTime = mktime(&ltm);
    int daysDiff = (now - lastTime) / (24 * 3600);
    
    // 更新连续打卡
    if (daysDiff == 1) {
        // 连续打卡
        currentStreak++;
        if (currentStreak > longestStreak) {
            longestStreak = currentStreak;
        }
    } else if (daysDiff > 1) {
        // 打卡中断
        currentStreak = 1;
    }
    
    // 更新数据库
    stringstream updateSql;
    updateSql << "UPDATE user_stats SET "
              << "current_streak = " << currentStreak << ", "
              << "longest_streak = " << longestStreak << ", "
              << "last_active_date = '" << today << "' "
              << "WHERE id = 1;";
    
    dbManager->execute(updateSql.str());
}

// === 番茄钟统计 ===

int StatisticsAnalyzer::getTotalPomodoros() {
    string sql = "SELECT SUM(pomodoro_count) FROM tasks;";
    return queryInt(sql);
}

int StatisticsAnalyzer::getPomodorosToday() {
    string today = getCurrentDate();
    string sql = "SELECT total_pomodoros FROM user_stats WHERE id = 1;";
    // 注意：这个需要配合Pomodoro模块实时更新
    return queryInt(sql);
}

// === 项目统计 ===

int StatisticsAnalyzer::getTotalProjects() {
    string sql = "SELECT COUNT(*) FROM projects WHERE archived = 0;";
    return queryInt(sql);
}

double StatisticsAnalyzer::getAverageProjectProgress() {
    string sql = "SELECT AVG(progress) FROM projects WHERE archived = 0;";
    return queryDouble(sql);
}

int StatisticsAnalyzer::getCompletedProjects() {
    string sql = "SELECT COUNT(*) FROM projects WHERE progress >= 1.0 AND archived = 0;";
    return queryInt(sql);
}

// === 游戏化统计 ===

int StatisticsAnalyzer::getAchievementsUnlocked() {
    string sql = "SELECT COUNT(*) FROM achievements WHERE unlocked = 1;";
    return queryInt(sql);
}

int StatisticsAnalyzer::getChallengesCompleted() {
    string sql = "SELECT COUNT(*) FROM challenges WHERE completed = 1;";
    return queryInt(sql);
}

// === 报告生成 ===

string StatisticsAnalyzer::generateDailyReport() {
    stringstream report;
    
    report << "\n";
    report << "═══════════════════════════════════════════════════\n";
    report << "          📊 每日统计报告\n";
    report << "═══════════════════════════════════════════════════\n";
    report << "日期: " << getCurrentDate() << "\n";
    report << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
    
    int todayTasks = getTasksCompletedToday();
    int todayPomodoros = getPomodorosToday();
    int currentStreak = getCurrentStreak();
    
    report << "✅ 今日完成任务: " << todayTasks << " 个\n";
    report << "🍅 今日番茄钟: " << todayPomodoros << " 个\n";
    report << "🔥 连续打卡: " << currentStreak << " 天\n";
    
    report << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    if (todayTasks > 0) {
        report << "💪 太棒了！继续保持！\n";
    } else {
        report << "💡 提示：今天还没有完成任务，加油！\n";
    }
    
    report << "═══════════════════════════════════════════════════\n\n";
    
    return report.str();
}

string StatisticsAnalyzer::generateWeeklyReport() {
    stringstream report;
    
    report << "\n";
    report << "═══════════════════════════════════════════════════\n";
    report << "          📈 每周统计报告\n";
    report << "═══════════════════════════════════════════════════\n";
    report << "周起始日期: " << getWeekStartDate() << "\n";
    report << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
    
    int weekTasks = getTasksCompletedThisWeek();
    double avgPerDay = weekTasks / 7.0;
    
    report << "✅ 本周完成任务: " << weekTasks << " 个\n";
    report << "📊 平均每天: " << fixed << setprecision(1) << avgPerDay << " 个\n";
    
    // 周趋势
    vector<int> trends = getWeeklyTrends(4);
    report << "\n📈 最近4周趋势:\n";
    for (size_t i = 0; i < trends.size(); i++) {
        report << "  第" << (4 - i) << "周: " << trends[i] << " 个任务\n";
    }
    
    report << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    if (weekTasks >= trends[0]) {
        report << "🎉 本周表现优秀，超过了上周！\n";
    } else {
        report << "💡 本周可以更努力一些！\n";
    }
    
    report << "═══════════════════════════════════════════════════\n\n";
    
    return report.str();
}

string StatisticsAnalyzer::generateMonthlyReport() {
    stringstream report;
    
    report << "\n";
    report << "═══════════════════════════════════════════════════\n";
    report << "          📅 每月统计报告\n";
    report << "═══════════════════════════════════════════════════\n";
    report << "月份起始: " << getMonthStartDate() << "\n";
    report << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
    
    int monthTasks = getTasksCompletedThisMonth();
    int totalTasks = getTotalTasksCompleted();
    double completionRate = getCompletionRate() * 100;
    int totalPomodoros = getTotalPomodoros();
    
    report << "✅ 本月完成任务: " << monthTasks << " 个\n";
    report << "📊 总完成任务: " << totalTasks << " 个\n";
    report << "💯 完成率: " << fixed << setprecision(1) << completionRate << "%\n";
    report << "🍅 总番茄钟数: " << totalPomodoros << " 个\n";
    
    int achievements = getAchievementsUnlocked();
    int challenges = getChallengesCompleted();
    report << "\n🎮 游戏化进展:\n";
    report << "  ⭐ 已解锁成就: " << achievements << " 个\n";
    report << "  🏆 已完成挑战: " << challenges << " 个\n";
    
    int projects = getTotalProjects();
    int completedProjects = getCompletedProjects();
    double avgProgress = getAverageProjectProgress() * 100;
    
    report << "\n📁 项目统计:\n";
    report << "  总项目数: " << projects << " 个\n";
    report << "  已完成: " << completedProjects << " 个\n";
    report << "  平均进度: " << fixed << setprecision(1) << avgProgress << "%\n";
    
    report << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    report << "🎊 本月总结：继续保持良好的工作习惯！\n";
    report << "═══════════════════════════════════════════════════\n\n";
    
    return report.str();
}

string StatisticsAnalyzer::generateSummary() {
    stringstream summary;
    
    summary << "\n";
    summary << "╔═══════════════════════════════════════════════════╗\n";
    summary << "║          🎯 统计数据总览                          ║\n";
    summary << "╚═══════════════════════════════════════════════════╝\n\n";
    
    int totalCreated = getTotalTasksCreated();
    int totalCompleted = getTotalTasksCompleted();
    double rate = getCompletionRate() * 100;
    
    summary << "📋 任务统计:\n";
    summary << "  ├─ 总创建: " << totalCreated << " 个\n";
    summary << "  ├─ 总完成: " << totalCompleted << " 个\n";
    summary << "  └─ 完成率: " << fixed << setprecision(1) << rate << "%\n\n";
    
    int todayTasks = getTasksCompletedToday();
    int weekTasks = getTasksCompletedThisWeek();
    int monthTasks = getTasksCompletedThisMonth();
    
    summary << "📆 时间维度:\n";
    summary << "  ├─ 今日: " << todayTasks << " 个\n";
    summary << "  ├─ 本周: " << weekTasks << " 个\n";
    summary << "  └─ 本月: " << monthTasks << " 个\n\n";
    
    int currentStreak = getCurrentStreak();
    int longestStreak = getLongestStreak();
    
    summary << "🔥 连续打卡:\n";
    summary << "  ├─ 当前: " << currentStreak << " 天\n";
    summary << "  └─ 最长: " << longestStreak << " 天\n\n";
    
    int projects = getTotalProjects();
    int achievements = getAchievementsUnlocked();
    
    summary << "🎮 其他统计:\n";
    summary << "  ├─ 活跃项目: " << projects << " 个\n";
    summary << "  └─ 解锁成就: " << achievements << " 个\n\n";
    
    summary << "══════════════════════════════════════════════════\n\n";
    
    return summary.str();
}

// === 热力图数据支持 ===

map<string, int> StatisticsAnalyzer::getTaskCompletionData(int days) {
    map<string, int> data;
    
    if (!dbManager->isOpen()) return data;
    
    // 查询过去N天的任务完成数据
    stringstream sql;
    sql << "SELECT DATE(completed_date) as date, COUNT(*) as count "
        << "FROM tasks "
        << "WHERE completed = 1 "
        << "AND DATE(completed_date) >= DATE('now', '-" << days << " days') "
        << "GROUP BY DATE(completed_date) "
        << "ORDER BY date;";
    
    sqlite3* db = dbManager->getRawConnection();
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* dateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int count = sqlite3_column_int(stmt, 1);
            
            if (dateStr) {
                data[string(dateStr)] = count;
            }
        }
        sqlite3_finalize(stmt);
    }
    
    return data;
}
