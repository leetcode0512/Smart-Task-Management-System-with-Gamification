#include "../../include/HeatmapVisualizer/HeatmapVisualizer.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

using namespace std;

HeatmapVisualizer::HeatmapVisualizer() {
    // 初始化
}

// === 私有辅助方法 ===

string HeatmapVisualizer::getColorBlock(int count) {
    // ANSI颜色码
    const string COLOR_RESET = "\033[0m";
    const string COLOR_GRAY = "\033[90m";    // 灰色
    const string COLOR_GREEN = "\033[32m";   // 绿色
    const string COLOR_YELLOW = "\033[33m";  // 黄色
    const string COLOR_RED = "\033[31m";     // 红色
    
    if (count == 0) {
        return COLOR_GRAY + "░░" + COLOR_RESET;
    } else if (count <= 3) {
        return COLOR_GREEN + "▒▒" + COLOR_RESET;
    } else if (count <= 6) {
        return COLOR_YELLOW + "▓▓" + COLOR_RESET;
    } else {
        return COLOR_RED + "██" + COLOR_RESET;
    }
}

int HeatmapVisualizer::getTaskCount(string date) {
    if (taskData.find(date) != taskData.end()) {
        return taskData[date];
    }
    return 0;
}

vector<string> HeatmapVisualizer::generateDateRange(int days) {
    vector<string> dates;
    time_t now = time(nullptr);
    
    for (int i = days - 1; i >= 0; i--) {
        time_t targetTime = now - (i * 24 * 3600);
        tm* ltm = localtime(&targetTime);
        
        stringstream ss;
        ss << (1900 + ltm->tm_year) << "-"
           << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-"
           << setfill('0') << setw(2) << ltm->tm_mday;
        
        dates.push_back(ss.str());
    }
    
    return dates;
}

// === 公共方法 ===

void HeatmapVisualizer::addTaskData(string date, int count) {
    taskData[date] = count;
}

string HeatmapVisualizer::generateHeatmap(int days) {
    stringstream heatmap;
    
    heatmap << "\n";
    heatmap << "═══════════════════════════════════════════════════\n";
    heatmap << "         📊 " << days << "天任务完成热力图\n";
    heatmap << "═══════════════════════════════════════════════════\n\n";
    
    vector<string> dateRange = generateDateRange(days);
    
    // 计算需要显示的周数
    int weeks = (days + 6) / 7;
    
    // 表头 - 周数标签
    heatmap << "      ";
    for (int w = 1; w <= min(weeks, 13); w++) {
        heatmap << " W" << setw(2) << setfill(' ') << w << " ";
    }
    heatmap << "\n";
    
    // 星期标签
    string weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    
    // 获取第一天是星期几
    time_t firstTime = time(nullptr) - ((days - 1) * 24 * 3600);
    tm* firstTm = localtime(&firstTime);
    int startWeekday = (firstTm->tm_wday == 0) ? 6 : firstTm->tm_wday - 1; // 转换为周一开始
    
    // 生成热力图矩阵
    for (int day = 0; day < 7; day++) {
        heatmap << weekdays[day] << "   ";
        
        int currentDay = (day - startWeekday + 7) % 7;
        
        for (int week = 0; week < min(weeks, 13); week++) {
            int dateIndex = week * 7 + currentDay;
            
            if (dateIndex < dateRange.size()) {
                string date = dateRange[dateIndex];
                int count = getTaskCount(date);
                heatmap << getColorBlock(count) << " ";
            } else {
                heatmap << "   ";
            }
        }
        
        heatmap << "\n";
    }
    
    // 图例
    heatmap << "\n图例:\n";
    heatmap << "  " << getColorBlock(0) << " = 0任务\n";
    heatmap << "  " << getColorBlock(1) << " = 1-3任务\n";
    heatmap << "  " << getColorBlock(4) << " = 4-6任务\n";
    heatmap << "  " << getColorBlock(7) << " = 7+任务\n";
    
    // 统计信息
    heatmap << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    heatmap << "📈 总计完成: " << getTotalTasks() << " 个任务\n";
    
    string mostActive = getMostActiveDay();
    if (!mostActive.empty()) {
        heatmap << "🔥 最活跃日期: " << mostActive << " ("
                << getTaskCount(mostActive) << "个任务)\n";
    }
    
    int streak = getCurrentStreak();
    if (streak > 0) {
        heatmap << "⚡ 当前连续: " << streak << " 天\n";
    }
    
    heatmap << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    heatmap << "═══════════════════════════════════════════════════\n\n";
    
    return heatmap.str();
}

string HeatmapVisualizer::generateMonthView(string month) {
    stringstream monthView;
    
    monthView << "\n";
    monthView << "═══════════════════════════════════════════════════\n";
    monthView << "         📅 " << month << " 月视图\n";
    monthView << "═══════════════════════════════════════════════════\n\n";
    
    // 解析月份字符串 (格式: YYYY-MM)
    int year, mon;
    sscanf(month.c_str(), "%d-%d", &year, &mon);
    
    // 获取该月的第一天和最后一天
    tm firstDay = {};
    firstDay.tm_year = year - 1900;
    firstDay.tm_mon = mon - 1;
    firstDay.tm_mday = 1;
    mktime(&firstDay);
    
    int firstWeekday = (firstDay.tm_wday == 0) ? 6 : firstDay.tm_wday - 1;
    
    // 获取该月天数
    tm lastDay = firstDay;
    lastDay.tm_mon++;
    lastDay.tm_mday = 0;
    mktime(&lastDay);
    int daysInMonth = lastDay.tm_mday + 1;
    
    // 表头
    monthView << "  Mon  Tue  Wed  Thu  Fri  Sat  Sun\n";
    monthView << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    // 前导空格
    for (int i = 0; i < firstWeekday; i++) {
        monthView << "     ";
    }
    
    // 日期格网
    for (int day = 1; day <= daysInMonth; day++) {
        stringstream dateStr;
        dateStr << year << "-" << setfill('0') << setw(2) << mon << "-"
                << setfill('0') << setw(2) << day;
        
        int count = getTaskCount(dateStr.str());
        monthView << " " << getColorBlock(count) << " ";
        
        int currentWeekday = (firstWeekday + day - 1) % 7;
        if (currentWeekday == 6 && day != daysInMonth) {
            monthView << "\n";
        }
    }
    
    monthView << "\n\n";
    monthView << "图例: " << getColorBlock(0) << "=0  "
              << getColorBlock(1) << "=1-3  "
              << getColorBlock(4) << "=4-6  "
              << getColorBlock(7) << "=7+\n";
    
    monthView << "═══════════════════════════════════════════════════\n\n";
    
    return monthView.str();
}

string HeatmapVisualizer::generateWeekView(string startDate) {
    stringstream weekView;
    
    weekView << "\n";
    weekView << "═══════════════════════════════════════════════════\n";
    weekView << "         📆 周视图 (从 " << startDate << ")\n";
    weekView << "═══════════════════════════════════════════════════\n\n";
    
    // 解析起始日期
    tm start = {};
    sscanf(startDate.c_str(), "%d-%d-%d", &start.tm_year, &start.tm_mon, &start.tm_mday);
    start.tm_year -= 1900;
    start.tm_mon -= 1;
    time_t startTime = mktime(&start);
    
    string weekdays[] = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
    
    weekView << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    for (int i = 0; i < 7; i++) {
        time_t dayTime = startTime + (i * 24 * 3600);
        tm* dayTm = localtime(&dayTime);
        
        stringstream dateStr;
        dateStr << (1900 + dayTm->tm_year) << "-"
                << setfill('0') << setw(2) << (1 + dayTm->tm_mon) << "-"
                << setfill('0') << setw(2) << dayTm->tm_mday;
        
        int count = getTaskCount(dateStr.str());
        
        weekView << weekdays[i] << " (" << dateStr.str() << "): "
                 << getColorBlock(count) << " " << count << " 个任务\n";
    }
    
    weekView << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    weekView << "═══════════════════════════════════════════════════\n\n";
    
    return weekView.str();
}

// === 统计信息 ===

int HeatmapVisualizer::getTotalTasks() {
    int total = 0;
    for (const auto& pair : taskData) {
        total += pair.second;
    }
    return total;
}

string HeatmapVisualizer::getMostActiveDay() {
    if (taskData.empty()) return "";
    
    string mostActiveDate;
    int maxCount = 0;
    
    for (const auto& pair : taskData) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            mostActiveDate = pair.first;
        }
    }
    
    return mostActiveDate;
}

int HeatmapVisualizer::getCurrentStreak() {
    if (taskData.empty()) return 0;
    
    int streak = 0;
    time_t now = time(nullptr);
    
    // 从今天往前推，统计连续有任务的天数
    for (int i = 0; i < 365; i++) {
        time_t dayTime = now - (i * 24 * 3600);
        tm* dayTm = localtime(&dayTime);
        
        stringstream dateStr;
        dateStr << (1900 + dayTm->tm_year) << "-"
                << setfill('0') << setw(2) << (1 + dayTm->tm_mon) << "-"
                << setfill('0') << setw(2) << dayTm->tm_mday;
        
        if (getTaskCount(dateStr.str()) > 0) {
            streak++;
        } else {
            break;
        }
    }
    
    return streak;
}
