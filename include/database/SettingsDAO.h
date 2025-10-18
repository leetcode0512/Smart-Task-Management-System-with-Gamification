#ifndef SETTINGS_DAO_H
#define SETTINGS_DAO_H

#include "Entities.h"
#include <map>
#include <optional>

class SettingsDAO {
public:
    virtual ~SettingsDAO() = default;
    
    // 设置管理
    virtual bool setSetting(const std::string& key, const std::string& value) = 0;
    virtual bool setIntSetting(const std::string& key, int value) = 0;
    virtual bool setBoolSetting(const std::string& key, bool value) = 0;
    virtual bool setDoubleSetting(const std::string& key, double value) = 0;
    
    // 查询设置
    virtual std::optional<std::string> getSetting(const std::string& key) = 0;
    virtual int getIntSetting(const std::string& key, int defaultValue = 0) = 0;
    virtual bool getBoolSetting(const std::string& key, bool defaultValue = false) = 0;
    virtual double getDoubleSetting(const std::string& key, double defaultValue = 0.0) = 0;
    
    // 批量操作
    virtual bool setMultipleSettings(const std::map<std::string, std::string>& settings) = 0;
    virtual std::map<std::string, std::string> getAllSettings() = 0;
    
    // 删除操作
    virtual bool deleteSetting(const std::string& key) = 0;
    virtual bool deleteSettingsByPrefix(const std::string& prefix) = 0;
    
    // 用户偏好
    virtual bool setUserPreference(const std::string& preference, const std::string& value) = 0;
    virtual std::optional<std::string> getUserPreference(const std::string& preference) = 0;
    
    // Pomodoro设置
    virtual bool setPomodoroSettings(const PomodoroSettings& settings) = 0;
    virtual PomodoroSettings getPomodoroSettings() = 0;
    
    // 主题和UI设置
    virtual bool setThemeSettings(const ThemeSettings& settings) = 0;
    virtual ThemeSettings getThemeSettings() = 0;
};

#endif // SETTINGS_DAO_H