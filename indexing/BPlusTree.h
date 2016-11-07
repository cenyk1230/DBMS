#ifndef __B_PLUS_TREE_H__
#define __B_PLUS_TREE_H__

#include <vector>

#include "../utils/base.h"
#include "../utils/RID.h"

class BPlusNode {
public:
    bool isLeaf;
    int num;
    int branch;
    int no;
    AttrType attrType;
    int attrLength;
    BPlusNode *parent;
    BPlusNode *left, *right;
    void **key;
    RID *data;
    BPlusNode **son;

public:
    BPlusNode(int branch, AttrType attrType, int attrLength);
    ~BPlusNode();

    void setKey(int index, void *pData);

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
    BPlusNode* getFirstDataNode();
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
    void getAllEntry(std::vector<std::pair<void *, RID> > &entries);
    BPlusNode* getRoot();
};

#endif // __B_PLUS_TREE_H__