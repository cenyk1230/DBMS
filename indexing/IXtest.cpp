#include <iostream>

#include "IX_Manager.h"
#include "IX_IndexHandle.h"

using namespace std;

int main() {
    IX_Manager *im = new IX_Manager(new FileManager());
    bool flag; 
    flag = im->createIndex("test", 0, INTEGER, 4);
    if (flag) {
        cout << "create index successfully" << endl;
    }
    IX_IndexHandle *indexHandle;
    flag = im->openIndex("test", 0, indexHandle);
    if (flag) {
        cout << "open index successfully" << endl;
    }

    int data1 = 0x3927F85A;
    RID rid1 = RID(1, 0, 0);
    indexHandle->insertEntry((void *)&data1, rid1);
    cout << "insert entry pData = " << &data1 << " RID = (1, 0, 0)" << endl;
    int data2 = 0x28408F21;
    RID rid2 = RID(1, 0, 1);
    indexHandle->insertEntry((void *)&data2, rid2);
    cout << "insert entry pData = " << &data2 << " RID = (1, 0, 1)" << endl;

    flag = indexHandle->deleteEntry((void *)&data1, rid1);
    if (flag) {
        cout << "delete entry successfully  pData = " << &data1 << " RID = (1, 0, 0)" << endl;
    }
    flag = indexHandle->deleteEntry((void *)&data2, rid1);
    if (!flag) {
        cout << "delete entry unsuccessfully  pData = " << &data2 << " RID = (1, 0, 0)" << endl;
    }
    flag = indexHandle->deleteEntry((void *)&data2, rid2);
    if (flag) {
        cout << "delete entry successfully  pData = " << &data2 << " RID = (1, 0, 1)" << endl;
    }

    flag = im->closeIndex(indexHandle);
    if (flag) {
        cout << "close index successfully" << endl;
    }
    flag = im->destroyIndex("test", 0);
    if (flag) {
        cout << "destroy index successfully" << endl;
    }

    delete im;
    return 0;
}