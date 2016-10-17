#include "RM_Manager.h"
#include "RM_FileHandle.h"

int main(){
	FileManager *f = new FileManager();
	RM_Manager *manager = new RM_Manager(f);
	RM_FileHandle *handle = NULL;
	manager->createFile("cyk1230.dbms", 4);
	/*
	manager->openFile("cyk1230.dbms", handle);
	int data = 0x4566A1;
	RID id;
	//handle->insertRec((const char *)&data, id);
	data = 0x933342;
	//handle->insertRec((const char *)&data, id);
	manager->closeFile(handle);
	*/
	
	delete manager;
	delete f;
}