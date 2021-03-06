#include "IX_IndexScan.h"

IX_IndexScan::IX_IndexScan() {
    dataLength = 0;
    current_pos = 0;
    condition_val_ptr = NULL;
}

IX_IndexScan::~IX_IndexScan() {
    // Do nothing
}

bool IX_IndexScan::satisfy(int indexNum){
    if (condition_op == NO_OP || condition_val_ptr == NULL) {
        return true;
    }
    another_common_type valueHolder_index, valueHolder_condition;
    int stringCompareResult = 0;
    switch(dataType){
        case INTEGER:
            valueHolder_index.integer = *(int *)allIndexes[indexNum].first;
            valueHolder_condition.integer = *(int *)condition_val_ptr;
            switch(condition_op){
                case EQ_OP:
                    return valueHolder_index.integer == valueHolder_condition.integer;
                    break;
                case LT_OP:
                    return valueHolder_index.integer < valueHolder_condition.integer;
                    break;
                case GT_OP: 
                    return valueHolder_index.integer > valueHolder_condition.integer;
                    break;
                case LE_OP: 
                    return valueHolder_index.integer <= valueHolder_condition.integer;
                    break;
                case GE_OP: 
                    return valueHolder_index.integer >= valueHolder_condition.integer;
                    break;
                case NE_OP: 
                    return valueHolder_index.integer != valueHolder_condition.integer;
                    break;
                default: 
                    return false;
                    break;
            }
            break;
        case FLOAT:
            valueHolder_index.real = *(float *)allIndexes[indexNum].first;
            valueHolder_condition.real = *(float *)condition_val_ptr;
            switch(condition_op){
                case EQ_OP:
                    return valueHolder_index.real == valueHolder_condition.real;
                    break;
                case LT_OP:
                    return valueHolder_index.real < valueHolder_condition.real;
                    break;
                case GT_OP: 
                    return valueHolder_index.real > valueHolder_condition.real;
                    break;
                case LE_OP: 
                    return valueHolder_index.real <= valueHolder_condition.real;
                    break;
                case GE_OP: 
                    return valueHolder_index.real >= valueHolder_condition.real;
                    break;
                case NE_OP: 
                    return valueHolder_index.real != valueHolder_condition.real;
                    break;
                default: 
                    return false;
                    break;
            }
            break;
        case STRING:
            valueHolder_index.string = (char *)allIndexes[indexNum].first;
            valueHolder_condition.string = (char *)condition_val_ptr;
            stringCompareResult = 0;
            for (int i = 0; i < dataLength; ++i) {
                if (valueHolder_index.string[i] < valueHolder_condition.string[i]) {
                    stringCompareResult = -1;
                    break;
                } else if (valueHolder_index.string[i] > valueHolder_condition.string[i]) {
                    stringCompareResult = 1;
                    break;
                }
            }
            switch(condition_op){
                case EQ_OP:
                    return stringCompareResult == 0;
                    break;
                case LT_OP:
                    return stringCompareResult == -1;
                    break;
                case GT_OP: 
                    return stringCompareResult == 1;
                    break;
                case LE_OP:
                    return stringCompareResult < 1;                
                    break;
                case GE_OP: 
                    return stringCompareResult > -1;
                    break;
                case NE_OP: 
                    return stringCompareResult != 0;
                    break;
                default: 
                    return false;
                    break;
            }
            break;
        default:
            break;
    }
    return false;
}

bool IX_IndexScan::openScan(IX_IndexHandle *indexHandle, CompOp compOp, void *value) {
    dataType = indexHandle->getAttrType();
    dataLength = indexHandle->getAttrLength();
    current_pos = 0;
    allIndexes.clear();
    condition_op = compOp;
    condition_val_ptr = value;
    indexHandle->getAllEntry(allIndexes);
    return true;
}

bool IX_IndexScan::getNextEntry(RID &rid) {
    //cout << "getNextEntry begin" << endl;
    while(current_pos < allIndexes.size()){
        //cout << current_pos << endl;
        if(satisfy(current_pos)){
            rid = allIndexes[current_pos++].second;
            return true;
        }
        //else
        ++current_pos;
    }
    //cout << "getNextEntry end" << endl;
    return false;
}

bool IX_IndexScan::closeScan() {
    dataLength = 0;
    current_pos = 0;
    condition_val_ptr = NULL;
    return true;
}