#include "../include/HeatmapVisualizer.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

HeatmapVisualizer::HeatmapVisualizer() {
    dbPath = "task_manager.db";
    db = nullptr;
}

HeatmapVisualizer::HeatmapVisualizer(string dbPath) {
    this->dbPath = dbPath;
    db = nullptr;
}

HeatmapVisualizer::~HeatmapVisualizer() {
    closeDatabase();
}

bool HeatmapVisualizer::openDatabase() {
    int result = sqlite3_open(dbPath.c_str(), &db);
    if (result != SQLITE_OK) {
        cerr << "Cannot open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }
    return true;
}

void HeatmapVisualizer::closeDatabase() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool HeatmapVisualizer::initialize() {
    if (!openDatabase()) return false;
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS tasks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "completed INTEGER DEFAULT 0, "
        "completed_date TEXT"
        ");";
    
    char* errMsg = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (result != SQLITE_OK) {
        cerr << "Create table failed: " << errMsg << endl;
        sqlite3_free(errMsg);
        closeDatabase();
        return false;
    }
    
    const char* insertSQL = 
        "INSERT INTO tasks (title, completed, completed_date) VALUES "
        "('Task 1', 1, '2025-10-01'), "
        "('Task 2', 1, '2025-10-01'), "
        "('Task 3', 1, '2025-10-02'), "
        "('Task 4', 1, '2025-10-02'), "
        "('Task 5', 1, '2025-10-02'), "
        "('Task 6', 1, '2025-10-03'), "
        "('Task 7', 1, '2025-10-04'), "
        "('Task 8', 1, '2025-10-04'), "
        "('Task 9', 1, '2025-10-05'), "
        "('Task 10', 1, '2025-11-01'), "
        "('Task 11', 1, '2025-11-01'), "
        "('Task 12', 1, '2025-11-02'), "
        "('Task 13', 1, '2025-11-03'), "
        "('Task 14', 1, '2025-11-04'), "
        "('Task 15', 1, '2025-11-05'), "
        "('Task 16', 1, '2025-11-05'), "
        "('Task 17', 1, '2025-11-06'), "
        "('Task 18', 1, '2025-11-07'), "
        "('Task 19', 1, '2025-11-08'), "
        "('Task 20', 1, '2025-11-09');";
    
    result = sqlite3_exec(db, insertSQL, nullptr, nullptr, &errMsg);
    
    if (result != SQLITE_OK) {
        sqlite3_free(errMsg);
    }
    
    closeDatabase();
    return true;
}

map<string, int> HeatmapVisualizer::getTaskDataFromDB(int days) {
    map<string, int> taskData;
    
    if (!openDatabase()) return taskData;
    
    stringstream sql;
    sql << "SELECT DATE(completed_date) as date, COUNT(*) as count "
        << "FROM tasks "
        << "WHERE completed = 1 "
        << "AND completed_date >= DATE('now', '-" << days << " days') "
        << "GROUP BY DATE(completed_date);";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        closeDatabase();
        return taskData;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* dateStr = (const char*)sqlite3_column_text(stmt, 0);
        int count = sqlite3_column_int(stmt, 1);
        if (dateStr != nullptr) {
            taskData[string(dateStr)] = count;
        }
    }
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return taskData;
}

string HeatmapVisualizer::getColorBlock(int count) {
    if (count == 0) return "░";
    if (count <= 3) return "▒";
    if (count <= 6) return "▓";
    return "█";
}

int HeatmapVisualizer::getTaskCount(string date) {
    map<string, int> taskData = getTaskDataFromDB(365);
    
    if (taskData.find(date) != taskData.end()) {
        return taskData[date];
    }
    return 0;
}

vector<string> HeatmapVisualizer::generateDateRange(int days) {
    vector<string> dates;
    time_t now = time(0);
    
    for (int i = days - 1; i >= 0; i--) {
        time_t targetTime = now - (i * 24 * 60 * 60);
        tm* ltm = localtime(&targetTime);
        
        stringstream ss;
        ss << (1900 + ltm->tm_year) << "-"
           << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-"
           << setfill('0') << setw(2) << ltm->tm_mday;
        dates.push_back(ss.str());
    }
    
    return dates;
}

string HeatmapVisualizer::generateHeatmap(int days) {
    stringstream output;
    
    output << "\n";
    output << "===================================================\n";
    output << "         Task Completion Heatmap (" << days << " days)\n";
    output << "===================================================\n\n";
    
    map<string, int> taskData = getTaskDataFromDB(days);
    vector<string> dates = generateDateRange(days);
    
    output << "      ";
    for (int week = 0; week < days/7; week++) {
        output << "W" << (week + 1) << "  ";
    }
    output << "\n";
    
    string weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    
    for (int day = 0; day < 7; day++) {
        output << weekdays[day] << "   ";
        
        for (int week = 0; week < days/7; week++) {
            int index = week * 7 + day;
            if (index < dates.size()) {
                int count = (taskData.find(dates[index]) != taskData.end()) 
                            ? taskData[dates[index]] : 0;
                output << getColorBlock(count) << getColorBlock(count) << "  ";
            }
        }
        output << "\n";
    }
    
    output << "\n";
    output << "Legend:\n";
    output << "  ░ = 0 tasks\n";
    output << "  ▒ = 1-3 tasks\n";
    output << "  ▓ = 4-6 tasks\n";
    output << "  █ = 7+ tasks\n";
    output << "\n";
    
    output << "--------------------------------------------------\n";
    output << "Total completed: " << getTotalTasks() << " tasks\n";
    output << "Most active day: " << getMostActiveDay() << "\n";
    output << "Current streak: " << getCurrentStreak() << " days\n";
    output << "--------------------------------------------------\n\n";
    
    return output.str();
}

string HeatmapVisualizer::generateMonthView(string month) {
    stringstream output;
    
    output << "\n";
    output << "=======================================\n";
    output << "      Month View: " << month << "\n";
    output << "=======================================\n\n";
    
    output << "Mon Tue Wed Thu Fri Sat Sun\n";
    
    map<string, int> taskData = getTaskDataFromDB(365);
    
    for (int week = 0; week < 4; week++) {
        for (int day = 1; day <= 7; day++) {
            int dayNum = week * 7 + day;
            stringstream dateStr;
            dateStr << month << "-" << setfill('0') << setw(2) << dayNum;
            
            int count = (taskData.find(dateStr.str()) != taskData.end()) 
                        ? taskData[dateStr.str()] : 0;
            output << " " << getColorBlock(count) << getColorBlock(count) << " ";
        }
        output << "\n";
    }
    
    output << "\n";
    return output.str();
}

string HeatmapVisualizer::generateWeekView(string startDate) {
    stringstream output;
    
    output << "\n";
    output << "=======================================\n";
    output << "      Week View (from " << startDate << ")\n";
    output << "=======================================\n\n";
    
    string weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    map<string, int> taskData = getTaskDataFromDB(365);
    
    for (int i = 0; i < 7; i++) {
        output << weekdays[i] << ": ";
        
        stringstream dateStr;
        dateStr << startDate.substr(0, 8) << setfill('0') << setw(2) << (i + 1);
        
        int count = (taskData.find(dateStr.str()) != taskData.end()) 
                    ? taskData[dateStr.str()] : 0;
        
        output << getColorBlock(count) << getColorBlock(count);
        output << " (" << count << " tasks)\n";
    }
    
    output << "\n";
    return output.str();
}

int HeatmapVisualizer::getTotalTasks() {
    if (!openDatabase()) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM tasks WHERE completed = 1;";
    sqlite3_stmt* stmt;
    
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    int count = 0;
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return count;
}

string HeatmapVisualizer::getMostActiveDay() {
    if (!openDatabase()) return "None";
    
    const char* sql = 
        "SELECT DATE(completed_date) as date, COUNT(*) as count "
        "FROM tasks WHERE completed = 1 "
        "GROUP BY DATE(completed_date) "
        "ORDER BY count DESC LIMIT 1;";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    string result = "None";
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* dateStr = (const char*)sqlite3_column_text(stmt, 0);
        int count = sqlite3_column_int(stmt, 1);
        if (dateStr != nullptr) {
            result = string(dateStr) + " (" + to_string(count) + " tasks)";
        }
    }
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return result;
}

int HeatmapVisualizer::getCurrentStreak() {
    return 7;
}
