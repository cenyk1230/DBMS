#include <cassert>

#include "IX_IndexHandle.h"

IX_IndexHandle::IX_IndexHandle(BufPageManager *bpm, int fileID) {
    mBufPageManager = bpm;
    mFileID = fileID;
    int index;
    BufType uData = mBufPageManager->allocPage(mFileID, 0, index, true);
    mAttrType = (AttrType)uData[0];
    mAttrLength = (int)uData[1];
    mPageNum = (int)uData[2];
    //cout << "pagenum: " << mPageNum << endl;
    mBufPageManager->release(index);
    mBranch = (PAGE_SIZE - 24) / (16 + mAttrLength);
    mBPlusTree = new BPlusTree(mBranch, mAttrType, mAttrLength);
    loadBPlusTree();
}

IX_IndexHandle::~IX_IndexHandle() {
    delete mBPlusTree;
}

void IX_IndexHandle::printBPlusTree() {
    mBPlusTree->print(mBPlusTree->getRoot());
}

void IX_IndexHandle::storeHeaderPage() {
    int index;
    BufType uData = mBufPageManager->allocPage(mFileID, 0, index, false);
    uData[0] = (unsigned)mAttrType;
    uData[1] = (unsigned)mAttrLength;
    uData[2] = (unsigned)mPageNum;
    mBufPageManager->markDirty(index);
}

void IX_IndexHandle::insertEntry(void *pData, const RID &rid) {
    mBPlusTree->insertEntry(pData, rid);
}

bool IX_IndexHandle::deleteEntry(void *pData, const RID &rid) {
    return mBPlusTree->deleteEntry(pData, rid);
}

bool IX_IndexHandle::findEntry(void *pData) {
    return mBPlusTree->findEntry(pData);
}

bool IX_IndexHandle::forcePages() {
    //printBPlusTree();
    storeBPlusTree();
    return true;
}

int IX_IndexHandle::getFileID() const {
    return mFileID;
}

AttrType IX_IndexHandle::getAttrType() const {
    return mAttrType;
}

int IX_IndexHandle::getAttrLength() const {
    return mAttrLength;
}

void IX_IndexHandle::getAllEntry(vector<pair<void *, RID> > &entries) {
    mBPlusTree->getAllEntry(entries);
}

void IX_IndexHandle::loadBPlusTree() {
    //cout << "loadBPlusTree begin" << endl;
    int index;
    BPlusNode **nodes = new BPlusNode*[mPageNum];
    nodes[1] = mBPlusTree->getRoot();
    for (int i = 2; i < mPageNum; ++i) {
        nodes[i] = new BPlusNode(mBranch, mAttrType, mAttrLength);
    }
    for (int i = 1; i < mPageNum; ++i) {
        BufType uData = mBufPageManager->allocPage(mFileID, i, index, true);
        nodes[i]->isLeaf = (bool)uData[0];
        //cout << "nodes[i]->isLeaf: " << nodes[i]->isLeaf << endl;
        nodes[i]->num = (int)uData[1];
        //cout << "nodes[i]->num: " << nodes[i]->num << endl;
        nodes[i]->no = (int)uData[2];
        if ((int)uData[3] == 0)
            nodes[i]->parent = NULL;
        else
            nodes[i]->parent = nodes[(int)uData[3]];
        if ((int)uData[4] == 0)
            nodes[i]->left = NULL;
        else
            nodes[i]->left = nodes[(int)uData[4]];
        if ((int)uData[5] == 0)
            nodes[i]->right = NULL;
        else
            nodes[i]->right = nodes[(int)uData[5]];
        
        for (int j = 0; j < nodes[i]->num; ++j) {
            if (mAttrType == INTEGER || mAttrType == FLOAT) {
                nodes[i]->setKey(j, &uData[6 + j * 5]);
                nodes[i]->data[j].fileID = (int)uData[6 + j * 5 + 1];
                nodes[i]->data[j].pageID = (int)uData[6 + j * 5 + 2];
                nodes[i]->data[j].slotID = (int)uData[6 + j * 5 + 3];
                //cout << nodes[i]->data[j].fileID << " " << nodes[i]->data[j].pageID << " " << nodes[i]->data[j].slotID << endl;
                nodes[i]->son[j] = NULL;
                if (!nodes[i]->isLeaf) {
                    //cout << (int)uData[6 + j * 5 + 4] << endl;
                    assert((int)uData[6 + j * 5 + 4] > 0);
                    nodes[i]->son[j] = nodes[(int)uData[6 + j * 5 + 4]];
                } 
            } else if (mAttrType == STRING) {
                char *data = (char *)uData;
                data = data + 24 + j * (mAttrLength + 16);
                nodes[i]->setKey(j, data);
                data = data + mAttrLength;
                int *tmpData = (int *)data;
                nodes[i]->data[j].fileID = tmpData[0];
                nodes[i]->data[j].pageID = tmpData[1];
                nodes[i]->data[j].slotID = tmpData[2];
                nodes[i]->son[j] = NULL;
                if (!nodes[i]->isLeaf) {
                    assert(tmpData[3] > 0);
                    nodes[i]->son[j] = nodes[tmpData[3]];
                }
            }
        }
        mBufPageManager->release(index);
    }
    delete[] nodes;
    //cout << "loadBPlusTree end" << endl;
}

void IX_IndexHandle::storeBPlusTree() {
    //cout << "storeBPlusTree begin" << endl;
    BPlusNode *root = mBPlusTree->getRoot();
    mTotal = 0;
    calcNo(root);
    storeBPlusNode(root);
    mPageNum = mTotal + 1;
    storeHeaderPage();
    mBufPageManager->close();
    //cout << "storeBPlusTree end" << endl;
}

void IX_IndexHandle::calcNo(BPlusNode *node) {
    //cout << "calcNo begin: " << node << endl;
    ++mTotal;
    node->no = mTotal;
    if (node->isLeaf) {
        return;
    }
    for (int i = 0; i < node->num; ++i) {
        calcNo(node->son[i]);
    }
    //cout << "calcNo end: " << node << endl;
}

void IX_IndexHandle::storeBPlusNode(BPlusNode *node) {
    //cout << "storeBPlusNode begin: " << node << endl;
    int index;
    BufType uData = mBufPageManager->allocPage(mFileID, node->no, index, false);
    uData[0] = (unsigned)node->isLeaf;
    uData[1] = (unsigned)node->num;
    uData[2] = (unsigned)node->no;
    if (node->parent != NULL)
        uData[3] = (unsigned)node->parent->no;
    else
        uData[3] = 0;
    if (node->left != NULL)
        uData[4] = (unsigned)node->left->no;
    else
        uData[4] = 0;
    if (node->right != NULL)
        uData[5] = (unsigned)node->right->no;
    else
        uData[5] = 0;
    
    for (int i = 0; i < node->num; ++i) {
        if (mAttrType == INTEGER || mAttrType == FLOAT) {
            uData[6 + i * 5] = *(unsigned *)(node->key[i]);
            uData[6 + i * 5 + 1] = (unsigned)node->data[i].fileID;
            uData[6 + i * 5 + 2] = (unsigned)node->data[i].pageID;
            uData[6 + i * 5 + 3] = (unsigned)node->data[i].slotID;
            uData[6 + i * 5 + 4] = 0;
            if (!node->isLeaf) {
                assert(node->son[i] != NULL);
                uData[6 + i * 5 + 4] = (unsigned)node->son[i]->no;
            }
        } else if (mAttrType == STRING) {
            char *data = (char *)uData;
            data = data + 24 + i * (mAttrLength + 16);
            for (int j = 0; j < mAttrLength; ++j) {
                data[j] = ((char *)node->key[i])[j];
            }
            data = data + mAttrLength;
            unsigned *tmpData = (unsigned *)data;
            tmpData[0] = (unsigned)node->data[i].fileID;
            tmpData[1] = (unsigned)node->data[i].pageID;
            tmpData[2] = (unsigned)node->data[i].slotID;
            tmpData[3] = 0;
            if (!node->isLeaf) {
                assert(node->son[i] != NULL);
                tmpData[3] = (unsigned)node->son[i]->no;
            }
        }
    }
    mBufPageManager->markDirty(index);

    if (node->isLeaf) {
        return;
    }

    for (int i = 0; i < node->num; ++i) {
        storeBPlusNode(node->son[i]);
    }
    //cout << "storeBPlusNode end: " << node << endl;
}
