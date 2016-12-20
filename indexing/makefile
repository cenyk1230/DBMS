all: IXtest

IXtest: IXtest.cpp IX_Manager.cpp IX_IndexHandle.cpp IX_IndexScan.cpp BPlusTree.cpp ../utils/MyBitMap.cpp IX_Manager.h IX_IndexHandle.h IX_IndexScan.h BPlusTree.h
	g++ IXtest.cpp IX_Manager.cpp IX_IndexHandle.cpp IX_IndexScan.cpp BPlusTree.cpp ../utils/MyBitMap.cpp -o IXtest -O2 -std=c++11