#ifndef __RID_H__
#define __RID_H__

class RID {
public:
	int fileID;
	int pageID;
	int slotID;

public:
	RID() {
		fileID = pageID = slotID = -1;
	}
	RID(int fileID, int pageID, int slotID) {
		this->fileID = fileID;
		this->pageID = pageID;
		this->slotID = slotID;
	}
	~RID() {}

	bool getAll(int &fileID, int &pageID, int &slotID) const {
		if (this->fileID == -1 || this->pageID == -1 || this->slotID == -1) {
			return false;
		}
		fileID = this->fileID;
		pageID = this->pageID;
		slotID = this->slotID;
		return true;
	}
	bool getPageNum(int &fileID, int &pageID) const {
		if (this->fileID == -1 || this->pageID == -1) {
			return false;
		}
		fileID = this->fileID;
		pageID = this->pageID;
		return true;
	}
	bool getSlotID(int &slotID) const {
		if (this->slotID == -1) {
			return false;
		}
		slotID = this->slotID;
		return true;
	}
	bool isValid() const {
		return this->fileID != -1 && this->pageID != -1 && this->slotID != -1;
	}

	bool operator < (const RID &rid) const {
		return fileID < rid.fileID || (fileID == rid.fileID && (pageID < rid.pageID || (pageID == rid.pageID && slotID < rid.slotID)));
	}

	bool operator == (const RID &rid) const {
		return fileID == rid.fileID && pageID == rid.pageID && slotID == rid.slotID; 
	}

	bool operator != (const RID &rid) const {
		return !(*this == rid);
	}
};

#endif // __RID_H__