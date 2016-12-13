#include <cstdio>
#include <iostream>

#include "BPlusTree.h"

using namespace std;

BPlusNode::BPlusNode(int branch, AttrType attrType, int attrLength) {
    isLeaf = false;
    num = 0;
    this->branch = branch;
    parent = NULL;
    left = right = NULL;
    son = new BPlusNode*[branch];
    for (int i = 0; i < branch; ++i) {
        son[i] = NULL;
    }
    this->attrType = attrType;
    this->attrLength = attrLength;
    key = new void*[branch];
    for (int i = 0; i < branch; ++i) {
        if (attrType == INTEGER) {
            key[i] = new int();
        } else if (attrType == FLOAT) {
            key[i] = new float();
        } else if (attrType == STRING) {
            key[i] = new char[attrLength];
        }
    }
    data = new RID[branch];
}

BPlusNode::~BPlusNode() {
    for (int i = 0; i < branch; ++i) {
        if (attrType == INTEGER) {
            int *data = (int *)key[i];
            delete data;
        } else if (attrType == FLOAT) {
            float *data = (float *)key[i];
            delete data;
        } else if (attrType == STRING) {
            char *data = (char *)key[i];
            delete[] data;
        }
    }
    delete[] son;
    delete[] key;
    delete[] data;
}

void BPlusNode::setKey(int index, void *pData) {
    if (attrType == INTEGER) {
        int *data = (int *)key[index];
        *data = *((int *)pData);
    } else if (attrType == FLOAT) {
        float *data = (float *)key[index];
        *data = *((float *)pData);
    } else if (attrType == STRING) {
        char *data = (char *)key[index];
        for (int i = 0; i < attrLength; ++i) {
            data[i] = ((char *)pData)[i];
        }
    }
}


BPlusTree::BPlusTree(int branch, AttrType attrType, int attrLength) {
    mBranch = branch;
    mAttrType = attrType;
    mAttrLength = attrLength;
    mRoot = new BPlusNode(branch, attrType, attrLength);
    mRoot->isLeaf = true;
}

BPlusTree::~BPlusTree() {
    dfs(mRoot);
}

void BPlusTree::dfs(BPlusNode *node) {
    if (!node->isLeaf) {
        for (int i = 0; i < mBranch; ++i) {
            if (node->son[i] != NULL) {
                dfs(node->son[i]);
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
    if (node->isLeaf) {
        return node;
    }
    for (int i = 0; i < node->num - 1; ++i) {
        if (lessThan(pData, node->key[i + 1]))
            return search(node->son[i], pData);
    }
    return search(node->son[node->num - 1], pData);
}

void BPlusTree::insertInLeaf(BPlusNode *node, void *pData, const RID &rid) {
    for (int i = node->num; i >= 0; --i) {
        if (i == 0 || lessThan(node->key[i - 1], pData)) {
            //node->key[i] = pData;
            node->setKey(i, pData);
            node->data[i] = rid;
            break;
        }
        //node->key[i] = node->key[i - 1];
        node->setKey(i, node->key[i - 1]);
        node->data[i] = node->data[i - 1];
    }
    ++node->num;
    if (node->num < mBranch) {
        return;
    }
    BPlusNode *newNode = new BPlusNode(mBranch, mAttrType, mAttrLength);
    newNode->isLeaf = true;
    int half = mBranch / 2;
    node->num = half;
    newNode->num = mBranch - half;
    for (int i = 0; i < newNode->num; ++i) {
        //newNode->key[i] = node->key[i + half];
        newNode->setKey(i, node->key[i + half]);
        newNode->data[i] = node->data[i + half];
    }
    for (int i = half; i < mBranch; ++i) {
        //node->key[i] = NULL;
        node->data[i] = RID();
    }
    newNode->right = node->right;
    node->right = newNode;
    if (node->parent == NULL) {
        mRoot = new BPlusNode(mBranch, mAttrType, mAttrLength);
        mRoot->num = 1;
        //mRoot->key[0] = node->key[0];
        mRoot->setKey(0, node->key[0]);
        mRoot->son[0] = node;
        node->parent = mRoot;
    }
    newNode->parent = node->parent;
    insertInInternal(node->parent, newNode, newNode->key[0]);
}

void BPlusTree::insertInInternal(BPlusNode *node, BPlusNode *sonNode, void *pData) {
    for (int i = node->num; i > 0; --i) {
        if (lessThan(node->key[i - 1], pData)) {
            //node->key[i] = pData;
            node->setKey(i, pData);
            node->son[i] = sonNode;
        }
        //node->key[i] = node->key[i - 1];
        node->setKey(i, node->key[i - 1]);
        node->son[i] = node->son[i - 1];
    }
    ++node->num;
    if (node->num < mBranch) {
        return;
    }
    BPlusNode *newNode = new BPlusNode(mBranch, mAttrType, mAttrLength);
    int half = mBranch / 2;
    node->num = half;
    newNode->num = mBranch - half;
    for (int i = 0; i < newNode->num; ++i) {
        //newNode->key[i] = node->key[i + half];
        newNode->setKey(i, node->key[i + half]);
        newNode->son[i] = node->son[i + half];
    }
    for (int i = half; i < mBranch; ++i) {
        //node->key[i] = NULL;
        node->son[i] = NULL;
    }
    if (node->parent == NULL) {
        mRoot = new BPlusNode(mBranch, mAttrType, mAttrLength);
        mRoot->num = 1;
        //mRoot->key[0] = node->key[0];
        mRoot->setKey(0, node->key[0]);
        mRoot->son[0] = node;
        node->parent = mRoot;
    }
    newNode->parent = node->parent;
    insertInInternal(node->parent, newNode, newNode->key[0]);
}

void BPlusTree::insertEntry(void *pData, const RID &rid) {
    BPlusNode *node = search(mRoot, pData);
    insertInLeaf(node, pData, rid);
}

void BPlusTree::deleteInInternal(BPlusNode *node, void *pData) {
    int index = 0;
    for (int i = 0; i < node->num; ++i) {
        //if (node->key[i] == pData) {
        if (!lessThan(node->key[i], pData) && !lessThan(pData, node->key[i])) {
            index = i;
            break;
        }
    }
    --node->num;
    for (int i = index; i < node->num; ++i) {
        //node->key[i] = node->key[i + 1];
        node->setKey(i, node->key[i + 1]);
        node->son[i] = node->son[i + 1];
    }
    //node->key[node->num] = NULL;
    node->son[node->num] = NULL;
    if (node->num == 0) {
        if (node->parent != NULL) {
            deleteInInternal(node->parent, pData);
            delete node;
        } else {
            node->isLeaf = true;
        }
    }
}

bool BPlusTree::deleteInLeaf(BPlusNode *node, void *pData, const RID &rid) {
    //cout << "deleteInLeaf " << pData << endl;
    int index = 0;
    for (int i = 0; i < node->num; ++i) {
        //cout << i << " " << node->key[i] << " " << pData << endl;
        //if (node->key[i] == pData) {
        if (!lessThan(node->key[i], pData) && !lessThan(pData, node->key[i])) {
            if (node->data[i] != rid) {
                return false;
            }
            index = i;
            break;
        }
    }
    --node->num;
    for (int i = index; i < node->num; ++i) {
        //node->key[i] = node->key[i + 1];
        node->setKey(i, node->key[i + 1]);
        node->data[i] = node->data[i + 1];
    }
    //node->key[node->num] = NULL;
    node->data[node->num] = RID();
    if (node->num == 0 && node->parent != NULL) {
        if (node->left != NULL) {
            node->left->right = node->right;
        }
        if (node->right != NULL) {
            node->right->left = node->left;
        }
        deleteInInternal(node->parent, pData);
        delete node;
    }
    return true;
}

bool BPlusTree::deleteEntry(void *pData, const RID &rid) {
    BPlusNode *node = search(mRoot, pData);
    return deleteInLeaf(node, pData, rid);
}

BPlusNode* BPlusTree::getFirstDataNode() {
    BPlusNode *node = mRoot;
    while (!node->isLeaf) {
        node = node->son[0];
    }
    return node;
}

void BPlusTree::getAllEntry(vector<pair<void *, RID> > &entries) {
    BPlusNode *node = getFirstDataNode();
    //cout << node->num << endl;
    entries.clear();
    while (node != NULL) {
        for (int i = 0; i < node->num; ++i) {
            //cout << i << " " << node->data[i].fileID << endl;
            entries.push_back(make_pair(node->key[i], node->data[i]));
        }
        node = node->right;
    }
}

BPlusNode* BPlusTree::getRoot() {
    return mRoot;
}
