#include <cmath>

#include "QL_Manager.h"

using namespace std;

extern const int MAX_NAME_LEN;
extern const double EPS;

QL_Manager::QL_Manager(SM_Manager *sm, IX_Manager *ix, RM_Manager *rm) {
    mSMManager = sm;
    mIXManager = ix;
    mRMManager = rm;
}

QL_Manager::~QL_Manager() {

}

bool QL_Manager::select(const std::vector<TableAttr> &attrs, 
                const std::vector<const char *> &tables, 
                const std::vector<Condition> &conditions)
{
    string DBName = mSMManager->getDBName();
    RM_FileHandle *fileHandle;
    vector<AttrInfoEx> attrInfos;

    int conditionNum = conditions.size();
    vector<int> conditionIndex;

    vector<shared_ptr<RM_Record> > *selected = new vector<shared_ptr<RM_Record> >[tables.size()];

    for (int index = 0; index < tables.size(); ++index) {
        const char *tableName = tables[index];
        string fullTableName = DBName + "/" + string(tableName);
        getAttrInfoEx(tableName, attrInfos);

        conditionIndex.clear();
        for (int i = 0; i < conditionNum; ++i) {
            bool flag = false;
            for (int j = 0; j < attrInfos.size(); ++j) {
                if (string(conditions[i].lAttr.attrName) == string(attrInfos[j].attrName)) {
                    flag = true;
                    conditionIndex.push_back(j);
                    break;
                }
            }
            if (!flag) {
                conditionIndex.push_back(-1);
            }
        }

        mRMManager->openFile(fullTableName.c_str(), fileHandle);
        int pageNum = fileHandle->getPageNum();
        vector<shared_ptr<RM_Record> > records;
        for (int i = 1; i < pageNum; ++i) {
            fileHandle->getAllRecFromPage(i, records);
            int num = records.size();
            //fprintf(stdout, "page = %d, size = %d\n", i, num);
            for (int j = 0; j < num; ++j) {
                bool flag = true;
                for (int k = 0; k < conditionNum; ++k) {
                    if (conditionIndex[k] != -1 && !satisfyCondition(records[i], conditions[k], attrInfos[conditionIndex[k]])) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    shared_ptr<RM_Record> ptr(new RM_Record(*records[j]));
                    selected[index].push_back(ptr);
                }
            }
        }
        mRMManager->closeFile(fileHandle);
    }
    //fprintf(stdout, "selected size = %d\n", (int)selected[0].size());
    if (tables.size() == 1) {
        const char *tableName = tables[0];
        string fullTableName = DBName + "/" + string(tableName);
        getAttrInfoEx(tableName, attrInfos);
        int *attrIndex = new int[attrs.size()];
        for (int i = 0; i < attrs.size(); ++i) {
            attrIndex[i] = -1;
            for (int j = 0; j < attrInfos.size(); ++j) {
                if (string(attrs[i].attrName) == string(attrInfos[j].attrName)) {
                    attrIndex[i] = j;
                }
            }
            if (attrIndex[i] == -1) {
                fprintf(stderr, "select failed: can't find selected attribute %s\n", attrs[i].attrName);
                return false;
            }
        }
        for (int i = 0; i < attrs.size(); ++i) {
            fprintf(stdout, "%-25s ", attrs[i].attrName);
        }
        fprintf(stdout, "\n");
        int num = selected[0].size();
        for (int i = 0; i < num; ++i) {
            shared_ptr<RM_Record> ptr = selected[0][i];
            for (int j = 0; j < attrs.size(); ++j) {
                AttrType attrType = attrInfos[attrIndex[j]].attrType;
                int offset = attrInfos[attrIndex[j]].offset;
                if (attrType == INTEGER) {
                    fprintf(stdout, "%-25d ", *(int *)(ptr->getData() + offset));
                } else if (attrType == FLOAT) {
                    fprintf(stdout, "%-25.6f ", *(float *)(ptr->getData() + offset));
                } else if (attrType == STRING) {
                    fprintf(stdout, "%-25s ", (char *)(ptr->getData() + offset));
                }
            }
            fprintf(stdout, "\n");
        }
        delete[] attrIndex;
    }
    delete[] selected;
    return true;
}

bool QL_Manager::insert(const char *tableName, 
            const vector<vector<Value> > &allValues)
{
    //cout << "enter QL_Manager::insert" << endl;
    vector<AttrInfoEx> attrInfos;
    getAttrInfoEx(tableName, attrInfos);

    RM_FileHandle *fileHandle;
    string DBName = mSMManager->getDBName();
    string fullTableName = DBName + "/" + string(tableName);
    mRMManager->openFile(fullTableName.c_str(), fileHandle);

    int indexNo = getIndexNo(tableName);
    IX_IndexHandle *indexHandle;
    if (indexNo != -1) {
        mIXManager->openIndex(fullTableName.c_str(), indexNo, indexHandle);
    }

    int valueNum = allValues.size();
    for (int valueIndex = 0; valueIndex < valueNum; ++valueIndex) {
        const vector<Value> &values = allValues[valueIndex];
        if (values.size() != attrInfos.size()) {
            fprintf(stderr, "insert failed: wrong attributes size %d, expected %d\n", (int)values.size(), (int)attrInfos.size());
            continue;
        }
        bool typeFlag = false;
        bool nullFlag = false;
        for (int i = 0; i < attrInfos.size(); ++i) {
            if (values[i].attrType != NOTYPE && attrInfos[i].attrType != values[i].attrType) {
                fprintf(stderr, "insert failed: wrong attibute type of %s.\n", attrInfos[i].attrName.c_str());
                typeFlag = true;
                break;
            }
            if (values[i].attrType == NOTYPE && !attrInfos[i].nullable) {
                fprintf(stderr, "insert failed: attribute %s can't be NULL.\n", attrInfos[i].attrName.c_str());
                nullFlag = true;
                break;
            }
        }
        if (typeFlag || nullFlag) {
            continue;
        }

        int total = 0;
        for (int i = 0; i < attrInfos.size(); ++i) {
            total += attrInfos[i].attrLength;
        }
        char *tData = new char[total];
        for (int i = 0; i < attrInfos.size(); ++i) {
            memcpy(tData + attrInfos[i].offset, values[i].data, attrInfos[i].attrLength);
        }

        RID rid;
        fileHandle->insertRec(tData, rid);
        delete []tData;

        if (indexNo != -1) {
            indexHandle->insertEntry(values[indexNo].data, rid);
        }
    } 

    mRMManager->closeFile(fileHandle);
    if (indexNo != -1) {
        mIXManager->closeIndex(indexHandle);
    }

    //cout << "leave QL_Manager::insert" << endl;
    return true;
}

bool QL_Manager::remove(const char *tableName, 
            const std::vector<Condition> &conditions)
{
    //cout << "enter QL_Manager::remove" << endl;
    string DBName = mSMManager->getDBName();
    string fullTableName = DBName + "/" + string(tableName);

    int indexNo = getIndexNo(tableName);
    vector<AttrInfoEx> attrInfos;
    getAttrInfoEx(tableName, attrInfos);
    int offset = -1;
    if (indexNo != -1) {
        offset = attrInfos[indexNo].offset;
    }
    vector<shared_ptr<RM_Record> > records;
    RM_FileHandle *fileHandle;
    IX_IndexHandle *indexHandle;
    mRMManager->openFile(fullTableName.c_str(), fileHandle);
    // fileHandle->getAllRecFromPage(1, records);
    // fprintf(stdout, "size = %d\n", (int)records.size());
    if (indexNo != -1) {
        mIXManager->openIndex(fullTableName.c_str(), indexNo, indexHandle);
    }
    int conditionNum = conditions.size();
    vector<int> conditionIndex;
    for (int i = 0; i < conditionNum; ++i) {
        for (int j = 0; j < attrInfos.size(); ++j) {
            if (string(conditions[i].lAttr.attrName) == string(attrInfos[j].attrName)) {
                conditionIndex.push_back(j);
                break;
            }
        }
        if (conditionIndex.size() != i + 1) {
            fprintf(stderr, "delete failed: can't find the attribute %s\n", conditions[i].lAttr.attrName);
            return false;
        }
    }
    int pageNum = fileHandle->getPageNum();
    
    for (int i = 1; i < pageNum; ++i) {
        fileHandle->getAllRecFromPage(i, records);
        int num = records.size();
        //fprintf(stdout, "page = %d, size = %d\n", i, num);
        for (int j = 0; j < num; ++j) {
            bool flag = true;
            for (int k = 0; k < conditionNum; ++k) {
                if (!satisfyCondition(records[i], conditions[k], attrInfos[conditionIndex[k]])) {
                    flag = false;
                    break;
                }
            }
            //fprintf(stdout, "delete %d ? %d\n", j, flag);
            if (flag) {
                fileHandle->deleteRec(records[j]->getRid());
                if (indexNo != -1) {
                    indexHandle->deleteEntry(records[j]->getData() + offset, records[j]->getRid());
                }
            }
        }
    }
    mRMManager->closeFile(fileHandle);
    if (indexNo != -1) {
        mIXManager->closeIndex(indexHandle);
    }
    //cout << "leave QL_Manager::remove" << endl;
    return true;
}

bool QL_Manager::update(const char *tableName, 
            const TableAttr &attr, 
            const Value &value,
            const std::vector<Condition> &conditions)
{
    string DBName = mSMManager->getDBName();
    string fullTableName = DBName + "/" + string(tableName);

    int indexNo = getIndexNo(tableName);
    vector<AttrInfoEx> attrInfos;
    getAttrInfoEx(tableName, attrInfos);
    int offset = -1;
    if (indexNo != -1) {
        offset = attrInfos[indexNo].offset;
    }

    RM_FileHandle *fileHandle;
    IX_IndexHandle *indexHandle;
    mRMManager->openFile(fullTableName.c_str(), fileHandle);
    if (indexNo != -1) {
        mIXManager->openIndex(fullTableName.c_str(), indexNo, indexHandle);
    }
    int conditionNum = conditions.size();
    vector<int> conditionIndex;
    for (int i = 0; i < conditionNum; ++i) {
        for (int j = 0; j < attrInfos.size(); ++j) {
            if (string(conditions[i].lAttr.attrName) == string(attrInfos[j].attrName)) {
                conditionIndex.push_back(j);
                break;
            }
        }
        if (conditionIndex.size() != i + 1) {
            fprintf(stderr, "update failed: can't find the attribute %s\n", conditions[i].lAttr.attrName);
            return false;
        }
    }
    int updateIndex = -1;
    for (int i = 0; i < attrInfos.size(); ++i) {
        //cout << string(attr.attrName) << " " << string(attrInfos[i].attrName) << endl;
        if (string(attr.attrName) == string(attrInfos[i].attrName)) {
            updateIndex = i;
            break;
        }
    }
    if (updateIndex == -1) {
        fprintf(stderr, "update failed: can't find the updated attribute %s\n", attr.attrName);
        return false;
    }
    int pageNum = fileHandle->getPageNum();
    vector<shared_ptr<RM_Record> > records;
    for (int i = 1; i < pageNum; ++i) {
        fileHandle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            bool flag = true;
            for (int k = 0; k < conditionNum; ++k) {
                if (!satisfyCondition(records[j], conditions[k], attrInfos[conditionIndex[k]])) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                fileHandle->deleteRec(records[j]->getRid());
                if (indexNo != -1) {
                    indexHandle->deleteEntry(records[j]->getData() + offset, records[j]->getRid());
                }
                char *uData = records[j]->getData();
                memcpy(uData + attrInfos[updateIndex].offset, value.data, attrInfos[updateIndex].attrLength);
                RID rid;
                fileHandle->insertRec(uData, rid);
                if (indexNo != -1) {
                    indexHandle->insertEntry(records[j]->getData() + offset, rid);
                }
            }
        }
    }
    mRMManager->closeFile(fileHandle);
    if (indexNo != -1) {
        mIXManager->closeIndex(indexHandle);
    }
    return true;
}

bool QL_Manager::satisfyCondition(shared_ptr<RM_Record> ptrRec,
                          Condition condition, AttrInfoEx info)
{
    if (condition.rIsValue) {
        if (condition.op == NO_OP || condition.rValue.data == NULL) {
            return true;
        }
        char *data = ptrRec->getData();
        data = data + info.offset;
        if (info.attrType == INTEGER) {
            int attrValue = *((int *)data);
            int intValue = *((int *)condition.rValue.data);
            switch (condition.op) {
                case EQ_OP: return attrValue == intValue;
                case LT_OP: return attrValue < intValue;
                case GT_OP: return attrValue > intValue;
                case LE_OP: return attrValue <= intValue;
                case GE_OP: return attrValue >= intValue;
                case NE_OP: return attrValue != intValue;
                default: return false;
            }
        } else if (info.attrType == FLOAT) {
            float attrValue = *((float *)data);
            float floatValue = *((float *)condition.rValue.data);
            switch (condition.op) {
                case EQ_OP: return fabs(attrValue - floatValue) < EPS;
                case LT_OP: return attrValue < floatValue;
                case GT_OP: return attrValue > floatValue;
                case LE_OP: return attrValue <= floatValue;
                case GE_OP: return attrValue >= floatValue;
                case NE_OP: return fabs(attrValue - floatValue) > EPS;
                default: return false;
            }
        } else if (info.attrType == STRING) {
            char *stringValue = (char *)condition.rValue.data;
            int cmpResult = 0;
            for (int i = 0; i < info.attrLength; ++i) {
                if (data[i] < stringValue[i]) {
                    cmpResult = -1;
                    break;
                } else if (data[i] > stringValue[i]) {
                    cmpResult = 1;
                    break;
                }
            }
            switch (condition.op) {
                case EQ_OP: return cmpResult == 0;
                case LT_OP: return cmpResult == -1;
                case GT_OP: return cmpResult == 1;
                case LE_OP: return cmpResult <= 0;
                case GE_OP: return cmpResult >= 0;
                case NE_OP: return cmpResult != 0;
                default: return false;
            }
        }
    }
    return false;
}

int QL_Manager::getIndexNo(const char *tableName) {
    string DBName = mSMManager->getDBName();
    string relcat = DBName + "/relcat";
    RM_FileHandle *fileHandle;
    mRMManager->openFile(relcat.c_str(), fileHandle);

    int indexNo = -1;
    string tableNameStr(tableName);
    int pageNum = fileHandle->getPageNum();
    vector<shared_ptr<RM_Record> > records;
    for (int i = 1; i < pageNum; ++i) {
        fileHandle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<RM_Record> rec = records[j];
            char *pData = rec->getData();
            string tableNameTmp(pData);
            if (tableNameStr != tableNameTmp)
                continue;
            BufType data = (BufType)(pData + MAX_NAME_LEN);
            indexNo = data[2];
        }
    }
    mRMManager->closeFile(fileHandle);
    return indexNo;
}

void QL_Manager::getAttrInfoEx(const char *tableName, vector<AttrInfoEx> &attrInfos) {
    string DBName = mSMManager->getDBName();
    string attrcat = DBName + "/attrcat";
    RM_FileHandle *fileHandle;
    mRMManager->openFile(attrcat.c_str(), fileHandle);
    string tableNameStr(tableName);
    int pageNum = fileHandle->getPageNum();
    vector<shared_ptr<RM_Record> > records;
    attrInfos.clear();
    for (int i = 1; i < pageNum; ++i) {
        fileHandle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<RM_Record> rec = records[j];
            char *pData = rec->getData();
            string tableNameTmp(pData);
            if (tableNameStr != tableNameTmp)
                continue;
            
            AttrInfoEx info;
            //cout << "attrName: " << (pData + MAX_NAME_LEN) << endl;
            info.attrName = string(pData + MAX_NAME_LEN);
            BufType data = (BufType)(pData + MAX_NAME_LEN * 2);
            info.offset = data[0];
            info.attrType = (AttrType)data[1];
            info.attrLength = data[2];
            info.indexNo = data[3];
            info.nullable = (bool)data[4];
            attrInfos.push_back(info);
        }
    }
    mRMManager->closeFile(fileHandle);
}