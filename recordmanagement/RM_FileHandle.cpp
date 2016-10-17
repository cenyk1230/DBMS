#include <vector>
#include <memory>
#include <cstdio>

#include "RM_FileHandle.h"

using namespace std;

RM_FileHandle::RM_FileHandle(BufPageManager *bpm, int fileID) {
    mModified = false;
    mBufPageManager = bpm;
    mFileID = fileID;
    int index;
    BufType uData = mBufPageManager->allocPage(mFileID, 0, index, true);
    mRecordSize = (int)uData[0];
    mSlotNum = (int)uData[1];
    mRecordOffset = mSlotNum;
    mPageNum = (int)uData[2];
    char *data = (char *)uData;
    data = data + 12;
    mAvailablePage.push_back(false);
    for (int i = 1; i < mPageNum; ++i) {
        if (data[i] == 0)
            mAvailablePage.push_back(true);
        else
            mAvailablePage.push_back(false);
    }
    mBufPageManager->release(index);
}

RM_FileHandle::~RM_FileHandle() {
    if (mModified) {
        writeBackHeaderPage();
    }
    mBufPageManager->close();
}

RID RM_FileHandle::getNewRid() {
    int pageID = -1, slotID = -1;
    for (int i = 1; i < mPageNum; ++i) {
        if (mAvailablePage[i]) {
            pageID = i;
            break;
        }
    }
    if (pageID == -1) {
        pageID = mPageNum;
        slotID = 0;
        ++mPageNum;
        mAvailablePage.push_back(true);
        mModified = true;
        initPage(pageID);
    } else {
        int index;
        BufType uData = mBufPageManager->getPage(mFileID, pageID, index);
        char *data = (char *)uData;
        for (int i = 0; i < mSlotNum; ++i) {
            if (data[i] == 0) {
                slotID = i;
                break;
            }
        }
    }
    return RID(mFileID, pageID, slotID);
}

void RM_FileHandle::initPage(int pageID) {
    int index;
    BufType uData = mBufPageManager->getPage(mFileID, pageID, index);
    char *data = (char *)uData;
    memset(data, 0, mSlotNum);
    mBufPageManager->markDirty(index);
}

void RM_FileHandle::writeBackHeaderPage() {
    int index;
    BufType uData = mBufPageManager->allocPage(mFileID, 0, index, false);
    uData[0] = mRecordSize;
    uData[1] = mSlotNum;
    uData[2] = mPageNum;
    char *data = (char *)uData;
    data = data + 12;
    for (int i = 1; i < mPageNum; ++i) {
        if (mAvailablePage[i])
            data[i] = 0;
        else
            data[i] = 1;
    }
    mBufPageManager->markDirty(index);
	mBufPageManager->writeBack(index);
}

void RM_FileHandle::checkPageAvailable(int pageID) {
    int index;
    BufType uData = mBufPageManager->getPage(mFileID, pageID, index);
    char *data = (char *)uData;
    bool available = false;
    for (int i = 0; i < mSlotNum; ++i)
        if (data[i] == 0) {
            available = true;
            break;
        }
    mAvailablePage[pageID] = available;
}

int RM_FileHandle::getFileID() const {
    return mFileID;
}

int RM_FileHandle::getPageNum() const {
    return mPageNum;
}

bool RM_FileHandle::getAllRecFromPage(int pageID, vector<shared_ptr<RM_Record> > &recordVector) {
    int index;
    BufType uData = mBufPageManager->getPage(mFileID, pageID, index);
    char *data = (char *)uData;
    recordVector.clear();
    for (int i = 0; i < mSlotNum; ++i) {
        if (data[i] == 1) {
            char *curData = data + mRecordOffset + mRecordSize * i;
            shared_ptr<RM_Record> ptrRec(new RM_Record(curData, mRecordSize, RID(mFileID, pageID, i)));
            recordVector.push_back(ptrRec);
        }
    }
    return true;
}

bool RM_FileHandle::getRec(const RID &rid, RM_Record &rec) const {
    int fileID, pageID, slotID;
    if (!rid.getAll(fileID, pageID, slotID)) {
        return false;
    }
    int index;
    BufType uData = mBufPageManager->getPage(fileID, pageID, index);
    char *data = (char *)uData;
    if (data[slotID] == 0) {
        return false;
    }
    data = data + mRecordOffset + mRecordSize * slotID;
    rec = RM_Record(data, mRecordSize, rid);
    return true;
}

bool RM_FileHandle::insertRec(const char *pData, RID &rid) {
    rid = getNewRid();
    int fileID, pageID, slotID;
    if (!rid.getAll(fileID, pageID, slotID)) {
        return false;
    }
    int index;
    BufType uData = mBufPageManager->getPage(fileID, pageID, index);
    char *data = (char *)uData;
    data[slotID] = 1;
    checkPageAvailable(pageID);
    data = data + mRecordOffset + mRecordSize * slotID;
    memcpy(data, pData, mRecordSize);
    mBufPageManager->markDirty(index);
    return true;
}

bool RM_FileHandle::deleteRec(const RID &rid) {
    int fileID, pageID, slotID;
    if (!rid.getAll(fileID, pageID, slotID)) {
        return false;
    }
    int index;
    BufType uData = mBufPageManager->getPage(fileID, pageID, index);
    char *data = (char *)uData;
    data[slotID] = 0;
    checkPageAvailable(pageID);
    mBufPageManager->markDirty(index);
    return true;
}

bool RM_FileHandle::updateRec(const RM_Record &rec) {
    RID rid = rec.getRid();
    int fileID, pageID, slotID;
    if (!rid.getAll(fileID, pageID, slotID)) {
        return false;
    }
    int index;
    BufType uData = mBufPageManager->getPage(fileID, pageID, index);
    char *data = (char *)uData;
    data = data + mRecordOffset + mRecordSize * slotID;
    memcpy(data, rec.getData(), mRecordSize);
    mBufPageManager->markDirty(index);
    return true;
}

bool RM_FileHandle::forcePage(int pageID) const {
    int index = -1;
    mBufPageManager->getPage(mFileID, pageID, index);
    if (index == -1) {
        return false;
    }
    mBufPageManager->writeBack(index);
    return true;
}
