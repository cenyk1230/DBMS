#ifndef __RM_FILE_HANDLE_H__
#define __RM_FILE_HANDLE_H__

#include "RID.h"
#include "RM_Record.h"

class RM_FileHandle {
public:
	RM_FileHandle();
	~RM_FileHandle();

	bool getRec(const RID &rid, RM_Record &rec) const;
	bool insertRec(const char *pData, RID &rid);
	bool deleteRec(const RID &rid);
	bool updateRec(const RM_Record &rec);
	bool forcePages(int fileID, int pageID) const;
};

#endif // __RM_FILE_HANDLE_H__