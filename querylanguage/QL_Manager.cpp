#include <cmath>

#include "QL_Manager.h"

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
    
}

bool QL_Manager::insert(const char *tableName, 
            const std::vector<Value> &values)
{
    vector<AttrInfoEx> attrs;
    getAttrInfoEx(tableName, attrs);

    if (values.size() != attrs.size()) {
        return false;
    }
    for (int i = 0; i < attrs.size(); ++i) {
        if (attrs[i].attrType != values[i].attrType) {
            return false;
        }
    }

    int total = 0;
    for (int i = 0; i < attrs.size(); ++i) {
        total += attrs[i].attrLength;
    }
    char *tData = new char[total];
    for (int i = 0; i < attrs.size(); ++i) {
        memcpy(tData + attrs[i].offset, values[i].data, attrs[i].attrLength);
    }

    RM_FileHandle *fileHandle;
    string DBName = mSMManager->getDBName();
    string fullTableName = DBName + "/" + string(tableName);
    mRMManager->openFile(fullTableName.c_str(), fileHandle);
    RID rid;
    fileHandle->insertRec(tData, rid);
    mRMManager->closeFile(fileHandle);

    int indexNo = getIndexNo(tableName);

    if (indexNo != -1) {
        IX_IndexHandle *indexHandle;
        mIXManager->openIndex(fullTableName.c_str(), indexNo, indexHandle);
        indexHandle->insertEntry(values[indexNo].data, rid);
        mIXManager->closeIndex(indexHandle);
    }

    return true;
}

bool QL_Manager::remove(const char *tableName, 
            const std::vector<Condition> &conditions)
{
    string DBName = mSMManager->getDBName();
    string fullTableName = DBName + "/" + string(tableName);

    int indexNo = getIndexNo(tableName);
    vector<AttrInfoEx> attrs;
    getAttrInfoEx(tableName, attrs);
    int offset = -1;
    if (indexNo != -1) {
        offset = attrs[indexNo].offset;
    }

    RM_FileHandle *fileHandle;
    IX_IndexHandle *indexHandle;
    mRMManager->openFile(fullTableName.c_str(), fileHandle);
    if (indexNo != -1) {
        mIXManager->openIndex(fullTableName.c_str(), indexNo, indexHandle);
    }
    int pageNum = fileHandle->getPageNum();
    vector<shared_ptr<RM_Record> > records;
    int conditionNum = conditions.size();
    vector<int> conditionIndex;
    for (int i = 0; i < conditionNum; ++i) {
        for (int j = 0; j < attrs.size(); ++j) {
            if (string(conditions[i].lAttr.attrName) == string(attrs[j].attrName)) {
                conditionIndex.push_back(j);
                break;
            }
        }
        if (conditionIndex.size() != i + 1) {
            fprintf(stderr, "Can't find the attribute %s\n", conditions[i].lAttr.attrName);
            return false;
        }
    }
    for (int i = 0; i < pageNum; ++i) {
        fileHandle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            bool flag = true;
            for (int k = 0; k < conditionNum; ++k) {
                if (!satisfyCondition(records[i], conditions[k], attrs[conditionIndex[k]])) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                fileHandle->deleteRec(records[i]->getRid());
                if (indexNo != -1) {
                    indexHandle->deleteEntry(records[i]->getData() + offset, records[i]->getRid());
                }
            }
        }
    }
    mRMManager->closeFile(fileHandle);
    if (indexNo != -1) {
        mIXManager->closeIndex(indexHandle);
    }
}

bool QL_Manager::update(const char *tableName, 
            const TableAttr &lAttr, 
            const bool rIsValue,
            const TableAttr &rAttr,
            const Value &rValue,
            const std::vector<Condition> &conditions)
{
    
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
    for (int i = 0; i < pageNum; ++i) {
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

void QL_Manager::getAttrInfoEx(const char *tableName, vector<AttrInfoEx> &attrs) {
    string DBName = mSMManager->getDBName();
    string attrcat = DBName + "/attrcat";
    RM_FileHandle *fileHandle;
    mRMManager->openFile(attrcat.c_str(), fileHandle);
    string tableNameStr(tableName);
    int pageNum = fileHandle->getPageNum();
    vector<shared_ptr<RM_Record> > records;
    for (int i = 0; i < pageNum; ++i) {
        fileHandle->getAllRecFromPage(i, records);
        int num = records.size();
        for (int j = 0; j < num; ++j) {
            shared_ptr<RM_Record> rec = records[j];
            char *pData = rec->getData();
            string tableNameTmp(pData);
            if (tableNameStr != tableNameTmp)
                continue;
            
            AttrInfoEx info;
            info.attrName = pData + MAX_NAME_LEN;
            BufType data = (BufType)(pData + MAX_NAME_LEN * 2);
            info.offset = data[0];
            info.attrType = (AttrType)data[1];
            info.attrLength = data[2];
            info.indexNo = data[3];
            info.nullable = (bool)data[4];
            attrs.push_back(info);
        }
    }
    mRMManager->closeFile(fileHandle);
}