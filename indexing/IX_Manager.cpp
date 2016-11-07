#include <string>

#include "IX_Manager.h"

using namespace std;

IX_Manager::IX_Manager(FileManager *fm) {
    mFileManager = fm;
    mBufPageManager = new BufPageManager(fm);
}

IX_Manager::~IX_Manager() {
    delete mFileManager;
    delete mBufPageManager;
}

string IX_Manager::getIndexFileName(const char *fileName, int indexNo) {
    string fileNameStr = fileName;
    char indexBuf[20];
    sprintf(indexBuf, "%d", indexNo);
    string indexNoStr = indexBuf;
    return fileNameStr + indexNoStr;
}

bool IX_Manager::createIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength) {
    string indexFileName = getIndexFileName(fileName, indexNo);
    bool flag = mFileManager->createFile(indexFileName.c_str());
    if (!flag) {
        return false;
    } 
    int fileID;
    flag = mFileManager->openFile(indexFileName.c_str(), fileID);
    if (!flag) {
        return false;
    }
    int index;
    BufType data = mBufPageManager->allocPage(fileID, 0, index, false);
    data[0] = (unsigned)attrType;
    data[1] = (unsigned)attrLength;
    data[2] = (unsigned)1;
    mBufPageManager->markDirty(index);
    mBufPageManager->writeBack(index);
    mFileManager->closeFile(fileID);
    return true;
}

bool IX_Manager::destroyIndex(const char *fileName, int indexNo) {
    string indexFileName = getIndexFileName(fileName, indexNo);
    return mFileManager->destroyFile(indexFileName.c_str());
}

bool IX_Manager::openIndex(const char *fileName, int indexNo, IX_IndexHandle *&indexHandle) {
    string indexFileName = getIndexFileName(fileName, indexNo);
    int fileID;
    bool flag = mFileManager->openFile(indexFileName.c_str(), fileID);
    if (!flag) {
        return false;
    }
    indexHandle = new IX_IndexHandle(mBufPageManager, fileID);
    return true;
}

bool IX_Manager::closeIndex(IX_IndexHandle *indexHandle) {
    int fileID = indexHandle->getFileID();
    mBufPageManager->close();
    delete indexHandle;
    return mFileManager->closeFile(fileID);
}