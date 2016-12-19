#include <cstdio>
#include <vector>

#include "QL_Manager.h"

using namespace std;

const int NAMES_LEN[3] = {2, 5, 4};
const char NAMES[3][10] = {"id", "score", "name"};

int main() {
    FileManager *fm = new FileManager();
    BufPageManager *bpm = new BufPageManager(fm);
    RM_Manager *rm = new RM_Manager(fm, bpm);
    IX_Manager *ix = new IX_Manager(fm, bpm);
    SM_Manager *sm = new SM_Manager(ix, rm);
    QL_Manager *ql = new QL_Manager(sm, ix, rm);

    char DBName[10] = "TestDB";
    sm->createDB(DBName);
    sm->useDB(DBName);

    char tableName[20] = "TestTable";
    int attrCount = 3;
    vector<AttrInfo> attributes;
    AttrInfo info;
    char **pData = new char*[attrCount];
    for (int i = 0; i < attrCount; ++i) {
        pData[i] = new char[20];
        memset(pData[i], 0, sizeof(char) * 20);
        memcpy(pData[i], NAMES[i], sizeof(char) * NAMES_LEN[i]);
        info.attrName = pData[i];
        info.attrType = (AttrType)(i % 3);
        info.attrLength = i % 3 == 2 ? 20 : 4;
        attributes.push_back(info);
    }
    sm->createTable(tableName, "id", attributes);

    vector<Value> values;
    ql->insert(tableName, values);

    vector<Condition> conditions;
    ql->remove(tableName, conditions);

    //sm->showDB(DBName);

    //sm->showTable(tableName);

    //sm->dropTable(tableName);

    //sm->dropDB(DBName);

    for (int i = 0; i < attrCount; ++i) {
        delete[] pData[i];
    }
    delete[] pData;

    delete fm;
    delete bpm;
    delete rm;
    delete ix;
    delete sm;
    delete ql;
    return 0;
}