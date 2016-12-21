#ifndef __RM_FILE_HANDLE_H__
#define __RM_FILE_HANDLE_H__

#include <vector>
#include <memory>

#include "RM_Record.h"

#include "../utils/RID.h"
#include "../utils/pagedef.h"
#include "../bufmanager/BufPageManager.h"

class RM_FileHandle {
private:
	bool mModified;
	int mFileID;
	int mRecordSize;
	int mPageNum;
	int mSlotNum;
	int mRecordOffset;
	BufPageManager *mBufPageManager;
	std::vector<int> mAvailablePage;

private:
	RID getNewRid();
	void initPage(int pageID);
	void writeBackHeaderPage();
	void checkPageAvailable(int pageID);

public:
	RM_FileHandle(BufPageManager *bpm, int fileID);
	~RM_FileHandle();

	bool getRec(const RID &rid, RM_Record &rec) const;
	bool insertRec(const char *pData, RID &rid);
	bool deleteRec(const RID &rid);
	bool updateRec(const RM_Record &rec);
	bool forcePage(int pageID) const;

	int getFileID() const;
	int getPageNum() const;
	bool getAllRecFromPage(int pageID, vector<shared_ptr<RM_Record> > &recordVector);
};

#endif // __RM_FILE_HANDLE_H__