#ifndef __RM_FILE_SCAN_H__
#define __RM_FILE_SCAN_H__

#include "RM_FileHandle.h"
#include "RM_Record.h"

#include "../util/base.h"

class RM_FileScan {
public:
	RM_FileScan();
	~RM_FileScan();

	bool openScan(const RM_FileHandle &fileHandle,
				  AttrType attrType,
				  int attrLength,
				  int attrOffset,
				  CompOp compOp,
				  void *value);
	bool getNextRec(RM_Record &rec);
	bool closeScan();
};

#endif // __RM_FILE_SCAN_H__