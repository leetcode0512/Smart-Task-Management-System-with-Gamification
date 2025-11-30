#pragma once
#include <string>
#include <vector>
#include <map>
#include "entities.h"

class AchievementDAO {
private:
    std::string dataFilePath;

    std::vector<Achievement> achievementDefinitions;
    std::vector<Achievement> userAchievements;

    std::string getDefinitionFilePath() const;
    std::string getUserAchievementFilePath(int userId) const;
    bool fileExists(const std::string& filename) const;
    bool writeFile(const std::string& filename, const std::string& content);

public:
    AchievementDAO();
    explicit AchievementDAO(const std::string& basePath);

    bool loadAchievementDefinitions();
    bool saveAchievementDefinitions();
    std::vector<Achievement> getAllAchievementDefinitions() const;
    Achievement getAchievementDefinition(const std::string& achievementKey) const;

    bool loadUserAchievements(int userId);
    bool saveUserAchievements(int userId);
    std::vector<Achievement> getUserAchievements(int userId) const;
    Achievement* getUserAchievement(int userId, const std::string& achievementKey);

    bool unlockAchievement(int userId, const std::string& achievementKey);
    bool updateAchievementProgress(int userId, const std::string& achievementKey, int progress);
    bool resetUserAchievements(int userId);

    int getUnlockedAchievementCount(int userId) const;
    int getTotalXP(int userId) const;
    std::vector<Achievement> getRecentlyUnlockedAchievements(int userId, int count = 5) const;

    void initializeDefaultAchievements();
    bool userAchievementFileExists(int userId) const;

    static std::string getCurrentTimestamp();

private:
    int generateAchievementId();
    int nextAchievementId = 1;
};


