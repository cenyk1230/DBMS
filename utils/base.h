#ifndef __BASE_H__
#define __BASE_H__

const int MAX_NAME_LEN = 20;
const double EPS = 1e-6;

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

struct AttrInfo {
	bool nullable;
	const char *attrName;
	AttrType attrType;
	int attrLength;
};

struct AttrInfoEx {
	bool nullable;
	const char *attrName;
	int offset;
	AttrType attrType;
	int attrLength;
	int indexNo;
};

struct TableAttr {
	const char *tableName;
	const char *attrName;
};

struct Value {
	AttrType attrType;
	void *data;
};

struct Condition {
	TableAttr lAttr;
	CompOp op;
	bool rIsValue;
	TableAttr rAttr;
	Value rValue;
};

#endif // __BASE_H__