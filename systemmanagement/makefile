HEADERS := ./SM_Manager.h ../indexing/BPlusTree.h ../indexing/IX_IndexHandle.h ../indexing/IX_IndexScan.h ../indexing/IX_Manager.h ../recordmanagement/RM_Manager.h ../recordmanagement/RM_FileHandle.h ../recordmanagement/RM_Record.h ../recordmanagement/RM_FileScan.h 
SOURCES := ./SMtest.cpp ./SM_Manager.cpp ../indexing/BPlusTree.cpp ../indexing/IX_IndexHandle.cpp ../indexing/IX_IndexScan.cpp ../indexing/IX_Manager.cpp ../recordmanagement/RM_Manager.cpp ../recordmanagement/RM_FileHandle.cpp ../recordmanagement/RM_FileScan.cpp
CPPFLAGS := -std=c++11 -O2

all: SMtest

SMtest: $(SOURCES) $(HEADERS)
	g++ $(SOURCES) -o SMtest $(CPPFLAGS)