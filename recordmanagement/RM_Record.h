#ifndef __RM_RECORD_H__
#define __RM_RECORD_H__

#include "RID.h"

#include "../utils/pagedef.h"

class RM_Record {
private:
	int mSize;
	char *mData;
	RID mRid;

public:
	RM_Record(char *pData, int size, RID rid) {
		mSize = size;
		mData = new char[mSize];
		memcpy(mData, pData, size);
		mRid = rid;
	}
	~RM_Record() {
		if (mData != NULL)
			delete[] mData;
		mData = NULL;
	}

	char *getData() const {
		return mData;
	}
	RID getRid() const {
		return mRid;
	}
	RM_Record getCopy() const {
		return RM_Record(mData, mSize, mRid);
	}
};

#endif // __RM_RECORD_H__