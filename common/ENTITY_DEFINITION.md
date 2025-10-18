# 实体类定义文档 (ENTITY_DEFINITION.md)

## 📋 文档概述

本文档定义了游戏化任务管理系统的所有数据实体类，作为各模块开发和集成的统一数据标准。

**最后更新**: 2025年10月18日  
**维护者**: Yu Zhixuan (数据库模块)  
**版本**: v1.0

## 🏗️ 实体类架构

### 基础实体结构
```cpp
BaseEntity
├── id: int (唯一标识)
├── created_date: string (创建时间)
└── updated_date: string (更新时间)
```

### 实体继承关系
```
BaseEntity
├── Task (任务)
├── Project (项目)
├── Challenge (挑战)
├── Reminder (提醒)
├── Achievement (成就)
├── UserStats (用户统计)
└── UserSettings (用户设置)
```

## 📊 实体类详细定义

### 1. Task (任务实体)
**负责人**: Kuang Wenqing - 任务管理模块

| 字段名 | 类型 | 必需 | 默认值 | 描述 |
|--------|------|------|--------|------|
| `title` | string | ✅ | - | 任务标题 |
| `description` | string | ❌ | "" | 任务详细描述 |
| `priority` | int | ✅ | 1 | 优先级 (0:低, 1:中, 2:高) |
| `due_date` | string | ❌ | "" | 截止日期 (YYYY-MM-DD) |
| `completed` | bool | ✅ | false | 完成状态 |
| `tags` | string | ❌ | "" | 标签 (逗号分隔) |
| `project_id` | int | ✅ | 0 | 所属项目ID |
| `pomodoro_count` | int | ✅ | 0 | 完成的番茄钟数量 |
| `estimated_pomodoros` | int | ❌ | 0 | 预估番茄钟数 |
| `completed_date` | string | ❌ | "" | 完成时间 |
| `reminder_time` | string | ❌ | "" | 提醒时间 |

**使用示例**:
```cpp
Task task;
task.title = "实现数据库模块";
task.description = "完成SQLite集成和DAO模式实现";
task.priority = 2; // 高优先级
task.due_date = "2025-10-25";
task.estimated_pomodoros = 8;
task.tags = "数据库,C++,SQLite";
```

### 2. Project (项目实体)
**负责人**: Zhou Tianjian - 项目管理模块

| 字段名 | 类型 | 必需 | 默认值 | 描述 |
|--------|------|------|--------|------|
| `name` | string | ✅ | - | 项目名称 |
| `description` | string | ❌ | "" | 项目描述 |
| `color_label` | string | ❌ | "" | 颜色标签 (#RRGGBB格式) |
| `progress` | double | ✅ | 0.0 | 进度百分比 (0.0-1.0) |
| `total_tasks` | int | ✅ | 0 | 总任务数 |
| `completed_tasks` | int | ✅ | 0 | 已完成任务数 |
| `target_date` | string | ❌ | "" | 目标完成日期 |
| `archived` | bool | ✅ | false | 是否已归档 |

**使用示例**:
```cpp
Project project;
project.name = "游戏化任务管理系统";
project.description = "C++课程期末项目";
project.color_label = "#4CAF50";
project.target_date = "2025-12-15";
```

### 3. Challenge (挑战实体)
**负责人**: Yu Zhixuan - 挑战系统

| 字段名 | 类型 | 必需 | 默认值 | 描述 |
|--------|------|------|--------|------|
| `title` | string | ✅ | - | 挑战标题 |
| `description` | string | ✅ | - | 挑战描述 |
| `type` | string | ✅ | - | 类型 ("daily", "weekly", "monthly") |
| `criteria` | string | ✅ | - | 完成条件描述 |
| `target_value` | int | ✅ | 0 | 目标值 |
| `current_value` | int | ✅ | 0 | 当前进度值 |
| `reward_xp` | int | ✅ | 0 | 奖励经验值 |
| `completed` | bool | ✅ | false | 是否完成 |
| `claimed` | bool | ✅ | false | 是否已领取奖励 |
| `expiry_date` | string | ✅ | - | 过期时间 |
| `category` | string | ✅ | - | 分类 ("task", "pomodoro", "project") |

**使用示例**:
```cpp
Challenge challenge;
challenge.title = "番茄钟大师";
challenge.description = "本周完成10个番茄钟工作会话";
challenge.type = "weekly";
challenge.criteria = "complete_pomodoros";
challenge.target_value = 10;
challenge.reward_xp = 100;
challenge.expiry_date = "2025-10-27";
challenge.category = "pomodoro";
```

### 4. Reminder (提醒实体)
**负责人**: Fei Yifan - 提醒系统

| 字段名 | 类型 | 必需 | 默认值 | 描述 |
|--------|------|------|--------|------|
| `title` | string | ✅ | - | 提醒标题 |
| `message` | string | ❌ | "" | 提醒消息 |
| `trigger_time` | string | ✅ | - | 触发时间 |
| `recurrence` | string | ✅ | "once" | 重复规则 ("once", "daily", "weekly", "monthly") |
| `triggered` | bool | ✅ | false | 是否已触发 |
| `task_id` | int | ❌ | 0 | 关联的任务ID |
| `enabled` | bool | ✅ | true | 是否启用 |
| `last_triggered` | string | ❌ | "" | 上次触发时间 |

**使用示例**:
```cpp
Reminder reminder;
reminder.title = "项目会议提醒";
reminder.message = "每周项目进度同步会议";
reminder.trigger_time = "2025-10-20 14:00:00";
reminder.recurrence = "weekly";
reminder.task_id = 42;
```

### 5. Achievement (成就实体)
**负责人**: Fei Yifan - 成就系统

| 字段名 | 类型 | 必需 | 默认值 | 描述 |
|--------|------|------|--------|------|
| `name` | string | ✅ | - | 成就名称 |
| `description` | string | ✅ | - | 成就描述 |
| `icon` | string | ❌ | "" | 图标标识 |
| `unlock_condition` | string | ✅ | - | 解锁条件 |
| `unlocked` | bool | ✅ | false | 是否已解锁 |
| `unlocked_date` | string | ❌ | "" | 解锁时间 |
| `reward_xp` | int | ✅ | 0 | 奖励经验值 |
| `category` | string | ✅ | - | 分类 ("task", "time", "streak", "special") |
| `progress` | int | ✅ | 0 | 解锁进度 (0-100) |
| `target_value` | int | ✅ | 0 | 目标值 |

**使用示例**:
```cpp
Achievement achievement;
achievement.name = "起步者";
achievement.description = "完成第一个任务";
achievement.unlock_condition = "complete_first_task";
achievement.reward_xp = 50;
achievement.category = "task";
achievement.target_value = 1;
```

### 6. UserStats (用户统计实体)
**负责人**: Mao Jingqi - 统计系统

| 字段名 | 类型 | 必需 | 默认值 | 描述 |
|--------|------|------|--------|------|
| `total_tasks_created` | int | ✅ | 0 | 总创建任务数 |
| `total_tasks_completed` | int | ✅ | 0 | 总完成任务数 |
| `total_pomodoros` | int | ✅ | 0 | 总番茄钟数 |
| `current_streak` | int | ✅ | 0 | 当前连续打卡天数 |
| `longest_streak` | int | ✅ | 0 | 最长连续打卡天数 |
| `total_xp` | int | ✅ | 0 | 总经验值 |
| `level` | int | ✅ | 1 | 当前等级 |
| `last_active_date` | string | ✅ | - | 最后活跃日期 |
| `completion_rate` | double | ✅ | 0.0 | 平均完成率 |
| `achievements_unlocked` | int | ✅ | 0 | 已解锁成就数 |

### 7. UserSettings (用户设置实体)
**负责人**: Mao Jingqi - UI/设置系统

| 字段名 | 类型 | 必需 | 默认值 | 描述 |
|--------|------|------|--------|------|
| `pomodoro_duration` | int | ✅ | 25 | 番茄钟工作时间 (分钟) |
| `short_break_duration` | int | ✅ | 5 | 短休息时间 (分钟) |
| `long_break_duration` | int | ✅ | 15 | 长休息时间 (分钟) |
| `pomodoros_until_long_break` | int | ✅ | 4 | 长休息间隔 |
| `sound_enabled` | bool | ✅ | true | 是否启用声音 |
| `notifications_enabled` | bool | ✅ | true | 是否启用通知 |
| `theme` | string | ✅ | "default" | 主题设置 |
| `language` | string | ✅ | "zh" | 语言设置 |
| `auto_start_pomodoros` | bool | ✅ | false | 是否自动开始番茄钟 |

## 🔄 数据关系图

```
UserStats (1:1) UserSettings
    ↑            ↑
    |            |
Task (n:1) Project
    ↑            ↑
    |            |
Reminder (n:1) Task
    |
    |
Achievement (独立)
    ↑
    |
Challenge (独立)
```

## 📝 字段规范说明

### 日期时间格式
- 所有日期字段使用 **ISO 8601** 格式: `YYYY-MM-DD`
- 日期时间字段格式: `YYYY-MM-DD HH:MM:SS`
- 示例: `"2025-10-20"`, `"2025-10-20 14:30:00"`

### 枚举值定义
```cpp
// 优先级枚举
enum Priority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2
};

// 挑战类型枚举
enum ChallengeType {
    DAILY = "daily",
    WEEKLY = "weekly", 
    MONTHLY = "monthly"
};

// 重复规则枚举
enum Recurrence {
    ONCE = "once",
    DAILY = "daily",
    WEEKLY = "weekly",
    MONTHLY = "monthly"
};
```

## 🛠️ 使用指南

### 包含头文件
```cpp
#include "common/entities.h"
```

### 创建实体对象
```cpp
// 创建任务
Task task;
task.title = "学习C++";
task.priority = 1;
task.due_date = "2025-10-25";

// 创建项目
Project project;
project.name = "学习项目";
project.color_label = "#2196F3";
```

### 序列化建议
```cpp
// 建议使用JSON格式进行数据交换
// 示例JSON结构:
{
  "task": {
    "id": 1,
    "title": "实现数据库模块",
    "priority": 2,
    "completed": false,
    "created_date": "2025-10-18 10:00:00"
  }
}
```

## 🔍 字段变更记录

| 版本 | 日期 | 变更内容 | 负责人 |
|------|------|----------|--------|
| v1.0 | 2025-10-18 | 初始版本发布 | Yu Zhixuan |

## 📞 技术支持

### 问题反馈
如发现字段定义问题或需要新增字段，请通过以下方式反馈：

1. **GitHub Issues**: 创建问题单
2. **团队群聊**: 直接讨论
3. **邮件**: 发送详细说明

### 负责人联系
- **数据库模块**: Yu Zhixuan
- **任务管理**: Kuang Wenqing  
- **项目管理**: Zhou Tianjian
- **提醒成就**: Fei Yifan
- **统计UI**: Mao Jingqi

## ✅ 验收标准

各模块在实现时应确保：

1. ✅ 所有必需字段都有合理默认值
2. ✅ 数据类型与定义一致
3. ✅ 字段验证逻辑完整
4. ✅ 与其他实体关联正确
5. ✅ 序列化/反序列化正常

---

*本文档将根据项目进展持续更新，请关注最新版本*
