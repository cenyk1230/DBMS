#ifndef __IX_INDEX_HANDLE_H__
#define __IX_INDEX_HANDLE_H__

#include <vector>

#include "BPlusTree.h"

#include "../bufmanager/BufPageManager.h"

#include "../utils/base.h"
#include "../utils/RID.h"

class IX_IndexHandle {
private:
    int mFileID;
    AttrType mAttrType;
    int mAttrLength;
    int mPageNum;
    int mBranch;
    int mTotal;
    BPlusTree *mBPlusTree;
    BufPageManager *mBufPageManager;

private:
    void calcNo(BPlusNode *node);
    void storeHeaderPage();
    void loadBPlusTree();
    void storeBPlusTree();
    void storeBPlusNode(BPlusNode *node);

public:
    IX_IndexHandle(BufPageManager *bpm, int fileID);
    ~IX_IndexHandle();

    void insertEntry(void *pData, const RID &rid);
    bool deleteEntry(void *pData, const RID &rid);
    bool forcePages();

    int getFileID() const;
    AttrType getAttrType() const;
    int getAttrLength() const;
    void getAllEntry(std::vector<std::pair<void *, RID> > &entries);
};

#endif // __IX_INDEX_HANDLE_H__