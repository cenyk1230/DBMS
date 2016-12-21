#include <cstdio>
#include <vector>

#include "QL_Manager.h"

using namespace std;

const int NAMES_LEN[3] = {2, 5, 4};
const char NAMES[3][10] = {"id", "score", "name"};

int main() {
    MyBitMap::initConst();

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
        info.attrLength = i % 3 == 2 ? 10 : 4;
        attributes.push_back(info);
    }
    sm->createTable(tableName, "id", attributes);

    vector<Value> values;
    int int1 = 124;
    float float1 = 2.5;
    char string1[10] = "abcde";
    // for (int i = 0; i < attrCount; ++i) {
    //     Value v;
    //     v.attrType = (AttrType)(i % 3);
    //     if (i == 0) {
    //         v.data = (char *)&int1;
    //     } else if (i == 1) {
    //         v.data = (char *)&float1;
    //     } else {
    //         v.data = string1;
    //     }
    //     values.push_back(v);
    // }
    for (int j = 0; j < 1000; ++j) {
        int tmpInt = j;
        values.clear();
        for (int i = 0; i < attrCount; ++i) {
            Value v;
            v.attrType = (AttrType)(i % 3);
            if (i == 0) {
                v.data = (char *)&tmpInt;
            } else if (i == 1) {
                v.data = (char *)&float1;
            } else {
                v.data = string1;
            }
            values.push_back(v);
        }
        fprintf(stdout, "j = %d, insert flag = %d\n", j, ql->insert(tableName, values));
    }
    vector<TableAttr> attrs;
    for (int i = 0; i < attrCount; ++i) {
        TableAttr attr;
        attr.attrName = NAMES[i];
        attrs.push_back(attr);
    }
    vector<const char *> tables;
    tables.push_back(tableName);
    vector<Condition> selectConds;
    fprintf(stdout, "select flag = %d\n", ql->select(attrs, tables, selectConds));

    float float2 = 3.9;
    Value updateValue;
    updateValue.attrType = FLOAT;
    updateValue.data = (char *)&float2;
    vector<Condition> updateConds;
    fprintf(stdout, "update flag = %d\n", ql->update(tableName, attrs[1], updateValue, updateConds));
    fprintf(stdout, "select flag = %d\n", ql->select(attrs, tables, selectConds));

    vector<Condition> removeConds;
    fprintf(stdout, "delete flag = %d\n", ql->remove(tableName, removeConds));

    fprintf(stdout, "select flag = %d\n", ql->select(attrs, tables, selectConds));
    
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