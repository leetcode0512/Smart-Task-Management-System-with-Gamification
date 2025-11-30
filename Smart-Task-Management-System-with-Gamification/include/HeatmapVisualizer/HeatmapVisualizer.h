#ifndef HEATMAP_VISUALIZER_H
#define HEATMAP_VISUALIZER_H

#include <string>
#include <map>
#include <vector>
#include <sqlite3.h>

using namespace std;

class HeatmapVisualizer {
private:
    sqlite3* db;
    string dbPath;
    
    bool openDatabase();
    void closeDatabase();
    
    string getColorBlock(int count);
    int getTaskCount(string date);
    map<string, int> getTaskDataFromDB(int days);
    vector<string> generateDateRange(int days);
    
public:
    HeatmapVisualizer();
    HeatmapVisualizer(string dbPath);
    ~HeatmapVisualizer();
    
    bool initialize();
    
    string generateHeatmap(int days = 90);
    string generateMonthView(string month);
    string generateWeekView(string startDate);
    
    int getTotalTasks();
    string getMostActiveDay();
    int getCurrentStreak();
};

#endif
