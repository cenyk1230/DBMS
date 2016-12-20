#include "userdef.h"
#include <vector>
#include <cstdio>
#include "../systemmanagement/SM_Manager.h"
#include "./QL_Manager.h"

extern SM_Manager *sm;
extern QL_Manager *qm;

AttrType adapt(int type){
  switch(type){
    case Node::INTEGER:
      return AttrType::INTEGER;
    case Node::FLOAT:
      return AttrType::FLOAT;
    case Node::VARCHAR:
    case Node::STRING:
      return AttrType::STRING;
  }
}

CompOp opAdapt(int type){
  switch(type){
    case Node::OP_EQU:
      return CompOp::EQ_OP;
    case Node::OP_NEQ:
      return CompOp::NE_OP;
    case Node::OP_LEQ:
      return CompOp::LE_OP;
    case Node::OP_GEQ:
      return CompOp::GE_OP;
    case Node::OP_LES:
      return CompOp::LT_OP;
    case Node::OP_GTR:
      return CompOp::GT_OP;
  }
}

void StmtNode::visit(){
  std::vector<Column> clist; 
  Column t;
  AttrType t_type;
  std::vector<Value> vlist;
  Value vt;
  std::vector<Condition> wlist;
  Condition wt;
  TableAttr tt;
  int *intp;
  char *charp;
  switch(stmttype){
    case CREATE_DATABASE:
      sm->createDB(str.c_str());
      break;
    case USE_DATABASE:
      sm->useDB(str.c_str());
      break;
    case DROP_DATABASE:
      sm->dropDB(str.c_str());
      break;
    case SHOW_DATABASE:
      sm->showDB(str.c_str());
      break;
    case CREATE_TABLE:
      for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
        t_type = adapt((*i)->datatype);
        t.attrType = t_type;
        t.attrName = ((*i)->str).c_str();
        t.nullable = ((*i)->flag&1) == 0;
        t.attrLength = (*i)->number;
        clist.push_back(t);
      }
      if(primary.empty()){
        sm->createTable(str.c_str(), "", clist);
      } 
      else{
        sm->createTable(str.c_str(), primary.c_str(), clist);
      }
      break;
    case DROP_TABLE:
      sm->dropTable(str.c_str());
      break;
    case SHOW_TABLE:
      sm->showTable(str.c_str());
      break;
    case SHOW_DATABASE_ALL:
      //TODO: add interface
      break;
    case INSERT:
      for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
        vlist.clear();
        for(std::vector<Node *>::reverse_iterator j = (*i)->subtree.rbegin(); j != (*i)->subtree.rend(); ++j){
          vt.attrType = adapt((*j)->datatype);
          switch((*j)->datatype){
            case Node::INTEGER:
              intp = new int;
              *intp = (*j)->number;
              vt.data = (void *)intp;
              intp = NULL;
              break;
            case Node::VARCHAR:
            case Node::STRING:
              (*j)->str += "\0";  // Add NULL
              
              charp = new char[((*j)->str).size()];
              vt.data = (void *)charp;
              memcpy(charp, ((*j)->str).c_str(), ((*j)->str).size());
              charp = NULL;
              break;
            default:
              break;
          }
          vlist.push_back(vt);
        }
        qm->insert(str.c_str(), vlist);
        for(std::vector<Value>::iterator j = vlist.begin(); j != vlist.end(); ++j){
          switch((*j).attrType){
            case AttrType::INTEGER:
              intp = (int *)(*j).data;
              delete intp;
              intp = NULL;
              break;
            case AttrType::STRING:
              charp = (char *)(*j).data;
              delete[] charp;
              charp = NULL;
              break;
          }
        }
      }
      vlist.clear();
      break;
    case DELETE:
      wlist.clear();
      for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
        if(getFlag(flag, 3)){
          // Null judgement
        }
        else{
          if(getFlag(flag, 2)){
            // Column
            wt.rIsValue = false;
            if(getFlag((*i)->subtree[0]->flag, 1)){
              tt.tableName = (*i)->subtree[0]->primary.c_str();
            }
            else{
              tt.tableName = NULL;
            }
            tt.attrName = (*i)->subtree[0]->str.c_str();
            wt.lAttr = tt;
            if(getFlag((*i)->subtree[1]->flag, 1)){
              tt.tableName = (*i)->subtree[1]->primary.c_str();
            }
            else{
              tt.tableName = NULL;
            }
            tt.attrName = (*i)->subtree[1]->str.c_str();
            wt.rAttr = tt;
            wt.op = opAdapt((*i)->datatype);
          }
          else{
            // Value
            wt.rIsValue = true;
            wt.op = opAdapt((*i)->datatype);
            if(getFlag((*i)->subtree[0]->flag, 1)){
              tt.tableName = (*i)->subtree[0]->primary.c_str();
            }
            else{
              tt.tableName = NULL;
            }
            tt.attrName = (*i)->subtree[0]->str.c_str();
            wt.lAttr = tt;
            // TODO: value assignment
            switch((*i)->subtree[1]->datatype){
              case Node::INTEGER:
                vt.attrType = AttrType::INTEGER;
                intp = new int;
                *intp = (*i)->subtree[1]->number;
                vt.data = (void *) intp;
                intp = NULL;
                break;
              case Node::VARCHAR:
              case Node::STRING:
                vt.attrType = AttrType::STRING;
                (*i)->subtree[1]->str += "\0";  // Add NULL
                charp = new char[((*i)->subtree[1]->str).size()];
                memcpy(charp, ((*i)->subtree[1]->str).c_str(), ((*i)->subtree[1]->str).size());
                vt.data = (void *) charp;
                charp = NULL;
                break;
            }
            wt.rValue = vt;
          }
        }
        wlist.push_back(wt);
      }
      qm->remove(str.c_str(), wlist);
      for(std::vector<Condition>::iterator i = wlist.begin(); i != wlist.end(); ++i){
        if((*i).rIsValue){
          switch(((*i).rValue).attrType){
            case AttrType::INTEGER:
              intp = (int *)((*i).rValue).data;
              delete intp;
              intp = NULL;
              break;
            case AttrType::STRING:
              charp = (char *)((*i).rValue).data;
              delete []charp;
              charp = NULL;
              break;
            default:
              break;
          }
        }
      }
      break;
    default:
      break;
  }
}
