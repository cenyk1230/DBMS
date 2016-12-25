#include <iostream>
#include <cmath>
#include <cassert>
#include <algorithm>

#include "QL_Manager.h"

using namespace std;

extern const int MAX_NAME_LEN;
extern const int MAX_ATTR_LEN;
extern const double EPS;

QL_Manager::QL_Manager(SM_Manager *sm, IX_Manager *ix, RM_Manager *rm) {
    mSMManager = sm;
    mIXManager = ix;
    mRMManager = rm;
}

QL_Manager::~QL_Manager() {

}

vector<shared_ptr<RM_Record> > *selected;
static int offset1;
static int offset2;
static int offset3;
static int ti1, ti2, ti3;

bool cmp1(int i, int j) {
    int v1 = *(int *)(selected[ti1][i]->getData() + offset1);
    int v2 = *(int *)(selected[ti1][j]->getData() + offset1);
    return v1 < v2;
}

bool cmp2(const shared_ptr<RM_Record> &p1, const shared_ptr<RM_Record> &p2) {
    int v1 = *(int *)(p1->getData() + offset2);
    int v2 = *(int *)(p2->getData() + offset2);
    return v1 < v2;
}

bool cmp3(const shared_ptr<RM_Record> &p1, const shared_ptr<RM_Record> &p2) {
    int v1 = *(int *)(p1->getData() + offset3);
    int v2 = *(int *)(p2->getData() + offset3);
    return v1 < v2;
}

bool QL_Manager::select(std::vector<TableAttr> &attrs, 
                const std::vector<const char *> &tables, 
                std::vector<Condition> &conditions)
{
    string DBName = mSMManager->getDBName();
    if (DBName == "") {
        fprintf(stderr, "select failed: please USE database first.\n");
        return false;
    }
    RM_FileHandle *fileHandle;
    vector<AttrInfoEx> attrInfos;

    int conditionNum = conditions.size();
    //fprintf(stdout, "condition num = %d\n", (int)conditions.size());
    vector<int> conditionIndex;

    selected = new vector<shared_ptr<RM_Record> >[tables.size()];

    //cout << "attrs.size() = " << attrs.size() << endl;

    vector<shared_ptr<string> > saves;
    if (attrs.size() == 0) {
        for (int i = 0; i < tables.size(); ++i) {
            getAttrInfoEx(tables[i], attrInfos);
            for (int j = 0; j < attrInfos.size(); ++j) {
                TableAttr tmpAttr;
                tmpAttr.tableName = tables[i];
                saves.push_back(shared_ptr<string>(new string(attrInfos[j].attrName)));
                tmpAttr.attrName = saves[saves.size() - 1]->c_str();
                attrs.push_back(tmpAttr);
            }
        }
    }

    for (int index = 0; index < tables.size(); ++index) {
        const char *tableName = tables[index];
        string fullTableName = DBName + "/" + string(tableName);
        getAttrInfoEx(tableName, attrInfos);

        conditionIndex.clear();
        for (int i = 0; i < conditionNum; ++i) {
            bool flag = false;
            for (int j = 0; j < attrInfos.size(); ++j) {
                if (conditions[i].rIsValue && string(conditions[i].lAttr.attrName) == string(attrInfos[j].attrName)) {
                    flag = true;
                    conditionIndex.push_back(j);
                    break;
                }
            }
            if (!flag) {
                conditionIndex.push_back(-1);
            }
            //fprintf(stdout, "conditionIndex = %d\n", conditionIndex[i]);
        }
        //fprintf(stdout, "tableName = %s\n", fullTableName.c_str());
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
                    if (conditionIndex[k] != -1 && !satisfyCondition(records[j], conditions[k], attrInfos[conditionIndex[k]])) {
                        flag = false;
                        break;
                    }
                }
                //fprintf(stdout, "flag = %d\n", flag);
                if (flag) {
                    shared_ptr<RM_Record> ptr(new RM_Record(*records[j]));
                    selected[index].push_back(ptr);
                }
            }
        }
        mRMManager->closeFile(fileHandle);
    }
    //fprintf(stdout, "selected[0] size = %d\n", (int)selected[0].size());
    if (tables.size() == 1) {
        const char *tableName = tables[0];
        string fullTableName = DBName + "/" + string(tableName);
        //cout << tableName << endl;
        getAttrInfoEx(tableName, attrInfos);
        int *attrIndex = new int[attrs.size()];
        for (int i = 0; i < attrs.size(); ++i) {
            attrIndex[i] = -1;
            for (int j = 0; j < attrInfos.size(); ++j) {
                //cout << attrs[i].attrName << " " << attrInfos[j].attrName << endl;
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
            fprintf(stdout, "%s", attrs[i].attrName);
            if (i + 1 != attrs.size())
                fprintf(stdout, ",");
        }
        fprintf(stdout, "\n");
        int num = selected[0].size();
        for (int i = 0; i < num; ++i) {
            shared_ptr<RM_Record> ptr = selected[0][i];
            for (int j = 0; j < attrs.size(); ++j) {
                AttrType attrType = attrInfos[attrIndex[j]].attrType;
                int offset = attrInfos[attrIndex[j]].offset;
                if (attrType == INTEGER) {
                    int tmp = *(int *)(ptr->getData() + offset);
                    if (tmp != -1)
                        fprintf(stdout, "%d", tmp);
                    else
                        fprintf(stdout, "%s", "null");
                } else if (attrType == FLOAT) {
                    fprintf(stdout, "%.6f", *(float *)(ptr->getData() + offset));
                } else if (attrType == STRING) {
                    char *tmp = (char *)(ptr->getData() + offset);
                    if (attrInfos[attrIndex[j]].attrLength == 1) {
                        if (tmp[0] == -1) {
                            fprintf(stdout, "%s", "null");
                        } else {
                            fprintf(stdout, "%c", tmp[0]);
                        }
                    } else {
                        bool isNull = true;
                        for (int k = 0; k < attrInfos[attrIndex[j]].attrLength - 1; ++k) {
                            if (tmp[k] != -1) {
                                isNull = false;
                                break;
                            }
                        }
                        if (!isNull) {
                            string printTmp(tmp, attrInfos[attrIndex[j]].attrLength);
                            fprintf(stdout, "%s", printTmp.c_str());
                        } else 
                            fprintf(stdout, "%s", "null");
                    }
                }
                if (j + 1 != attrs.size())
                    fprintf(stdout, ",");
            }
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "\n");
        delete[] attrIndex;
    } else {
        vector<shared_ptr<RM_Record> > selected0, selected1, selected2;
        vector<AttrInfoEx> *infos = new vector<AttrInfoEx>[tables.size()];
        bool *isJoined = new bool[tables.size()];
        for (int i = 0; i < tables.size(); ++i) {
            isJoined[i] = false;
            getAttrInfoEx(tables[i], infos[i]);
        }
        for (int i = 0; i < conditionNum; ++i) {
            if (conditions[i].rIsValue)
                continue;
            if (conditions[i].op != EQ_OP) {
                fprintf(stdout, "select failed: wrong condition\n");
                delete[] isJoined;
                delete[] infos;
                delete[] selected;
                return false;
            }
            if (conditions[i].lAttr.tableName == NULL) {
                for (int j = 0; j < tables.size(); ++j) {
                    for (int k = 0; k < infos[j].size(); ++k)
                        if (string(conditions[i].lAttr.attrName) == infos[j][k].attrName) {
                            conditions[i].lAttr.tableName = tables[j];
                        }
                }
            }
            if (conditions[i].rAttr.tableName == NULL) {
                for (int j = 0; j < tables.size(); ++j) {
                    for (int k = 0; k < infos[j].size(); ++k)
                        if (string(conditions[i].rAttr.attrName) == infos[j][k].attrName) {
                            conditions[i].rAttr.tableName = tables[j];
                        }
                }
            }
            ti1 = -1, ti2 = -1;
            for (int j = 0; j < tables.size(); ++j) {
                if (string(tables[j]) == string(conditions[i].lAttr.tableName)) {
                    ti1 = j;
                    continue;
                }
                if (string(tables[j]) == string(conditions[i].rAttr.tableName)) {
                    ti2 = j;
                    continue;
                }
            }
            if (ti1 == -1 || ti2 == -1) {
                fprintf(stdout, "select failed: can't find the table in condition\n");
                return false;
            }
            if (isJoined[ti2]) {
                swap(ti1, ti2);
                swap(conditions[i].lAttr, conditions[i].rAttr);
            }
            //cout << ti1 << " " << ti2 << endl;
            ti3 = 3 - ti1 - ti2;
            offset1 = -1;
            offset2 = -1;
            for (int j = 0; j < infos[ti1].size(); ++j) {
                if (string(conditions[i].lAttr.attrName) == infos[ti1][j].attrName) {
                    offset1 = infos[ti1][j].offset;
                }
            }
            for (int j = 0; j < infos[ti2].size(); ++j) {
                if (string(conditions[i].rAttr.attrName) == infos[ti2][j].attrName) {
                    offset2 = infos[ti2][j].offset;
                }
            }
            vector<int> sortIndex;
            for (int j = 0; j < selected[ti1].size(); ++j)
                sortIndex.push_back(j);
            //sort(selected[ti1].begin(), selected[ti1].end(), cmp1);
            sort(sortIndex.begin(), sortIndex.end(), cmp1);
            sort(selected[ti2].begin(), selected[ti2].end(), cmp2);
            selected0.clear();
            selected1.clear();
            selected2.clear();
            int len1 = selected[ti1].size();
            int len2 = selected[ti2].size();
            int j = 0, k = 0;
            while (j < len1 && k < len2) {
                int v1 = *(int *)(selected[ti1][sortIndex[j]]->getData() + offset1);
                int v2 = *(int *)(selected[ti2][k]->getData() + offset2);
                //fprintf(stdout, "j = %d, k = %d, v1 = %d, v2 = %d\n", j, k, v1, v2);
                if (v1 == v2) {
                    selected0.push_back(selected[ti1][sortIndex[j]]);
                    selected1.push_back(selected[ti2][k]);
                    if (isJoined[ti1] && tables.size() > 2) {
                        selected2.push_back(selected[ti3][sortIndex[j]]);
                    }
                    ++j;
                    ++k;
                } else if (v1 < v2) {
                    ++j;
                } else {
                    ++k;
                }
            }
            selected[ti1].clear();
            selected[ti2].clear();
            if (isJoined[ti1] && tables.size() > 2) {
                selected[ti3].clear();
            }
            int len = selected0.size();
            for (int j = 0; j < len; ++j) {
                selected[ti1].push_back(selected0[j]);
                selected[ti2].push_back(selected1[j]);
                if (isJoined[ti1] && tables.size() > 2) {
                    selected[ti3].push_back(selected2[j]);
                }
            }
            isJoined[ti1] = true;
            isJoined[ti2] = true;
        }
        int num = selected[0].size();
        //fprintf(stdout, "num = %d\n", (int)selected[0].size());
        for (int i = 1; i < tables.size(); ++i) {
            assert((int)selected[i].size() == num);
        }
        vector<pair<int, int> > attrIndexes;
        for (int i = 0; i < attrs.size(); ++i) {
            if (attrs[i].tableName != NULL) {
                fprintf(stdout, "%s", (string(attrs[i].tableName) + "." + string(attrs[i].attrName)).c_str());
            } else {
                fprintf(stdout, "%s", attrs[i].attrName);
            }
            if (i + 1 != attrs.size())
                fprintf(stdout, ",");
            for (int j = 0; j < tables.size(); ++j)
                if (attrs[i].tableName == NULL || string(attrs[i].tableName) == string(tables[j])) {
                    bool flag = false;
                    for (int k = 0; k < infos[j].size(); ++k) {
                        //cout << "attrName: " << string(attrs[i].attrName) << " " << string(infos[j][k].attrName) << endl;
                        if (string(attrs[i].attrName) == string(infos[j][k].attrName)) {
                            flag = true;
                            attrIndexes.push_back(make_pair(j, k));
                            break;
                        }
                    }
                    if (flag) {
                        break;
                    }
            }
        }
        fprintf(stdout, "\n");
        assert(attrIndexes.size() == attrs.size());
        for (int i = 0; i < num; ++i) {
            for (int j = 0; j < attrs.size(); ++j) {
                int x = attrIndexes[j].first;
                int y = attrIndexes[j].second;
                if (infos[x][y].attrType == INTEGER) {
                    int tmp = *(int *)(selected[x][i]->getData() + infos[x][y].offset);
                    if (tmp != -1)
                        fprintf(stdout, "%d", tmp);
                    else
                        fprintf(stdout, "%s", "null");
                } else if (infos[x][y].attrType == FLOAT) {
                    fprintf(stdout, "%.6f", *(float *)(selected[x][i]->getData() + infos[x][y].offset));
                } else if (infos[x][y].attrType == STRING) {
                    char *tmp = (char *)(selected[x][i]->getData() + infos[x][y].offset);
                    if (infos[x][y].attrLength == 1) {
                        if (tmp[0] == -1) {
                            fprintf(stdout, "%s", "null");
                        } else {
                            fprintf(stdout, "%c", tmp[0]);
                        }
                    } else {
                        bool isNull = true;
                        for (int k = 0; k < infos[x][y].attrLength - 1; ++k) {
                            if (tmp[k] != -1) {
                                isNull = false;
                                break;
                            }
                        }
                        if (!isNull)
                            fprintf(stdout, "%s", tmp);
                        else 
                            fprintf(stdout, "%s", "null");
                    }
                }
                if (j + 1 != attrs.size())
                    fprintf(stdout, ",");
            }
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "\n");
        delete[] isJoined;
        delete[] infos;
    }
    delete[] selected;
    return true;
}

bool QL_Manager::selectGB(const std::vector<TableAttrEx> &attrs,
              const char *tableName,
              const char *attrName,
              const std::vector<Condition> &conditions)
{
    string DBName = mSMManager->getDBName();
    if (DBName == "") {
        fprintf(stderr, "select failed: please USE database first.\n");
        return false;
    }
    RM_FileHandle *fileHandle;
    vector<AttrInfoEx> attrInfos;

    int conditionNum = conditions.size();
    //fprintf(stdout, "condition num = %d\n", (int)conditions.size());
    vector<int> conditionIndex;

    vector<shared_ptr<RM_Record> > selectedGB;

    string fullTableName = DBName + "/" + string(tableName);
    getAttrInfoEx(tableName, attrInfos);

    conditionIndex.clear();
    for (int i = 0; i < conditionNum; ++i) {
        bool flag = false;
        for (int j = 0; j < attrInfos.size(); ++j) {
            if (conditions[i].rIsValue && string(conditions[i].lAttr.attrName) == string(attrInfos[j].attrName)) {
                flag = true;
                conditionIndex.push_back(j);
                break;
            }
        }
        if (!flag) {
            conditionIndex.push_back(-1);
        }
        //fprintf(stdout, "conditionIndex = %d\n", conditionIndex[i]);
    }
    //fprintf(stdout, "tableName = %s\n", fullTableName.c_str());
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
                if (conditionIndex[k] != -1 && !satisfyCondition(records[j], conditions[k], attrInfos[conditionIndex[k]])) {
                    flag = false;
                    break;
                }
            }
            //fprintf(stdout, "flag = %d\n", flag);
            if (flag) {
                shared_ptr<RM_Record> ptr(new RM_Record(*records[j]));
                selectedGB.push_back(ptr);
            }
        }
    }
    mRMManager->closeFile(fileHandle);

    int groupIndex = -1;
    for (int i = 0; i < attrInfos.size(); ++i)
        if (string(attrName) == attrInfos[i].attrName) {
            groupIndex = i;
            break;
        }
    if (groupIndex == -1) {
        fprintf(stdout, "select failed: can't find the grouped attribute");
        return false;
    }

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

    offset3 = attrInfos[groupIndex].offset;
    sort(selectedGB.begin(), selectedGB.end(), cmp3);

    vector<vector<Value> > res;
    vector<Value> tmpRes;
    int num = selectedGB.size();
    for (int i = 0, j; i < num; ++i) {
        j = i;
        while (j + 1 < num) {
            if (attrInfos[groupIndex].attrType == INTEGER) {
                int v1 = *(int *)(selectedGB[i]->getData() + offset3);
                int v2 = *(int *)(selectedGB[j + 1]->getData() + offset3);
                if (v1 == v2) {
                    ++j;
                } else {
                    break;
                }
            } else if (attrInfos[groupIndex].attrType == STRING) {
                char *c1 = selectedGB[i]->getData() + offset3;
                char *c2 = selectedGB[j + 1]->getData() + offset3;
                if (strncmp(c1, c2, attrInfos[groupIndex].attrLength) == 0) {
                    ++j;
                } else {
                    break;
                }
            }
            
        }
        tmpRes.clear();
        for (int z = 0; z < attrs.size(); ++z) {
            if (attrs[z].func == NO_FUNC) {
                Value value;
                if (attrInfos[attrIndex[z]].attrType == INTEGER) {
                    value.attrType = INTEGER;
                    int *intValue = new int;
                    *intValue = *(int *)(selectedGB[i]->getData() + attrInfos[attrIndex[z]].offset);
                    value.data = intValue;
                    tmpRes.push_back(value);
                    continue;
                } else if (attrInfos[attrIndex[z]].attrType == STRING) {
                    value.attrType = STRING;
                    char *stringValue = new char[attrInfos[attrIndex[z]].attrLength];
                    memcpy(stringValue, selectedGB[i]->getData() + attrInfos[attrIndex[z]].offset, attrInfos[attrIndex[z]].attrLength);
                    value.data = stringValue;
                    tmpRes.push_back(value);
                    continue;
                }
            }
            int minAttr = 1000000000, maxAttr = -1;
            long long sumAttr = 0;
            for (int k = i; k <= j; ++k) {
                int v = *(int *)(selectedGB[k]->getData() + attrInfos[attrIndex[z]].offset);
                if (v > maxAttr) {
                    maxAttr = v;
                }
                if (v < minAttr) {
                    minAttr = v;
                }
                sumAttr += v;
            }
            if (attrs[z].func == AVG_FUNC) {
                Value value;
                value.attrType = FLOAT;
                float *floatValue = new float(sumAttr * 1.0 / (j - i + 1));
                value.data = floatValue;
                tmpRes.push_back(value);
            } else if (attrs[z].func == SUM_FUNC) {
                Value value;
                value.attrType = LONGLONG;
                long long *longValue = new long long;
                *longValue = sumAttr;
                value.data = longValue;
                tmpRes.push_back(value);
            } else if (attrs[z].func == MIN_FUNC) {
                Value value;
                value.attrType = INTEGER;
                int *intValue = new int;
                *intValue = minAttr;
                value.data = intValue;
                tmpRes.push_back(value);
            } else if (attrs[z].func == MAX_FUNC) {
                Value value;
                value.attrType = INTEGER;
                int *intValue = new int;
                *intValue = maxAttr;
                value.data = intValue;
                tmpRes.push_back(value);
            }
        }
        res.push_back(tmpRes);
        i = j;
    }
    
    for (int i = 0; i < attrs.size(); ++i) {
        fprintf(stdout, "%s", attrs[i].attrName);
        if (i + 1 != attrs.size())
            fprintf(stdout, ",");
    }
    fprintf(stdout, "\n");

    int resNum = res.size();
    for (int i = 0; i < resNum; ++i) {
        for (int j = 0; j < attrs.size(); ++j) {
            AttrType attrType = res[i][j].attrType;
            if (attrType == INTEGER) {
                int tmp = *(int *)res[i][j].data;
                fprintf(stdout, "%d", tmp);
            } else if (attrType == FLOAT) {
                float tmp = *(float *)res[i][j].data;
                fprintf(stdout, "%.6f", tmp);
            } else if (attrType == LONGLONG) {
                long long tmp = *(long long *)res[i][j].data;
                fprintf(stdout, "%lld", tmp);
            } else if (attrType == STRING) {
                fprintf(stdout, "%s", (char *)res[i][j].data);
            }
            if (j + 1 != attrs.size())
                fprintf(stdout, ",");
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");

    delete[] attrIndex;
    return true;
}

bool QL_Manager::insert(const char *tableName, 
            const vector<vector<Value> > &allValues)
{
    //cout << "enter QL_Manager::insert" << endl;
    string DBName = mSMManager->getDBName();
    if (DBName == "") {
        fprintf(stderr, "insert failed: please USE database first.\n");
        return false;
    }
    vector<AttrInfoEx> attrInfos;
    getAttrInfoEx(tableName, attrInfos);

    RM_FileHandle *fileHandle;
    string fullTableName = DBName + "/" + string(tableName);
    mRMManager->openFile(fullTableName.c_str(), fileHandle);

    int indexNo = getIndexNo(tableName);
    //fprintf(stdout, "indexNo = %d\n", indexNo);
    IX_IndexHandle *indexHandle;
    if (indexNo != -1) {
        mIXManager->openIndex(fullTableName.c_str(), indexNo, indexHandle);
    }
    vector<int> rangeL, rangeR;
    for (int i = 0; i < attrInfos.size(); ++i) {
        if (attrInfos[i].isForeignKey) {
            int l, r;
            getRange(attrInfos[i].foreignTable, attrInfos[i].foreignAttr, l, r);
            rangeL.push_back(l);
            rangeR.push_back(r);
        } else {
            rangeL.push_back(-1);
            rangeR.push_back(1000000000);
        }
    }
    int valueNum = allValues.size();
    for (int valueIndex = 0; valueIndex < valueNum; ++valueIndex) {
        const vector<Value> &values = allValues[valueIndex];
        // for (int i = 0; i < values.size(); ++i) {
        //     if (attrInfos[i].attrType == INTEGER) {
        //         if (values[i].data != NULL) {
        //             fprintf(stdout, "i = %d, value = %d\n", i, *(int *)values[i].data);
        //         } else {
        //             fprintf(stdout, "i = %d, value = null\n", i);
        //         }
        //     }
        //     if (attrInfos[i].attrType == STRING) {
        //         if (values[i].data != NULL) {
        //             fprintf(stdout, "i = %d, value = %s\n", i, (char *)values[i].data);
        //         } else {
        //             fprintf(stdout, "i = %d, value = null\n", i);
        //         }
        //     }
        // }
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

        if (indexNo != -1) {
            bool dupFlag = indexHandle->findEntry(values[indexNo].data);
            //fprintf(stdout, "dupFlag = %d\n", dupFlag);
            if (dupFlag) {
                fprintf(stdout, "insert failed: primary key %d has alreday existed\n", *(int *)values[indexNo].data);
                continue;
            }
        }

        bool consFlag = false;
        char *tData = new char[total];
        for (int i = 0; i < attrInfos.size(); ++i) {
            //fprintf(stdout, "i = %d, isCheck = %d\n", i, attrInfos[i].isCheck);
            //cout << i << " " << values[i].data << endl;
            if (values[i].data != NULL) {
                if (attrInfos[i].isCheck) {
                    bool outFlag = true;
                    for (int j = 0; j < attrInfos[i].values.size(); ++j) {
                        if (string((char *)values[i].data) == attrInfos[i].values[j]) {
                            outFlag = false;
                            break;
                        }
                    }
                    if (outFlag) {
                        fprintf(stdout, "insert failed: attribute %s can't be %s\n", attrInfos[i].attrName.c_str(), (char *)values[i].data);
                        consFlag = true;
                        break;
                    }
                }
                if (attrInfos[i].isForeignKey) {
                    int v = *(int *)values[i].data;
                    if (v < rangeL[i] || v > rangeR[i]) {
                        fprintf(stdout, "insert failed: attribute %s should be in foreign key constraint (%d, %d)\n", attrInfos[i].attrName.c_str(), rangeL[i], rangeR[i]);
                        consFlag = true;
                        break;
                    }
                }
                memcpy(tData + attrInfos[i].offset, (char *)values[i].data, attrInfos[i].attrLength);
                //cout << string(tData + attrInfos[i].offset) << endl;
            } else {
                memset(tData + attrInfos[i].offset, 0xFF, attrInfos[i].attrLength);
                if (attrInfos[i].attrType == STRING) {
                    if (attrInfos[i].attrLength == 1) {
                        tData[attrInfos[i].offset + attrInfos[i].attrLength - 1] = 0xFF;
                    } else {
                        tData[attrInfos[i].offset + attrInfos[i].attrLength - 1] = '\0';
                    }
                }
            }
        }

        if (consFlag) {
            continue;
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
    if (DBName == "") {
        fprintf(stderr, "delete failed: please USE database first.\n");
        return false;
    }
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
                if (!satisfyCondition(records[j], conditions[k], attrInfos[conditionIndex[k]])) {
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
    if (DBName == "") {
        fprintf(stderr, "update failed: please USE database first.\n");
        return false;
    }
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

    if (attrInfos[updateIndex].isCheck) {
        bool outFlag = true;
        for (int i = 0; i < attrInfos[updateIndex].values.size(); ++i) {
            if (string((char *)value.data) == attrInfos[updateIndex].values[i]) {
                outFlag = false;
                break;
            }
        }
        if (outFlag) {
            fprintf(stdout, "update failed: attribute %s can't be %s\n", attrInfos[updateIndex].attrName.c_str(), (char *)value.data);
            return false;
        }
    }
    //fprintf(stdout, "isForeignKey = %d\n", attrInfos[updateIndex].isForeignKey);
    if (attrInfos[updateIndex].isForeignKey) {
        int v = *(int *)value.data;
        int l, r;
        getRange(attrInfos[updateIndex].foreignTable, attrInfos[updateIndex].foreignAttr, l, r);
        if (v < l || v > r) {
            fprintf(stdout, "update failed: attribute %s should be in foreign key constraint (%d, %d)\n", attrInfos[updateIndex].attrName.c_str(), l, r);
            return false;
        }
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
    //fprintf(stdout, "rIsValue = %d\n", condition.rIsValue);
    if (condition.rIsValue) {
        if (condition.op == NO_OP) {
            return true;
        }
        char *data = ptrRec->getData();
        data = data + info.offset;
        if (info.attrType == INTEGER) {
            int attrValue = *((int *)data);
            if (condition.rValue.attrType == NOTYPE || condition.rValue.data == NULL) {
                switch (condition.op) {
                    case EQ_OP: return attrValue == -1;
                    case NE_OP: return attrValue != -1;
                    default: return false;
                }
            }
            int intValue = *((int *)condition.rValue.data);
            //fprintf(stdout, "condition data: %d %d\n", attrValue, intValue);
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
            if (condition.rValue.attrType == NOTYPE || condition.rValue.data == NULL) {
                return false;
            }
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
            if (condition.rValue.attrType == NOTYPE || condition.rValue.data == NULL) {
                bool isNull;
                if (info.attrLength == 1) {
                    //cout << (int)data[0] << endl;
                    if (data[0] == -1) {
                        isNull = true;
                    } else {
                        isNull = false;
                    }
                } else {
                    isNull = true;
                    for (int k = 0; k < info.attrLength - 1; ++k) {
                        if (data[k] != -1) {
                            isNull = false;
                            break;
                        }
                    }
                }
                switch (condition.op) {
                    case EQ_OP: return isNull == true;
                    case NE_OP: return isNull == false;
                    default: return false;
                }
            }
            char *stringValue = (char *)condition.rValue.data;
            //fprintf(stdout, "%s\n%s\n", data, stringValue);
            if (condition.op == LK_OP) {
                int len = strlen(stringValue);
                int index = -1;
                for (int i = 0; i < len; ++i) {
                    if (stringValue[i] == '%') {
                        index = i;
                        break;
                    }
                }
                string v = string(stringValue);
                if (index != -1) {
                    int dataLen = strlen(data);
                    if (len > dataLen) {
                        return false;
                    }
                    v.erase(index);
                    v.insert(index, string(data), index, dataLen - len + 1);
                }
                if (v.length() != strlen(data)) {
                    return false;
                }
                for (int i = 0; i < v.length(); ++i) {
                    if (v[i] == '_') {
                        v[i] = data[i];
                    }
                }
                //fprintf(stdout, "%s %s\n", data, v.c_str());
                return strncmp(data, v.c_str(), info.attrLength) == 0;
            }
            int cmpResult = strncmp(data, stringValue, info.attrLength);
            // for (int i = 0; i < info.attrLength; ++i) {
            //     if (data[i] == 0 && stringValue[i] == 0) {
            //         cmpResult = 0;
            //         break;
            //     }
            //     if (data[i] < stringValue[i]) {
            //         cmpResult = -1;
            //         break;
            //     } else if (data[i] > stringValue[i]) {
            //         cmpResult = 1;
            //         break;
            //     }
            // }
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

            info.isForeignKey = (bool)data[5];
            //fprintf(stdout, "%s %s %d\n", tableName, info.attrName.c_str(), data[5]);
            if (data[5]) {
                info.foreignTable = string(pData + MAX_NAME_LEN * 2 + 20 + 4);
                info.foreignAttr = string(pData + MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN);
            }

            data = (BufType)(pData + MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2);
            int s = data[0];
            info.isCheck = (s > 0);
            vector<string> values;
            char *cData = pData + MAX_NAME_LEN * 2 + 20 + 4 + MAX_NAME_LEN * 2 + 4;
            for (int i = 0; i < s; ++i) {
                values.push_back(string(cData + (MAX_ATTR_LEN / s) * i));
            }
            info.values = values;

            attrInfos.push_back(info);
        }
    }
    mRMManager->closeFile(fileHandle);
}

void QL_Manager::getRange(std::string tableName, std::string attrName, int &l, int &r) {
    //cout << "enter QL_Manager::getRange" << endl;
    l = -1;
    r = 1000000000;

    int indexNo = getIndexNo(tableName.c_str());
    //fprintf(stdout, "indexNo = %d\n", indexNo);

    string DBName = mSMManager->getDBName();
    string fullTableName = DBName + "/" + string(tableName);

    IX_IndexHandle *indexHandle;
    if (indexNo != -1) {
        mIXManager->openIndex(fullTableName.c_str(), indexNo, indexHandle);
        indexHandle->getRange(l, r);
        mIXManager->closeIndex(indexHandle);
    }
    
    //cout << "leave QL_Manager::getRange" << endl;
}
