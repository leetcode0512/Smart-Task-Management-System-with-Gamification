#include "AchievementDAO.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <iomanip>

using namespace std;

AchievementDAO::AchievementDAO() : dataFilePath("./data/") {
    initializeDefaultAchievements();
}

AchievementDAO::AchievementDAO(const std::string& basePath) : dataFilePath(basePath) {
    if (!dataFilePath.empty() && dataFilePath.back() != '/') {
        dataFilePath += '/';
    }
    initializeDefaultAchievements();
}

void AchievementDAO::initializeDefaultAchievements() {
    if (!loadAchievementDefinitions()) {
        achievementDefinitions.push_back({
            generateAchievementId(),
            getCurrentTimestamp(),
            getCurrentTimestamp(),
            "首次任务",
            "完成第一个任务",
            "🎯",
            "完成1个任务",
            false,
            "",
            100,
            "task",
            0,
            1
        });
        
        achievementDefinitions.push_back({
            generateAchievementId(),
            getCurrentTimestamp(),
            getCurrentTimestamp(),
            "任务达人", 
            "完成50个任务",
            "⭐",
            "完成50个任务",
            false,
            "",
            500,
            "task",
            0,
            50
        });
        
        achievementDefinitions.push_back({
            generateAchievementId(),
            getCurrentTimestamp(),
            getCurrentTimestamp(),
            "七日连胜",
            "连续7天完成任务", 
            "🔥",
            "连续7天有任务完成",
            false,
            "",
            300,
            "streak",
            0,
            7
        });
        
        saveAchievementDefinitions();
    }
}

bool AchievementDAO::loadAchievementDefinitions() {
    ifstream file(getDefinitionFilePath());
    if (!file.is_open()) {
        return false;
    }
    
    achievementDefinitions.clear();
    string line;
    getline(file, line); // 跳过标题行
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        string id, created_date, updated_date, name, description, icon;
        string unlock_condition, unlocked_str, unlocked_date, reward_xp_str;
        string category, progress_str, target_value_str;
        
        getline(ss, id, ',');
        getline(ss, created_date, ',');
        getline(ss, updated_date, ',');
        getline(ss, name, ',');
        getline(ss, description, ',');
        getline(ss, icon, ',');
        getline(ss, unlock_condition, ',');
        getline(ss, unlocked_str, ',');
        getline(ss, unlocked_date, ',');
        getline(ss, reward_xp_str, ',');
        getline(ss, category, ',');
        getline(ss, progress_str, ',');
        getline(ss, target_value_str, ',');
        
        try {
            Achievement achievement;
            achievement.id = stoi(id);
            achievement.created_date = created_date;
            achievement.updated_date = updated_date;
            achievement.name = name;
            achievement.description = description;
            achievement.icon = icon;
            achievement.unlock_condition = unlock_condition;
            achievement.unlocked = (unlocked_str == "1");
            achievement.unlocked_date = unlocked_date;
            achievement.reward_xp = stoi(reward_xp_str);
            achievement.category = category;
            achievement.progress = stoi(progress_str);
            achievement.target_value = stoi(target_value_str);
            
            achievementDefinitions.push_back(achievement);
            nextAchievementId = max(nextAchievementId, achievement.id + 1);
        } catch (const exception& e) {
            cerr << "Error parsing achievement: " << e.what() << endl;
        }
    }
    
    file.close();
    return !achievementDefinitions.empty();
}

bool AchievementDAO::saveAchievementDefinitions() {
    ofstream file(getDefinitionFilePath());
    if (!file.is_open()) {
        return false;
    }
    
    file << "id,created_date,updated_date,name,description,icon,unlock_condition,unlocked,unlocked_date,reward_xp,category,progress,target_value\n";
    
    for (const auto& achievement : achievementDefinitions) {
        file << achievement.id << ","
             << achievement.created_date << ","
             << achievement.updated_date << ","
             << achievement.name << ","
             << achievement.description << ","
             << achievement.icon << ","
             << achievement.unlock_condition << ","
             << (achievement.unlocked ? "1" : "0") << ","
             << achievement.unlocked_date << ","
             << achievement.reward_xp << ","
             << achievement.category << ","
             << achievement.progress << ","
             << achievement.target_value << "\n";
    }
    
    file.close();
    return true;
}

vector<Achievement> AchievementDAO::getAllAchievementDefinitions() const {
    return achievementDefinitions;
}

Achievement AchievementDAO::getAchievementDefinition(const string& achievementName) const {
    for (const auto& achievement : achievementDefinitions) {
        if (achievement.name == achievementName) {
            return achievement;
        }
    }
    return Achievement();
}

bool AchievementDAO::loadUserAchievements(int userId) {
    ifstream file(getUserAchievementFilePath(userId));
    if (!file.is_open()) {
        return false;
    }
    
    userAchievements.clear();
    string line;
    getline(file, line); // 跳过标题行
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        string id, created_date, updated_date, name, description, icon;
        string unlock_condition, unlocked_str, unlocked_date, reward_xp_str;
        string category, progress_str, target_value_str;
        
        getline(ss, id, ',');
        getline(ss, created_date, ',');
        getline(ss, updated_date, ',');
        getline(ss, name, ',');
        getline(ss, description, ',');
        getline(ss, icon, ',');
        getline(ss, unlock_condition, ',');
        getline(ss, unlocked_str, ',');
        getline(ss, unlocked_date, ',');
        getline(ss, reward_xp_str, ',');
        getline(ss, category, ',');
        getline(ss, progress_str, ',');
        getline(ss, target_value_str, ',');
        
        try {
            Achievement achievement;
            achievement.id = stoi(id);
            achievement.created_date = created_date;
            achievement.updated_date = updated_date;
            achievement.name = name;
            achievement.description = description;
            achievement.icon = icon;
            achievement.unlock_condition = unlock_condition;
            achievement.unlocked = (unlocked_str == "1");
            achievement.unlocked_date = unlocked_date;
            achievement.reward_xp = stoi(reward_xp_str);
            achievement.category = category;
            achievement.progress = stoi(progress_str);
            achievement.target_value = stoi(target_value_str);
            
            userAchievements.push_back(achievement);
        } catch (const exception& e) {
            cerr << "Error parsing user achievement: " << e.what() << endl;
        }
    }
    
    file.close();
    return true;
}

bool AchievementDAO::saveUserAchievements(int userId) {
    ofstream file(getUserAchievementFilePath(userId));
    if (!file.is_open()) {
        return false;
    }
    
    file << "id,created_date,updated_date,name,description,icon,unlock_condition,unlocked,unlocked_date,reward_xp,category,progress,target_value\n";
    
    for (const auto& achievement : userAchievements) {
        file << achievement.id << ","
             << achievement.created_date << ","
             << achievement.updated_date << ","
             << achievement.name << ","
             << achievement.description << ","
             << achievement.icon << ","
             << achievement.unlock_condition << ","
             << (achievement.unlocked ? "1" : "0") << ","
             << achievement.unlocked_date << ","
             << achievement.reward_xp << ","
             << achievement.category << ","
             << achievement.progress << ","
             << achievement.target_value << "\n";
    }
    
    file.close();
    return true;
}

vector<Achievement> AchievementDAO::getUserAchievements(int userId) const {
    return userAchievements;
}

Achievement* AchievementDAO::getUserAchievement(int userId, const string& achievementName) {
    for (auto& achievement : userAchievements) {
        if (achievement.name == achievementName) {
            return &achievement;
        }
    }
    
    Achievement definition = getAchievementDefinition(achievementName);
    if (definition.id == 0) {
        return nullptr;
    }
    
    Achievement newAchievement = definition;
    newAchievement.id = generateAchievementId();
    newAchievement.created_date = getCurrentTimestamp();
    newAchievement.updated_date = getCurrentTimestamp();
    newAchievement.unlocked = false;
    newAchievement.unlocked_date = "";
    newAchievement.progress = 0;
    
    userAchievements.push_back(newAchievement);
    return &userAchievements.back();
}

bool AchievementDAO::unlockAchievement(int userId, const string& achievementName) {
    Achievement* achievement = getUserAchievement(userId, achievementName);
    if (achievement && !achievement->unlocked) {
        achievement->unlocked = true;
        achievement->progress = achievement->target_value;
        achievement->unlocked_date = getCurrentTimestamp();
        achievement->updated_date = getCurrentTimestamp();
        return saveUserAchievements(userId);
    }
    return false;
}

bool AchievementDAO::updateAchievementProgress(int userId, const string& achievementName, int progress) {
    Achievement* achievement = getUserAchievement(userId, achievementName);
    if (achievement && !achievement->unlocked) {
        achievement->progress = progress;
        achievement->updated_date = getCurrentTimestamp();
        
        if (progress >= achievement->target_value) {
            return unlockAchievement(userId, achievementName);
        }
        
        return saveUserAchievements(userId);
    }
    return false;
}

bool AchievementDAO::resetUserAchievements(int userId) {
    userAchievements.clear();
    return saveUserAchievements(userId);
}

int AchievementDAO::getUnlockedAchievementCount(int userId) const {
    int count = 0;
    for (const auto& achievement : userAchievements) {
        if (achievement.unlocked) count++;
    }
    return count;
}

int AchievementDAO::getTotalXP(int userId) const {
    int totalXP = 0;
    for (const auto& achievement : userAchievements) {
        if (achievement.unlocked) totalXP += achievement.reward_xp;
    }
    return totalXP;
}

vector<Achievement> AchievementDAO::getRecentlyUnlockedAchievements(int userId, int count) const {
    vector<Achievement> result;
    vector<Achievement> unlocked;
    
    for (const auto& achievement : userAchievements) {
        if (achievement.unlocked && !achievement.unlocked_date.empty()) {
            unlocked.push_back(achievement);
        }
    }
    
    sort(unlocked.begin(), unlocked.end(),
        [](const Achievement& a, const Achievement& b) {
            return a.unlocked_date > b.unlocked_date;
        });
    
    for (int i = 0; i < min(count, (int)unlocked.size()); i++) {
        result.push_back(unlocked[i]);
    }
    
    return result;
}

bool AchievementDAO::userAchievementFileExists(int userId) const {
    return fileExists(getUserAchievementFilePath(userId));
}

string AchievementDAO::getDefinitionFilePath() const {
    return dataFilePath + "achievement_definitions.csv";
}

string AchievementDAO::getUserAchievementFilePath(int userId) const {
    return dataFilePath + "user_" + to_string(userId) + "_achievements.csv";
}

bool AchievementDAO::fileExists(const string& filename) const {
    ifstream file(filename);
    return file.good();
}

bool AchievementDAO::writeFile(const string& filename, const string& content) {
    ofstream file(filename);
    if (!file.is_open()) return false;
    file << content;
    file.close();
    return true;
}

string AchievementDAO::getCurrentTimestamp() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    stringstream ss;
    ss << put_time(localTime, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int AchievementDAO::generateAchievementId() {
    return nextAchievementId++;
}
