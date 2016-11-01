#include "IX_Manager.h"

IX_Manager::IX_Manager(FileManager *fm) {
    mFileManager = fm;
    mBufPageManager = new BufPageManager(fm);
}

IX_Manager::~IX_Manager() {
    delete mFileManager;
    delete mBufPageManager;
}

bool IX_Manager::createIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength) {
    
}

bool IX_Manager::destroyIndex(const char *fileName, int indexNo) {

}

bool IX_Manager::openIndex(const char *fileName, int indexNo, IX_IndexHandle *&indexHandle) {

}

bool IX_Manager::closeIndex(IX_IndexHandle *indexHandle) {
    
}