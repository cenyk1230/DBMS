#include "userdef.h"
#include <vector>
#include <cstdio>
#include "SM_Manager.h"

extern SM_Manager *sm;

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
void StmtNode::visit(){
  std::vector<Column> clist; 
  Column t;
  AttrType t_type;
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
    default:
      break;
  }
}
