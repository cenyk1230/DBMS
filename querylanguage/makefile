HEADERS := ./userdef.h ./QL_Manager.h ../systemmanagement/SM_Manager.h ../indexing/BPlusTree.h ../indexing/IX_IndexHandle.h ../indexing/IX_IndexScan.h ../indexing/IX_Manager.h ../recordmanagement/RM_Manager.h ../recordmanagement/RM_FileHandle.h ../recordmanagement/RM_Record.h ../recordmanagement/RM_FileScan.h ../utils/RID.h ../utils/base.h ../bufmanager/BufPageManager.h ../bufmanager/FindReplace.h ../fileio/FileManager.h ../fileio/FileTable.h
SOURCES := ./compile.tab.c ./visit.cpp ./compiletest.cpp ./QL_Manager.cpp ../systemmanagement/SM_Manager.cpp ../indexing/BPlusTree.cpp ../indexing/IX_IndexHandle.cpp ../indexing/IX_IndexScan.cpp ../indexing/IX_Manager.cpp ../recordmanagement/RM_Manager.cpp ../recordmanagement/RM_FileHandle.cpp ../recordmanagement/RM_FileScan.cpp ../utils/MyBitMap.cpp
CPPFLAGS := -std=c++11 -O2

all: QLtest

QLtest: $(SOURCES) $(HEADERS)
	g++ $(SOURCES) -o QLtest $(CPPFLAGS)

compile.tab.c: compile.y
	bison compile.y
clean:
	rm QLtest
run:
	./QLtest