#ifndef __RM_RECORD_H__
#define __RM_RECORD_H__

#include <cstring>
#include <cstdio>

#include "../utils/RID.h"
#include "../utils/pagedef.h"

class RM_Record {
private:
	int mSize;
	char *mData;
	RID mRid;

public:
	RM_Record() {
		mData = NULL;
	}
	RM_Record(const char *pData, int size, RID rid) {
		mSize = size;
		mData = new char[mSize];
		memcpy(mData, pData, size);
		mRid = rid;
	}
	RM_Record(const RM_Record &rec) {
		// fprintf(stdout, "copy construction\n");
		//if (mData != NULL) {
		//	delete[] mData;
		//}
		mSize = rec.getSize();
		mData = new char[mSize];
		memcpy(mData, rec.getData(), mSize);
		mRid = rec.getRid();
	}
	~RM_Record() {
		// fprintf(stdout, "%p\n", mData);
		if (mData != NULL)
			delete[] mData;
		mData = NULL;
	}
	RM_Record& operator =(const RM_Record &rec) {
		// fprintf(stdout, "assignment operator\n");
		if (mData != NULL) {
			delete[] mData;
		}
		mSize = rec.getSize();
		mData = new char[mSize];
		memcpy(mData, rec.getData(), mSize);
		mRid = rec.getRid();
		return *this;
	}

	int getSize() const {
		return mSize;
	}
	char *getData() const {
		return mData;
	}
	RID getRid() const {
		return mRid;
	}
};

#endif // __RM_RECORD_H__