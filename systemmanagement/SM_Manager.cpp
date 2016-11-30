#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <string>

#include "SM_Manager.h"

static const int MAX_NAME_LEN = 20; 

SM_Manager::SM_Manager(IX_Manager *ix, RM_Manager *rm) {
    mIXManager = ix;
    mRMManager = rm;
    mDBName = "";
}

SM_Manager::~SM_Manager() {

}

bool SM_Manager::createDB(const char *DBName) {
    int len = strlen(DBName);
    assert(len < MAX_NAME_LEN);

    char command[MAX_NAME_LEN + 20];
    sprintf(command, "mkdir %s", DBName);
    system(command);

    char relcat[MAX_NAME_LEN + 20];
    sprintf(relcat, "%s/relcat", DBName);
    mRMManager->createFile(relcat, MAX_NAME_LEN + 12);

    char attrcat[MAX_NAME_LEN + 20];
    sprintf(attrcat, "%s/attrcat", DBName);
    mRMManager->createFile(attrcat, MAX_NAME_LEN * 2 + 16);

    return true;
}

bool SM_Manager::dropDB(const char *DBName) {
    int len = strlen(DBName);
    char command[MAX_NAME_LEN + 20];
    sprintf(command, "rm -r %s", DBName);
    system(command);
    return true;
}

bool SM_Manager::useDB(const char *DBName) {
    mDBName = string(DBName);
    return true;
}

bool SM_Manager::showDB(const char *DBName) {
    RM_FileHandle *handle;
    char relcat[MAX_NAME_LEN + 20];
    sprintf(relcat, "%s/relcat", DBName);
    mRMManager->openFile(relcat, handle);
    
    int pageNum = handle->getPageNum();
    vector<shared_ptr<RM_Record> > records;
    fprintf(stdout, "DATABASE %s:\n", DBName);
    for (int i = 0; i < pageNum; ++i) {
        handle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<RM_Record> rec = records[j];
            char *pData = rec->getData();
            fprintf(stdout, "  TABLE %s\n", pData);
        }
    }
    mRMManager->closeFile(handle);
    return true;
}

bool SM_Manager::createTable(const char *tableName, int attrCount, AttrInfo *attributes, int primaryKey) {
    //cerr << "SM_Manager::createTable begin" << endl;
    assert(mDBName != "");

    int recordSize = 0;
    for (int i = 0; i < attrCount; ++i) {
        recordSize += attributes->attrLength;
    }
    string fullTableName = mDBName + "/" + string(tableName);
    mRMManager->createFile(fullTableName.c_str(), recordSize);

    RM_FileHandle *handle;
    string relcat = mDBName + "/relcat";
    mRMManager->openFile(relcat.c_str(), handle);
    RID rid;
    char *pData = new char[MAX_NAME_LEN + 12];
    int len = strlen(tableName);
    assert(len < MAX_NAME_LEN);
    for (int i = 0; i < MAX_NAME_LEN; ++i) {
        if (i < len)
            pData[i] = tableName[i];
        else
            pData[i] = 0;
    }
    BufType data = (BufType)(pData + MAX_NAME_LEN);
    data[0] = recordSize;
    data[1] = attrCount;
    data[2] = primaryKey;
    handle->insertRec(pData, rid);
    mRMManager->closeFile(handle);
    delete[] pData;

    string attrcat = mDBName + "/attrcat"; 
    mRMManager->openFile(attrcat.c_str(), handle);
    int offset = 0;
    for (int i = 0; i < attrCount; ++i) {
        pData = new char[MAX_NAME_LEN * 2 + 16];
        for (int i = 0; i < MAX_NAME_LEN; ++i) {
            if (i < len)
                pData[i] = tableName[i];
            else
                pData[i] = 0;
        }
        int attrLen = strlen(attributes[i].attrName);
        assert(attrLen < MAX_NAME_LEN);
        for (int j = 0; j < MAX_NAME_LEN; ++j) {
            if (j < attrLen)
                pData[MAX_NAME_LEN + j] = attributes[i].attrName[j];
            else
                pData[MAX_NAME_LEN + j] = 0;   
        }
        BufType data = (BufType)(pData + MAX_NAME_LEN * 2);
        data[0] = offset;
        data[1] = (unsigned int)attributes[i].attrType;
        data[2] = attributes[i].attrLength;
        if (primaryKey == i)
            data[3] = 0;
        else
            data[3] = -1;
        
        handle->insertRec(pData, rid);
        offset += attributes[i].attrLength;
        delete[] pData;
    }
    mRMManager->closeFile(handle);
    //cerr << "SM_Manager::createTable end" << endl;
    return true;
}

bool SM_Manager::dropTable(const char *tableName) {
    int indexNo = 0;
    for (int i = 0; i < indexNo; ++i) {
        mIXManager->destroyIndex(tableName, i);
    }
    return mRMManager->destroyFile(tableName);
}

bool SM_Manager::showTable(const char *tableName) {
    RM_FileHandle *handle;
    string attrcat = mDBName + "/attrcat"; 
    mRMManager->openFile(attrcat.c_str(), handle);
    
    string tableNameStr(tableName);
    int pageNum = handle->getPageNum();
    vector<shared_ptr<RM_Record> > records;
    fprintf(stdout, "TABLE %s:\n", tableName);
    for (int i = 0; i < pageNum; ++i) {
        handle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<RM_Record> rec = records[j];
            char *pData = rec->getData();
            string tableNameTmp(pData);
            if (tableNameStr != tableNameTmp)
                continue;
            fprintf(stdout, "  attrName: %s, ", pData + MAX_NAME_LEN);
            BufType data = (BufType)(pData + MAX_NAME_LEN * 2);
            fprintf(stdout, "attrType: ");
            switch (data[1]) {
                case INTEGER: fprintf(stdout, "int, "); break;
                case FLOAT: fprintf(stdout, "float, "); break;
                case STRING: fprintf(stdout, "string, "); break;
                default: break;
            }
            fprintf(stdout, "attrLength: %d\n", data[2]);
        }
    }
    mRMManager->closeFile(handle);
    return true;
}
