#include "IX_IndexHandle.h"

IX_IndexHandle::IX_IndexHandle(BufPageManager *bpm, int fileID) {
    mBufPageManager = bpm;
    mFileID = fileID;
    mModified = false;
    int index;
    BufType uData = mBufPageManager->allocPage(mFileID, 0, index, true);
    mAttrType = (AttrType)uData[0];
    mAttrLength = (int)uData[1];
    mPageNum = (int)uData[2];
    mBufPageManager->release(index);

    int branch = (PAGE_SIZE - 16) / 28;
    mBPlusTree = new BPlusTree(branch, mAttrType, mAttrLength);
}

IX_IndexHandle::~IX_IndexHandle() {
    if (mModified) {
        writeBackHeaderPage();
    }
    mBufPageManager->close();
    delete mBPlusTree;
}

void IX_IndexHandle::writeBackHeaderPage() {
    int index;
    BufType uData = mBufPageManager->allocPage(mFileID, 0, index, false);
    uData[0] = (unsigned)mAttrType;
    uData[1] = (unsigned)mAttrLength;
    uData[2] = (unsigned)mPageNum;
    mBufPageManager->markDirty(index);
    mBufPageManager->writeBack(index);
}

void IX_IndexHandle::insertEntry(void *pData, const RID &rid) {
    mBPlusTree->insertEntry(pData, rid);
}

bool IX_IndexHandle::deleteEntry(void *pData, const RID &rid) {
    return mBPlusTree->deleteEntry(pData, rid);
}

bool IX_IndexHandle::forcePages() {
    mBufPageManager->close();
}

int IX_IndexHandle::getFileID() const {
    return mFileID;
}

AttrType IX_IndexHandle::getAttrType() const {
    return mAttrType;
}

int IX_IndexHandle::getAttrLength() const {
    return mAttrLength;
}

void getAllRec(vector<pair<void *, RID> > &recordVector) {
    
}