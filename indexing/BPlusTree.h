#ifndef __B_PLUS_TREE_H__
#define __B_PLUS_TREE_H__

#include "../utils/base.h"
#include "../utils/RID.h"

class BPlusNode {
public:
    bool mIsLeaf;
    int mNum;
    int mBranch;
    BPlusNode *mParent;
    void **mKey;
    RID *mData;
    BPlusNode **mSon;

public:
    BPlusNode(int branch);
    ~BPlusNode();

};

class BPlusTree {
private:
    int mBranch;
    AttrType mAttrType;
    int mAttrLength;
    BPlusNode *mRoot;

private:
    void dfs(BPlusNode *node);
    bool lessThan(void *pData, void *key);
    BPlusNode* search(BPlusNode *node, void *pData);
    void insertInLeaf(BPlusNode *node, void *pData, const RID &rid);
    void insertInInternal(BPlusNode *node, BPlusNode *sonNode, void *pData);
    bool deleteInLeaf(BPlusNode *node, void *pData, const RID &rid);
    void deleteInInternal(BPlusNode *node, void *pData);

public:
    BPlusTree(int branch, AttrType attrType, int attrLength);
    ~BPlusTree();

    void insertEntry(void *pData, const RID &rid);
    bool deleteEntry(void *pData, const RID &rid);
};

#endif // __B_PLUS_TREE_H__