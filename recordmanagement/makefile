all: RMtest.exe

RMtest.exe: RMtest.cpp ../utils/MyBitMap.cpp RM_FileHandle.cpp RM_Manager.cpp RM_FileHandle.h RM_Manager.h RM_FileScan.h
	g++ RMtest.cpp ../utils/MyBitMap.cpp RM_FileHandle.cpp RM_Manager.cpp RM_FileScan.cpp -o RMtest -O2 -std=c++11