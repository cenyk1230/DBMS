#ifndef __RM_RECORD_H__
#define __RM_RECORD_H__

#include "RID.h"

class RM_Record {
private:
	char *mData;
public:
	RM_Record() {}
	~RM_Record() {}

	bool getData(char *&pData) const {
		pData = mData;
	}
	bool getRid(RID &rid) const {

	}
};

#endif // __RM_RECORD_H__