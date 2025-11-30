#ifndef PROJECT_DAO_H
#define PROJECT_DAO_H

#include "../../project/Project.h"
#include <vector>
#include <string>
#include <sqlite3.h>

using namespace std;

class ProjectDAO {
private:
    sqlite3* db;
    string dbPath;
    
    bool openDatabase();
    void closeDatabase();

public:
    ProjectDAO();
    ProjectDAO(string dbPath);
    ~ProjectDAO();
    
    bool createTable();
    
    int insert(const Project& project);
    Project* selectById(int id);
    vector<Project*> selectAll();
    vector<Project*> selectAllIncludingArchived();
    bool update(const Project& project);
    bool deleteById(int id);
    bool hardDeleteById(int id);
    
    int count();
    int countActive();
};

#endif
