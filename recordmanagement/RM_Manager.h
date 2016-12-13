#ifndef __RM_MANAGER_H__
#define __RM_MANAGER_H__

#include "RM_FileHandle.h"

#include "../fileio/FileManager.h"
#include "../bufmanager/BufPageManager.h"

class RM_Manager {
private:
	BufPageManager *mBufPageManager;
	FileManager *mFileManager;
public:
	RM_Manager(FileManager *fm, BufPageManager *bpm);
	~RM_Manager();

	bool createFile(const char *fileName, int recordSize);
	bool destroyFile(const char *fileName);
	bool openFile(const char *fileName, RM_FileHandle *&fileHandle);
	bool closeFile(RM_FileHandle *fileHandle);
};

#endif // __RM_MANAGER_H__