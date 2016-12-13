#ifndef __IX_INDEX_SCAN_H__
#define __IX_INDEX_SCAN_H__

#include "IX_IndexHandle.h"

#include "../utils/base.h"
#include "../utils/RID.h"

#define IX_EOF

class IX_IndexScan {
private:
    AttrType dataType;
    int dataLength;
    int current_pos;
    std::vector<std::pair<void *, RID> > allIndexes;
    CompOp condition_op;
    void * condition_val_ptr;
    union another_common_type{
        int integer;
        float real;
        char * string;
    };
    bool satisfy(int indexNum);

public:
    IX_IndexScan();
    ~IX_IndexScan();

    bool openScan(IX_IndexHandle *indexHandle, CompOp compOp, void *value);
    bool getNextEntry(RID &rid);
    bool closeScan();
};

#endif // __IX_INDEX_SCAN_H__