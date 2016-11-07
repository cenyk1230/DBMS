#ifndef __BASE_H__
#define __BASE_H__

enum AttrType {
	INTEGER = 0,
	FLOAT = 1,
	STRING = 2
};

enum CompOp {
	EQ_OP,
	LT_OP,
	GT_OP,
	LE_OP,
	GE_OP,
	NE_OP,
	NO_OP
};

#endif // __BASE_H__