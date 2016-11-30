#include <cstdio>

#include "SM_Manager.h"

using namespace std;

int main() {
    FileManager *fm = new FileManager();
    BufPageManager *bpm = new BufPageManager(fm);
    RM_Manager *rm = new RM_Manager(fm, bpm);
    IX_Manager *ix = new IX_Manager(fm, bpm);
    SM_Manager *sm = new SM_Manager(ix, rm);

    char DBName[10] = "TestDB";
    sm->createDB(DBName);
    sm->useDB(DBName);
    
    char tableName1[20] = "TestTable1";
    char tableName2[20] = "TestTable2";
    int attrCount = 3;
    AttrInfo *attributes = new AttrInfo[attrCount];
    char **pData = new char*[attrCount];
    for (int i = 0; i < attrCount; ++i) {
        pData[i] = new char[20];
        memset(pData[i], 0, sizeof(char) * 20);
        pData[i][0] = 'a' + i;
        attributes[i].attrName = pData[i];
        attributes[i].attrType = (AttrType)(i % 3);
        attributes[i].attrLength = 4;
    }
    sm->createTable(tableName1, attrCount, attributes, -1);
    sm->createTable(tableName2, attrCount, attributes, -1);

    sm->showDB(DBName);

    sm->showTable(tableName1);
    sm->showTable(tableName2);

    sm->dropTable(tableName1);
    sm->dropTable(tableName2);

    sm->dropDB(DBName);

    for (int i = 0; i < attrCount; ++i) {
        delete[] pData[i];
    }
    delete[] pData;
    delete[] attributes;

    delete fm;
    delete bpm;
    delete rm;
    delete ix;
    delete sm;
    return 0;
}