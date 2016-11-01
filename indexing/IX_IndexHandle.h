#ifndef __IX_INDEX_HANDLE_H__
#define __IX_INDEX_HANDLE_H__

class IX_IndexHandle {
private:

public:
    IX_IndexHandle();
    ~IX_IndexHandle();

    bool insertEntry(void *pData, const RID &rid);
    bool deleteEntry(void *pData, const RID &rid);
    bool forcePages();
};

#endif // __IX_INDEX_HANDLE_H__