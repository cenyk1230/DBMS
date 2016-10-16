#ifndef __RM_FILE_SCAN_H__
#define __RM_FILE_SCAN_H__

#include <memory>

#include "RM_FileHandle.h"
#include "RM_Record.h"

#include "../utils/base.h"

class RM_FileScan {
private:
	int mCurIndex;
	std::vector<shared_ptr<RM_Record> > mRecordVector;

public:
	RM_FileScan();
	~RM_FileScan();

	bool satisfyCondition(shared_ptr<RM_Record> ptrRec,
						  AttrType attrType,
						  int attrLength,
						  int attrOffset,
						  CompOp compOp,
						  void *value);
	bool openScan(RM_FileHandle *fileHandle,
				  AttrType attrType,
				  int attrLength,
				  int attrOffset,
				  CompOp compOp,
				  void *value);
	bool getNextRec(RM_Record &rec);
	bool closeScan();
};

#endif // __RM_FILE_SCAN_H__