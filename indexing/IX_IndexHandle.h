#ifndef __IX_INDEX_HANDLE_H__
#define __IX_INDEX_HANDLE_H__

#include <vector>

#include "BPlusTree.h"

#include "../bufmanager/BufPageManager.h"

#include "../utils/base.h"
#include "../utils/RID.h"

class IX_IndexHandle {
private:
    bool mModified;
    int mFileID;
    AttrType mAttrType;
    int mAttrLength;
    int mPageNum;
    BPlusTree *mBPlusTree;
    BufPageManager *mBufPageManager;

private:
    void writeBackHeaderPage();

public:
    IX_IndexHandle(BufPageManager *bpm, int fileID);
    ~IX_IndexHandle();

    void insertEntry(void *pData, const RID &rid);
    bool deleteEntry(void *pData, const RID &rid);
    bool forcePages();

    int getFileID() const;
    AttrType getAttrType() const;
    int getAttrLength() const;
    void getAllRec(std::vector<std::pair<void *, RID> > &recordVector);
};

#endif // __IX_INDEX_HANDLE_H__