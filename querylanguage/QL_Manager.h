#ifndef __QL_MANAGER_H__
#define __QL_MANAGER_H__

#include <vector>

#include "../systemmanagement/SM_Manager.h"
#include "../indexing/IX_Manager.h"
#include "../recordmanagement/RM_Manager.h"
#include "../utils/base.h"

class QL_Manager {
private:
    SM_Manager *mSMManager;
    IX_Manager *mIXManager;
    RM_Manager *mRMManager;

private:
    bool satisfyCondition(shared_ptr<RM_Record> ptrRec,
                          Condition condition,
                          AttrInfoEx info);
    int getIndexNo(const char *tableName);
    void getAttrInfoEx(const char *tableName, vector<AttrInfoEx> &attrs);

public:
    QL_Manager(SM_Manager *sm, IX_Manager *ix, RM_Manager *rm);
    ~QL_Manager();

    bool select(const std::vector<TableAttr> &attrs, 
                const std::vector<const char *> &tables, 
                const std::vector<Condition> &conditions);
    bool insert(const char *tableName, 
                const std::vector<Value> &values);
    bool remove(const char *tableName, 
                const std::vector<Condition> &conditions);
    bool update(const char *tableName, 
                const TableAttr &lAttr,
                const bool rIsValue,
                const TableAttr &rAttr,
                const Value &rValue,
                const std::vector<Condition> &conditions);
};

#endif // __QL_MANAGER_H__