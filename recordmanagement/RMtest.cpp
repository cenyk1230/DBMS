#include <iostream>
#include <cstdio>

#include "RM_Record.h"
#include "RM_Manager.h"
#include "RM_FileHandle.h"
#include "RM_FileScan.h"

#include "../utils/base.h"

using namespace std;

int main() {
	FileManager *fm = new FileManager();
    BufPageManager *bpm = new BufPageManager(fm);
	RM_Manager *manager = new RM_Manager(fm, bpm);
	RM_FileHandle *handle = NULL;
	manager->createFile("cyk1230.dbms", 4);

	manager->openFile("cyk1230.dbms", handle);
	int data1 = 0x124566A1, data2 = 0x56933342, data3 = 0x2C392821;
	RID rid1, rid2;
	RM_Record rec;
	handle->insertRec((const char *)&data1, rid1);
	handle->insertRec((const char *)&data2, rid2);

	cout << "deleteRec: " << handle->deleteRec(rid2) << endl;
	cout << "getRec: " << handle->getRec(rid2, rec) << endl;

	cout << "updateRec: " << handle->updateRec(RM_Record((const char *)&data3, 4, rid1)) << endl;
	cout << "getRec: " << handle->getRec(rid1, rec) << endl;
	fprintf(stdout, "data = 0x%x\n", *((int *)rec.getData()));

	cout << "forcePage: " << handle->forcePage(0) << endl;

	RM_FileScan *scan = new RM_FileScan();
	int data4 = 0x2B194837;
	handle->insertRec((const char *)&data2, rid2);
	scan->openScan(handle, INTEGER, 4, 0, GT_OP, (void *)&data4);

	cout << "getNextRec: " << scan->getNextRec(rec) << endl;
	fprintf(stdout, "data = 0x%x\n", *((int *)rec.getData()));
	cout << "getNextRec: " << scan->getNextRec(rec) << endl;
	fprintf(stdout, "data = 0x%x\n", *((int *)rec.getData()));
	cout << "getNextRec: " << scan->getNextRec(rec) << endl;

	scan->closeScan();
	delete scan;
	manager->closeFile(handle);

	delete fm;
	delete bpm;
	delete manager;
	return 0;
}