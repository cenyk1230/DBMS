#ifndef __SM_MANAGER_H__
#define __SM_MANAGER_H__

#include <string>
#include <vector>

#include "../utils/base.h"
#include "../recordmanagement/RM_Manager.h"
#include "../indexing/IX_Manager.h"


class SM_Manager {
private:
    IX_Manager *mIXManager;
    RM_Manager *mRMManager;
    std::string mDBName;

public:
    SM_Manager(IX_Manager *ix, RM_Manager *rm);
    ~SM_Manager();

    bool createDB(const char *DBName);
    bool dropDB(const char *DBName);
    bool useDB(const char *DBName);
    bool showDB(const char *DBName);

    bool createTable(const char *tableName, const char *primaryKey, std::vector<AttrInfo> attributes);
    bool dropTable(const char *tableName);
    bool showTable(const char *tableName);
};

#endif // __SM_MANAGER_H__