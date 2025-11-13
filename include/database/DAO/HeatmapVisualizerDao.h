#ifndef HEATMAP_VISUALIZER_DAO_H
#define HEATMAP_VISUALIZER_DAO_H

#include "HeatmapVisualizer.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

class HeatmapVisualizerDAO {
private:
    string dataFilePath;
    
    // 数据序列化方法
    string serializeTaskData(const map<string, int>& taskData);
    map<string, int> deserializeTaskData(const string& data);
    
    // 文件操作
    bool fileExists(const string& filename);
    string readFile(const string& filename);
    bool writeFile(const string& filename, const string& content);

public:
    HeatmapVisualizerDAO();
    explicit HeatmapVisualizerDAO(const string& filePath);
    
    // 数据持久化操作
    bool saveHeatmapData(const HeatmapVisualizer& visualizer, const string& identifier = "default");
    bool loadHeatmapData(HeatmapVisualizer& visualizer, const string& identifier = "default");
    
    // 批量操作
    bool saveMultipleVisualizers(const vector<HeatmapVisualizer>& visualizers, 
                                const vector<string>& identifiers);
    vector<HeatmapVisualizer> loadMultipleVisualizers(const vector<string>& identifiers);
    
    // 数据统计和查询
    vector<string> getAllSavedIdentifiers();
    bool deleteHeatmapData(const string& identifier = "default");
    bool heatmapDataExists(const string& identifier = "default");
    
    // 数据导出导入
    bool exportToCSV(const HeatmapVisualizer& visualizer, const string& csvFilePath);
    bool importFromCSV(HeatmapVisualizer& visualizer, const string& csvFilePath);
    
    // 备份和恢复
    bool createBackup(const string& backupIdentifier);
    bool restoreFromBackup(const string& backupIdentifier);
    
    // 工具方法
    static string generateBackupName();
    static bool validateDate(const string& date);
};

#endif // HEATMAP_VISUALIZER_DAO_H
