#include <cstdio>
#include <iostream>

#include "BPlusTree.h"

using namespace std;

BPlusNode::BPlusNode(int branch) {
    mIsLeaf = false;
    mNum = 0;
    mBranch = branch;
    mParent = NULL;
    mSon = new BPlusNode*[mBranch];
    for (int i = 0; i < mBranch; ++i) {
        mSon[i] = NULL;
    }
    mKey = new void*[mBranch];
    for (int i = 0; i < mBranch; ++i) {
        mKey[i] = NULL;
    }
    mData = new RID[mBranch];
}

BPlusNode::~BPlusNode() {
    delete[] mSon;
    delete[] mKey;
    delete[] mData;
}

BPlusTree::BPlusTree(int branch, AttrType attrType, int attrLength) {
    mBranch = branch;
    mAttrType = attrType;
    mAttrLength = attrLength;
    mRoot = new BPlusNode(branch);
    mRoot->mIsLeaf = true;
}

BPlusTree::~BPlusTree() {
    dfs(mRoot);
}

void BPlusTree::dfs(BPlusNode *node) {
    if (!node->mIsLeaf) {
        for (int i = 0; i < mBranch; ++i) {
            if (node->mSon[i] != NULL) {
                dfs(node->mSon[i]);
            }
        }
    }
    delete node;
    node = NULL;
}

bool BPlusTree::lessThan(void *pData, void *key) {
    if (mAttrType == INTEGER) {
        int dataValue = *((int *)pData);
        int keyValue = *((int *)key);
        return dataValue < keyValue; 
    } else if (mAttrType == FLOAT) {
        float dataValue = *((float *)pData);
        float keyValue = *((float *)key);
        return dataValue < keyValue;
    } else if (mAttrType == STRING) {
        char *dataStr = (char *)pData;
        char *keyStr = (char *)key;
        for (int i = 0; i < mAttrLength; ++i)
            if (dataStr[i] < keyStr[i])
                return true;
            else if (dataStr[i] > keyStr[i])
                return false;
        return false;
    }
    return false;
}

BPlusNode* BPlusTree::search(BPlusNode *node, void *pData) {
    if (node->mIsLeaf) {
        return node;
    }
    for (int i = 0; i < node->mNum - 1; ++i) {
        if (lessThan(pData, node->mKey[i + 1]))
            return search(node->mSon[i], pData);
    }
    return search(node->mSon[node->mNum - 1], pData);
}

void BPlusTree::insertInLeaf(BPlusNode *node, void *pData, const RID &rid) {
    for (int i = node->mNum; i >= 0; --i) {
        if (i == 0 || lessThan(node->mKey[i - 1], pData)) {
            node->mKey[i] = pData;
            node->mData[i] = rid;
            break;
        }
        node->mKey[i] = node->mKey[i - 1];
        node->mData[i] = node->mData[i - 1];
    }
    ++node->mNum;
    if (node->mNum < node->mBranch) {
        return;
    }
    BPlusNode *newNode = new BPlusNode(mBranch);
    newNode->mIsLeaf = true;
    int half = mBranch / 2;
    node->mNum = half;
    newNode->mNum = mBranch - half;
    for (int i = 0; i < newNode->mNum; ++i) {
        newNode->mKey[i] = node->mKey[i + half];
        newNode->mData[i] = node->mData[i + half];
    }
    for (int i = half; i < mBranch; ++i) {
        node->mKey[i] = NULL;
        node->mData[i] = RID();
    }
    newNode->mSon[mBranch - 1] = node->mSon[mBranch - 1];
    node->mSon[mBranch - 1] = newNode;
    if (node->mParent == NULL) {
        mRoot = new BPlusNode(mBranch);
        mRoot->mNum = 1;
        mRoot->mKey[0] = node->mKey[0];
        mRoot->mSon[0] = node;
        node->mParent = mRoot;
    }
    newNode->mParent = node->mParent;
    insertInInternal(node->mParent, newNode, newNode->mKey[0]);
}

void BPlusTree::insertInInternal(BPlusNode *node, BPlusNode *sonNode, void *pData) {
    for (int i = node->mNum; i > 0; --i) {
        if (lessThan(node->mKey[i - 1], pData)) {
            node->mKey[i] = pData;
            node->mSon[i] = sonNode;
        }
        node->mKey[i] = node->mKey[i - 1];
        node->mSon[i] = node->mSon[i - 1];
    }
    ++node->mNum;
    if (node->mNum < node->mBranch) {
        return;
    }
    BPlusNode *newNode = new BPlusNode(mBranch);
    int half = mBranch / 2;
    node->mNum = half;
    newNode->mNum = mBranch - half;
    for (int i = 0; i < newNode->mNum; ++i) {
        newNode->mKey[i] = node->mKey[i + half];
        newNode->mSon[i] = node->mSon[i + half];
    }
    for (int i = half; i < mBranch; ++i) {
        node->mKey[i] = NULL;
        node->mSon[i] = NULL;
    }
    if (node->mParent == NULL) {
        mRoot = new BPlusNode(mBranch);
        mRoot->mNum = 1;
        mRoot->mKey[0] = node->mKey[0];
        mRoot->mSon[0] = node;
        node->mParent = mRoot;
    }
    newNode->mParent = node->mParent;
    insertInInternal(node->mParent, newNode, newNode->mKey[0]);
}

void BPlusTree::insertEntry(void *pData, const RID &rid) {
    BPlusNode *node = search(mRoot, pData);
    insertInLeaf(node, pData, rid);
}

void BPlusTree::deleteInInternal(BPlusNode *node, void *pData) {
    int index = 0;
    for (int i = 0; i < node->mNum; ++i) {
        if (node->mKey[i] == pData) {
            index = i;
            break;
        }
    }
    --node->mNum;
    for (int i = index; i < node->mNum; ++i) {
        node->mKey[i] = node->mKey[i + 1];
        node->mSon[i] = node->mSon[i + 1];
    }
    node->mKey[node->mNum] = NULL;
    node->mSon[node->mNum] = NULL;
    if (node->mNum == 0 && node->mParent != NULL) {
        deleteInInternal(node->mParent, pData);
        delete node;
    }
}

bool BPlusTree::deleteInLeaf(BPlusNode *node, void *pData, const RID &rid) {
    cout << "deleteInLeaf " << pData << endl;
    int index = 0;
    for (int i = 0; i < node->mNum; ++i) {
        cout << i << " " << node->mKey[i] << " " << pData << endl;
        if (node->mKey[i] == pData) {
            if (node->mData[i] != rid) {
                return false;
            }
            index = i;
            break;
        }
    }
    --node->mNum;
    for (int i = index; i < node->mNum; ++i) {
        node->mKey[i] = node->mKey[i + 1];
        node->mData[i] = node->mData[i + 1];
    }
    node->mKey[node->mNum] = NULL;
    node->mData[node->mNum] = RID();
    if (node->mNum == 0 && node->mParent != NULL) {
        deleteInInternal(node->mParent, pData);
        delete node;
    }
    return true;
}

bool BPlusTree::deleteEntry(void *pData, const RID &rid) {
    BPlusNode *node = search(mRoot, pData);
    return deleteInLeaf(node, pData, rid);
}