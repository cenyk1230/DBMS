#ifndef __RID_H__
#define __RID_H__

class RID {
private:
	int mFileID;
	int mPageID;
	int mSlotID;

public:
	RID() {
		mFileID = mPageID = mSlotID = -1;
	}
	RID(int fileID, int pageID, int slotID) {
		mFileID = fileID;
		mPageID = pageID;
		mSlotID = slotID;
	}
	~RID() {}

	bool getPageNum(int &fileID, int &pageID) const {
		if (mFileID == -1 || mPageID == -1) {
			return false;
		}
		fileID = mFileID;
		pageID = mPageID;
		return true;
	}
	bool getSlotID(int &slotID) const {
		if (mSlotID == -1) {
			return false;
		}
		slotID = mSlotID;
		return true;
	}
};

#endif // __RID_H__