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

	bool getAll(int &fileID, int &pageID, int &slotID) const {
		if (mFileID == -1 || mPageID == -1 || mSlotID == -1) {
			return false;
		}
		fileID = mFileID;
		pageID = mPageID;
		slotID = mSlotID;
		return true;
	}
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
	bool isValid() const {
		return mFileID != -1 && mPageID != -1 && mSlotID != -1;
	}

	bool operator < (const RID &rid) const {
		int fileID, pageID, slotID;
		rid.getPageNum(fileID, pageID);
		rid.getSlotID(slotID);
		return mFileID < fileID || (mFileID == fileID && (mPageID < pageID || (mPageID == pageID && mSlotID < slotID)));
	}

	bool operator == (const RID &rid) const {
		int fileID, pageID, slotID;
		rid.getAll(fileID, pageID, slotID);
		return fileID == mFileID && pageID == mPageID && slotID == mSlotID; 
	}

	bool operator != (const RID &rid) const {
		return !(*this == rid);
	}
};

#endif // __RID_H__