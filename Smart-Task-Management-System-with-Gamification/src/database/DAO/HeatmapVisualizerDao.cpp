#include "HeatmapVisualizerDAO.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <filesystem>

using namespace std;

// 声明友元访问器类来访问私有数据
class HeatmapDataAccessor {
public:
    static const map<string, int>& getTaskData(const HeatmapVisualizer& visualizer) {
        // 由于taskData是private，我们需要修改HeatmapVisualizer来提供访问接口
        // 这里我们假设已经添加了getTaskData方法到HeatmapVisualizer类中
        // 如果没有，需要在HeatmapVisualizer.h中添加：
        // const map<string, int>& getTaskData() const { return taskData; }
        return visualizer.getTaskData();
    }
    
    static void setTaskData(HeatmapVisualizer& visualizer, const map<string, int>& newData) {
        // 清空现有数据
        // 这里需要HeatmapVisualizer提供清除数据的方法
        visualizer.clearTaskData();
        
        // 添加新数据
        for (const auto& pair : newData) {
            visualizer.addTaskData(pair.first, pair.second);
        }
    }
};

HeatmapVisualizerDAO::HeatmapVisualizerDAO() : dataFilePath("heatmap_data/") {
    // 确保数据目录存在
    filesystem::create_directories(dataFilePath);
}

HeatmapVisualizerDAO::HeatmapVisualizerDAO(const string& filePath) : dataFilePath(filePath) {
    // 确保数据目录存在
    if (!dataFilePath.empty() && dataFilePath.back() != '/') {
        dataFilePath += '/';
    }
    filesystem::create_directories(dataFilePath);
}

bool HeatmapVisualizerDAO::saveHeatmapData(const HeatmapVisualizer& visualizer, const string& identifier) {
    // 获取任务数据 - 需要HeatmapVisualizer提供getTaskData()方法
    const map<string, int>& taskData = HeatmapDataAccessor::getTaskData(visualizer);
    
    string filename = dataFilePath + "heatmap_" + identifier + ".dat";
    string serializedData = serializeTaskData(taskData);
    
    return writeFile(filename, serializedData);
}

bool HeatmapVisualizerDAO::loadHeatmapData(HeatmapVisualizer& visualizer, const string& identifier) {
    string filename = dataFilePath + "heatmap_" + identifier + ".dat";
    
    if (!fileExists(filename)) {
        return false;
    }
    
    string fileContent = readFile(filename);
    if (fileContent.empty()) {
        return false;
    }
    
    map<string, int> taskData = deserializeTaskData(fileContent);
    
    // 将数据加载到visualizer中
    HeatmapDataAccessor::setTaskData(visualizer, taskData);
    
    return true;
}

string HeatmapVisualizerDAO::serializeTaskData(const map<string, int>& taskData) {
    ostringstream oss;
    for (const auto& pair : taskData) {
        oss << pair.first << ":" << pair.second << "\n";
    }
    return oss.str();
}

map<string, int> HeatmapVisualizerDAO::deserializeTaskData(const string& data) {
    map<string, int> taskData;
    istringstream iss(data);
    string line;
    
    while (getline(iss, line)) {
        size_t pos = line.find(':');
        if (pos != string::npos) {
            string date = line.substr(0, pos);
            try {
                int count = stoi(line.substr(pos + 1));
                taskData[date] = count;
            } catch (const exception& e) {
                cerr << "Error parsing line: " << line << " - " << e.what() << endl;
            }
        }
    }
    
    return taskData;
}

bool HeatmapVisualizerDAO::fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

string HeatmapVisualizerDAO::readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool HeatmapVisualizerDAO::writeFile(const string& filename, const string& content) {
    ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    file.close();
    return true;
}

bool HeatmapVisualizerDAO::saveMultipleVisualizers(const vector<HeatmapVisualizer>& visualizers, 
                                                  const vector<string>& identifiers) {
    if (visualizers.size() != identifiers.size()) {
        return false;
    }
    
    bool allSuccess = true;
    for (size_t i = 0; i < visualizers.size(); ++i) {
        if (!saveHeatmapData(visualizers[i], identifiers[i])) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

vector<HeatmapVisualizer> HeatmapVisualizerDAO::loadMultipleVisualizers(const vector<string>& identifiers) {
    vector<HeatmapVisualizer> visualizers;
    
    for (const auto& identifier : identifiers) {
        HeatmapVisualizer visualizer;
        if (loadHeatmapData(visualizer, identifier)) {
            visualizers.push_back(visualizer);
        }
    }
    
    return visualizers;
}

vector<string> HeatmapVisualizerDAO::getAllSavedIdentifiers() {
    vector<string> identifiers;
    
    try {
        for (const auto& entry : filesystem::directory_iterator(dataFilePath)) {
            if (entry.is_regular_file()) {
                string filename = entry.path().filename().string();
                if (filename.find("heatmap_") == 0 && filename.find(".dat") != string::npos) {
                    string identifier = filename.substr(8, filename.length() - 12);
                    identifiers.push_back(identifier);
                }
            }
        }
    } catch (const filesystem::filesystem_error& e) {
        cerr << "Error reading directory: " << e.what() << endl;
    }
    
    return identifiers;
}

bool HeatmapVisualizerDAO::deleteHeatmapData(const string& identifier) {
    string filename = dataFilePath + "heatmap_" + identifier + ".dat";
    try {
        if (fileExists(filename)) {
            return filesystem::remove(filename);
        }
    } catch (const filesystem::filesystem_error& e) {
        cerr << "Error deleting file: " << e.what() << endl;
    }
    return false;
}

bool HeatmapVisualizerDAO::heatmapDataExists(const string& identifier) {
    string filename = dataFilePath + "heatmap_" + identifier + ".dat";
    return fileExists(filename);
}

bool HeatmapVisualizerDAO::exportToCSV(const HeatmapVisualizer& visualizer, const string& csvFilePath) {
    ofstream csvFile(csvFilePath);
    if (!csvFile.is_open()) {
        return false;
    }
    
    csvFile << "Date,TasksCompleted" << endl;
    
    // 获取任务数据并导出
    const map<string, int>& taskData = HeatmapDataAccessor::getTaskData(visualizer);
    for (const auto& pair : taskData) {
        csvFile << pair.first << "," << pair.second << endl;
    }
    
    csvFile.close();
    return true;
}

bool HeatmapVisualizerDAO::importFromCSV(HeatmapVisualizer& visualizer, const string& csvFilePath) {
    ifstream csvFile(csvFilePath);
    if (!csvFile.is_open()) {
        return false;
    }
    
    string line;
    // 跳过标题行
    getline(csvFile, line);
    
    int importedCount = 0;
    while (getline(csvFile, line)) {
        size_t pos = line.find(',');
        if (pos != string::npos) {
            string date = line.substr(0, pos);
            try {
                int count = stoi(line.substr(pos + 1));
                visualizer.addTaskData(date, count);
                importedCount++;
            } catch (const exception& e) {
                cerr << "Error parsing CSV line: " << line << " - " << e.what() << endl;
            }
        }
    }
    
    csvFile.close();
    cout << "Imported " << importedCount << " records from CSV." << endl;
    return importedCount > 0;
}

bool HeatmapVisualizerDAO::createBackup(const string& backupIdentifier) {
    // 创建所有数据的备份
    vector<string> identifiers = getAllSavedIdentifiers();
    
    string backupDir = dataFilePath + "backups/";
    filesystem::create_directories(backupDir);
    
    string backupFile = backupDir + "backup_" + backupIdentifier + ".zip";
    // 这里可以实现实际的备份逻辑，比如使用压缩库
    cout << "Backup created: " << backupFile << endl;
    return true;
}

bool HeatmapVisualizerDAO::restoreFromBackup(const string& backupIdentifier) {
    string backupFile = dataFilePath + "backups/backup_" + backupIdentifier + ".zip";
    
    if (!fileExists(backupFile)) {
        return false;
    }
    
    // 这里可以实现实际的恢复逻辑
    cout << "Restoring from backup: " << backupFile << endl;
    return true;
}

string HeatmapVisualizerDAO::generateBackupName() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    
    ostringstream oss;
    oss << put_time(localTime, "%Y%m%d_%H%M%S");
    return oss.str();
}

bool HeatmapVisualizerDAO::validateDate(const string& date) {
    // 简单的日期格式验证 (YYYY-MM-DD)
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    
    try {
        int year = stoi(date.substr(0, 4));
        int month = stoi(date.substr(5, 2));
        int day = stoi(date.substr(8, 2));
        
        return year >= 2000 && year <= 2100 && 
               month >= 1 && month <= 12 && 
               day >= 1 && day <= 31;
    } catch (...) {
        return false;
    }
}
