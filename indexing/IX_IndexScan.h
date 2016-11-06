#ifndef __IX_INDEX_SCAN_H__
#define __IX_INDEX_SCAN_H__

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
    union common_type{
        int integer;
        float real;
        char * string;
    };

public:
    IX_IndexScan();
    ~IX_IndexScan();

    bool openScan(IX_IndexHandle *indexHandle, CompOp compOp, void *value);
    bool getNextEntry(RID &rid);
    bool closeScan();
};

#endif // __IX_INDEX_SCAN_H__