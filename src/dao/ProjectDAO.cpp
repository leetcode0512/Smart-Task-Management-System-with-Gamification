#include "../../include/dao/ProjectDAO.h"
#include <iostream>
#include <sstream>

ProjectDAO::ProjectDAO() {
    dbPath = "task_manager.db";
    db = nullptr;
}

ProjectDAO::ProjectDAO(string dbPath) {
    this->dbPath = dbPath;
    db = nullptr;
}

ProjectDAO::~ProjectDAO() {
    closeDatabase();
}

bool ProjectDAO::openDatabase() {
    int result = sqlite3_open(dbPath.c_str(), &db);
    if (result != SQLITE_OK) {
        cerr << "Cannot open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }
    return true;
}

void ProjectDAO::closeDatabase() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool ProjectDAO::createTable() {
    if (!openDatabase()) return false;
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS projects ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "description TEXT, "
        "color_label TEXT, "
        "progress REAL DEFAULT 0.0, "
        "total_tasks INTEGER DEFAULT 0, "
        "completed_tasks INTEGER DEFAULT 0, "
        "target_date TEXT, "
        "archived INTEGER DEFAULT 0, "
        "created_date TEXT DEFAULT CURRENT_TIMESTAMP, "
        "updated_date TEXT DEFAULT CURRENT_TIMESTAMP"
        ");";
    
    char* errMsg = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (result != SQLITE_OK) {
        cerr << "Create table failed: " << errMsg << endl;
        sqlite3_free(errMsg);
        closeDatabase();
        return false;
    }
    
    closeDatabase();
    return true;
}

int ProjectDAO::insert(const Project& project) {
    if (!openDatabase()) return -1;
    
    const char* sql = 
        "INSERT INTO projects (name, description, color_label, target_date) "
        "VALUES (?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        cerr << "Prepare SQL failed: " << sqlite3_errmsg(db) << endl;
        closeDatabase();
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, project.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, project.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, project.getColorLabel().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, project.getTargetDate().c_str(), -1, SQLITE_TRANSIENT);
    
    result = sqlite3_step(stmt);
    int lastId = -1;
    
    if (result == SQLITE_DONE) {
        lastId = sqlite3_last_insert_rowid(db);
        cout << "Project inserted successfully. ID: " << lastId << endl;
    } else {
        cerr << "Insert failed: " << sqlite3_errmsg(db) << endl;
    }
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return lastId;
}

Project* ProjectDAO::selectById(int id) {
    if (!openDatabase()) return nullptr;
    
    const char* sql = 
        "SELECT id, name, description, color_label, progress, "
        "total_tasks, completed_tasks, target_date, archived, "
        "created_date, updated_date FROM projects WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        cerr << "Prepare SQL failed: " << sqlite3_errmsg(db) << endl;
        closeDatabase();
        return nullptr;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    Project* project = nullptr;
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        project = new Project();
        project->setId(sqlite3_column_int(stmt, 0));
        project->setName((char*)sqlite3_column_text(stmt, 1));
        project->setDescription((char*)sqlite3_column_text(stmt, 2));
        project->setColorLabel((char*)sqlite3_column_text(stmt, 3));
        project->setProgress(sqlite3_column_double(stmt, 4));
        project->setTotalTasks(sqlite3_column_int(stmt, 5));
        project->setCompletedTasks(sqlite3_column_int(stmt, 6));
        project->setTargetDate((char*)sqlite3_column_text(stmt, 7));
        project->setArchived(sqlite3_column_int(stmt, 8) == 1);
        project->setCreatedDate((char*)sqlite3_column_text(stmt, 9));
        project->setUpdatedDate((char*)sqlite3_column_text(stmt, 10));
    }
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return project;
}

vector<Project*> ProjectDAO::selectAll() {
    vector<Project*> projects;
    
    if (!openDatabase()) return projects;
    
    const char* sql = 
        "SELECT id, name, description, color_label, progress, "
        "total_tasks, completed_tasks, target_date, archived, "
        "created_date, updated_date FROM projects WHERE archived = 0;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        cerr << "Prepare SQL failed: " << sqlite3_errmsg(db) << endl;
        closeDatabase();
        return projects;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Project* project = new Project();
        project->setId(sqlite3_column_int(stmt, 0));
        project->setName((char*)sqlite3_column_text(stmt, 1));
        project->setDescription((char*)sqlite3_column_text(stmt, 2));
        project->setColorLabel((char*)sqlite3_column_text(stmt, 3));
        project->setProgress(sqlite3_column_double(stmt, 4));
        project->setTotalTasks(sqlite3_column_int(stmt, 5));
        project->setCompletedTasks(sqlite3_column_int(stmt, 6));
        project->setTargetDate((char*)sqlite3_column_text(stmt, 7));
        project->setArchived(sqlite3_column_int(stmt, 8) == 1);
        project->setCreatedDate((char*)sqlite3_column_text(stmt, 9));
        project->setUpdatedDate((char*)sqlite3_column_text(stmt, 10));
        
        projects.push_back(project);
    }
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return projects;
}

vector<Project*> ProjectDAO::selectAllIncludingArchived() {
    vector<Project*> projects;
    
    if (!openDatabase()) return projects;
    
    const char* sql = 
        "SELECT id, name, description, color_label, progress, "
        "total_tasks, completed_tasks, target_date, archived, "
        "created_date, updated_date FROM projects;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        closeDatabase();
        return projects;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Project* project = new Project();
        project->setId(sqlite3_column_int(stmt, 0));
        project->setName((char*)sqlite3_column_text(stmt, 1));
        project->setDescription((char*)sqlite3_column_text(stmt, 2));
        project->setColorLabel((char*)sqlite3_column_text(stmt, 3));
        project->setProgress(sqlite3_column_double(stmt, 4));
        project->setTotalTasks(sqlite3_column_int(stmt, 5));
        project->setCompletedTasks(sqlite3_column_int(stmt, 6));
        project->setTargetDate((char*)sqlite3_column_text(stmt, 7));
        project->setArchived(sqlite3_column_int(stmt, 8) == 1);
        project->setCreatedDate((char*)sqlite3_column_text(stmt, 9));
        project->setUpdatedDate((char*)sqlite3_column_text(stmt, 10));
        
        projects.push_back(project);
    }
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return projects;
}

bool ProjectDAO::update(const Project& project) {
    if (!openDatabase()) return false;
    
    const char* sql = 
        "UPDATE projects SET name = ?, description = ?, color_label = ?, "
        "progress = ?, total_tasks = ?, completed_tasks = ?, "
        "target_date = ?, archived = ?, updated_date = CURRENT_TIMESTAMP "
        "WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        closeDatabase();
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, project.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, project.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, project.getColorLabel().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, project.getProgress());
    sqlite3_bind_int(stmt, 5, project.getTotalTasks());
    sqlite3_bind_int(stmt, 6, project.getCompletedTasks());
    sqlite3_bind_text(stmt, 7, project.getTargetDate().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, project.isArchived() ? 1 : 0);
    sqlite3_bind_int(stmt, 9, project.getId());
    
    result = sqlite3_step(stmt);
    bool success = (result == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return success;
}

bool ProjectDAO::deleteById(int id) {
    if (!openDatabase()) return false;
    
    const char* sql = "UPDATE projects SET archived = 1 WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        closeDatabase();
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    result = sqlite3_step(stmt);
    bool success = (result == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return success;
}

bool ProjectDAO::hardDeleteById(int id) {
    if (!openDatabase()) return false;
    
    const char* sql = "DELETE FROM projects WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        closeDatabase();
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    result = sqlite3_step(stmt);
    bool success = (result == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    closeDatabase();
    
    return success;
}

int ProjectDAO::count() {
    if (!openDatabase()) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM projects;";
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

int ProjectDAO::countActive() {
    if (!openDatabase()) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM projects WHERE archived = 0;";
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
