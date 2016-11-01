#ifndef __IX_INDEX_SCAN_H__
#define __IX_INDEX_SCAN_H__

#include "../utils/base.h"
#include "../utils/RID.h"

class IX_IndexScan {
private:

public:
    IX_IndexScan();
    ~IX_IndexScan();

    bool openScan(IX_IndexHandle *indexHandle, CompOp compOp, void *value);
    bool getNextEntry(RID &rid);
    bool closeScan();
};

#endif // __IX_INDEX_SCAN_H__