#ifndef __IX_MANAGER_H__
#define __IX_MANAGER_H__

#include "IX_IndexHandle.h"

#include "../fileio/FileManager.h"
#include "../fileio/BufPageManager.h"
#include "../utils/base.h"

class IX_Manager {
private:
    FileManager *mFileManager;
    BufPageManager *mBufPageManager;
public:
    IX_Manager(FileManager *fm);
    ~IX_Manager();
    bool createIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength);
    bool destroyIndex(const char *fileName, int indexNo);
    bool openIndex(const char *fileName, int indexNo, IX_IndexHandle *&indexHandle);
    bool closeIndex(IX_IndexHandle *indexHandle);
};


#endif // __IX_MANAGER_H__