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
    case Node::NULLDATA:
      return AttrType::NOTYPE;
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

TableAttr getColumn(Node *x){
  TableAttr res;
  res.attrName = (x->str).c_str();
  res.tableName = Node::getFlag(x->flag, 1) ? (x->primary).c_str() : NULL; 
  return res;
}

Value getValue(Node *x){
  Value res;
  res.attrType = adapt(x->datatype);
  int *intp;
  char *charp;
  switch(x->datatype){
    case Node::INTEGER:
      intp = new int;
      *intp = x->number;
      res.data = (void *)intp;
      intp = NULL;
      break;
    case Node::VARCHAR:
    case Node::STRING:
      x->str += "\0";
      charp = new char[x->str.size()];
      memcpy(charp, x->str.c_str(), x->str.size());
      res.data = (void *)charp;
      charp = NULL;
      break;
    case Node::NULLDATA:
      res.data = NULL;
      break;
    default:
      res.data = NULL;
      break;
  }
  return res;
}

Condition getCondition(Node *x){
  Condition res;
  for(std::vector<Node *>::reverse_iterator i = x->subtree.rbegin(); i != x->subtree.rend(); ++i){
    if(Node::getFlag(x->flag, 3)){
      // Null judgement
      if(Node::getFlag(x->flag, 0)){
        // NOT NULL
        res.rIsValue = true;
        res.op = CompOp::NE_OP;  //differ
        res.rValue.attrType = AttrType::NOTYPE;
        res.rValue.data = NULL;
        res.lAttr = getColumn((*i)->subtree[0]);
      }
      else{
        // IS NULL
        res.rIsValue = true;
        res.op = CompOp::EQ_OP;  //differ
        res.rValue.attrType = AttrType::NOTYPE;
        res.rValue.data = NULL;
        res.lAttr = getColumn((*i)->subtree[0]);
      }
    }
    else{
      if(Node::getFlag(x->flag, 2)){
        // Column
        res.rIsValue = false;
        res.lAttr = getColumn((*i)->subtree[0]);
        res.rAttr = getColumn((*i)->subtree[1]);
        res.op = opAdapt((*i)->datatype);
      }
      else{
        // Value
        res.rIsValue = true;
        res.op = opAdapt((*i)->datatype);
        res.lAttr = getColumn((*i)->subtree[0]);
        res.rValue = getValue((*i)->subtree[1]);
      }
    }
  }
}

void releaseValue(Value &v){
  int *intp;
  char *charp;
  switch(v.attrType){
    case AttrType::INTEGER:
      intp = (int *)v.data;
      delete intp;
      intp = NULL;
      break;
    case AttrType::STRING:
      charp = (char *)v.data;
      delete[] charp;
      charp = NULL;
      break;
    default:
      break;
  }
}

void releaseCondition(Condition &c){
  if(c.rIsValue){
    releaseValue(c.rValue);
  }
}

void StmtNode::visit(){
  std::vector<Column> clist; 
  Column t;
  AttrType t_type;
  std::vector<Value> vlist;
  std::vector<std::vector<Value> > vvlist;
  Value vt;
  std::vector<Condition> wlist;
  Condition wt;
  TableAttr tt;
  std::vector<TableAttr> tlist;
  std::vector<const char *> slist;
  Node *ptr;
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
    case INSERT_DATA:
      vvlist.clear();
      for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
        vlist.clear();
        for(std::vector<Node *>::reverse_iterator j = (*i)->subtree.rbegin(); j != (*i)->subtree.rend(); ++j){
          vt = getValue(*j);
          vlist.push_back(vt);
        }
        vvlist.push_back(vlist);
      }
      qm->insert(str.c_str(), vvlist);
      for(std::vector<std::vector<Value> >::iterator i = vvlist.begin(); i != vvlist.end(); ++i){
        for(std::vector<Value>::iterator j = (*i).begin(); j != (*i).end(); ++j){
          releaseValue(*j);
        }
      }
      break;
    case DELETE_DATA:
      wlist.clear();
      for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
        wt = getCondition(*i);
        wlist.push_back(wt);
      }
      qm->remove(str.c_str(), wlist);
      for(std::vector<Condition>::iterator i = wlist.begin(); i != wlist.end(); ++i){
        releaseCondition(*i);
      }
      break;
    case UPDATE_DATA:
      tt = getColumn(subtree[0]);
      vt = getValue(subtree[1]);
      wlist.clear();
      ptr = subtree[2];
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        wt = getCondition(*i);
        wlist.push_back(wt);
      }
      qm->update(str.c_str(), tt, vt, wlist);
      for(std::vector<Condition>::iterator i = wlist.begin(); i != wlist.end(); ++i){
        releaseCondition(*i);
      }
      break;
    case SELECT_DATA:
      ptr = subtree[0];
      tlist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        tt = getColumn(*i);
        tlist.push_back(tt);
      }
      ptr = subtree[1];
      slist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        slist.push_back(((*i)->str).c_str());
      }
      ptr = subtree[2];
      wlist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        wt = getCondition(*i);
        wlist.push_back(wt);
      }
      qm->select(tlist, slist, wlist);
      for(std::vector<Condition>::iterator i = wlist.begin(); i != wlist.end(); ++i){
        releaseCondition(*i);
      }
      break;
    case SELECT_DATA_ALL:
      ptr = subtree[0];
      tlist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        tt = getColumn(*i);
        tlist.push_back(tt);
      }
      ptr = subtree[1];
      slist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        slist.push_back(((*i)->str).c_str());
      }
      wlist.clear();
      qm->select(tlist, slist, wlist);
      break;
    default:
      break;
  }
}
