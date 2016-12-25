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
    default:
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
    case Node::OP_LIKE:
      return CompOp::LK_OP;
    default:
      return CompOp::NO_OP;
  }
}

GroupFunc funcAdapt(int type){
  switch(type){
    case Node::FUNC_NO:
      return GroupFunc::NO_FUNC;
    case Node::FUNC_AVG:
      return GroupFunc::AVG_FUNC;
    case Node::FUNC_SUM:
      return GroupFunc::SUM_FUNC;
    case Node::FUNC_MIN:
      return GroupFunc::MIN_FUNC;
    case Node::FUNC_MAX:
      return GroupFunc::MAX_FUNC;
    default:
      return GroupFunc::NO_FUNC;
  }
}

TableAttr getColumn(Node *x){
  TableAttr res;
  res.attrName = (x->str).c_str();
  res.tableName = Node::getFlag(x->flag, 1) ? (x->primary).c_str() : NULL; 
  return res;
}

TableAttrEx getColumnExtension(Node *x){
  TableAttrEx res;
  res.attrName = (x->str).c_str();
  res.tableName = Node::getFlag(x->flag, 1) ? (x->primary).c_str() : NULL; 
  res.func = funcAdapt(x->datatype);
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
      //fprintf(stdout, "len = %d, x->str = %s\n", x->str.length(), x->str.c_str());
      x->str += '\0';
      //fprintf(stdout, "len = %d, x->str = %s\n", x->str.length(), x->str.c_str());
      charp = new char[x->str.size()];
      memcpy(charp, x->str.c_str(), x->str.size());
      res.data = (void *)charp;
      //fprintf(stdout, "res.data = %s\n", res.data);
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
  if(Node::getFlag(x->flag, 3)){
    // Null judgement
    if(Node::getFlag(x->flag, 0)){
      // NOT NULL
      res.rIsValue = true;
      res.op = CompOp::NE_OP;  //differ
      res.rValue.attrType = AttrType::NOTYPE;
      res.rValue.data = NULL;
      res.lAttr = getColumn(x->subtree[0]);
    }
    else{
      // IS NULL
      res.rIsValue = true;
      res.op = CompOp::EQ_OP;  //differ
      res.rValue.attrType = AttrType::NOTYPE;
      res.rValue.data = NULL;
      res.lAttr = getColumn(x->subtree[0]);
    }
  }
  else{
    if(Node::getFlag(x->flag, 2)){
      // Column
      res.rIsValue = false;
      res.lAttr = getColumn(x->subtree[0]);
      res.rAttr = getColumn(x->subtree[1]);
      res.op = opAdapt(x->datatype);
    }
    else{
      // Value
      res.rIsValue = true;
      res.op = opAdapt(x->datatype);
      res.lAttr = getColumn(x->subtree[0]);
      res.rValue = getValue(x->subtree[1]);
    }
  }
  return res;
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
  TableAttrEx ttex;
  std::vector<TableAttrEx> texlist;
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
          // if (vt.attrType == AttrType::STRING) {
          //   fprintf(stdout, "vt.data = %s\n", vt.data);
          // }
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
      tlist.clear();
      ptr = subtree[0];
      slist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        slist.push_back(((*i)->str).c_str());
      }
      ptr = subtree[1];
      wlist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        wt = getCondition(*i);
        wlist.push_back(wt);
      }
      qm->select(tlist, slist, wlist);
      break;
    case SELECT_GROUP:
      ptr = subtree[0];
      texlist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        ttex = getColumnExtension(*i);
        texlist.push_back(ttex);
      }
      ptr = subtree[1];
      wlist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        wt = getCondition(*i);
        wlist.push_back(wt);
      }
      qm->selectGB(texlist, primary.c_str(), str.c_str(), wlist);
      break;
    case CONSTRIANT_CHECK:
      ptr = this;
      vlist.clear();
      for(std::vector<Node *>::reverse_iterator i = ptr->subtree.rbegin(); i != ptr->subtree.rend(); ++i){
        vt = getValue(*i);
        vlist.push_back(vt);
      }
      tt.tableName = primary.c_str();
      tt.attrName = str.c_str();
      sm->alterCheck(tt, vlist);
      break;
    case CONSTRIANT_FOREIGN:
      tt = getColumn(subtree[0]);
      tlist.clear();
      tlist.push_back(tt);
      tt.tableName = primary.c_str();
      tt.attrName = str.c_str();
      sm->alterForeign(tt, tlist[0]);
      break;
    default:
      break;
  }
}
