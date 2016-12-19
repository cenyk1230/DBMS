#ifndef USERDEF_H_
#define USERDEF_H_
#include <string>
#include <vector>
#include "SM_Manager.h"

typedef AttrInfo Column;
class Node{
  protected:
  const static int DATA = 0;
  const static int STMT = 1;
  std::string toString(int type){
    std::string res;
    switch(type){
      case 0:
        res = "int";
        break;
      case 1:
        res = "varchar";
        break;
      case 2:
        res = "float";
        break;
      case 3:
        res = "string";
        break;
      default:
        res = "";
        break;
    }
    return res;
  }
  public:
  int number;
  std::string str;
  std::string primary;
  std::vector<Node *> subtree;
  /*
    Flags:
    bit 0:
      0 -- nullable
      1 -- non-nullable    
  */
  char flag;



  const static int INTEGER = 0;
  const static int VARCHAR = 1;
  const static int FLOAT = 2;
  const static int STRING = 3;

  const static int CREATE_DATABASE = 0;
  const static int USE_DATABASE = 1;
  const static int DROP_DATABASE = 2;
  const static int SHOW_DATABASE = 3;
  const static int CREATE_TABLE = 4;
  const static int DROP_TABLE = 5;
  const static int SHOW_TABLE = 6;
  const static int SHOW_DATABASE_ALL = 7;

  const static int OP_EQU = 0;
  const static int OP_NEQ = 1;
  const static int OP_LEQ = 2;
  const static int OP_GEQ = 3;
  const static int OP_LES = 4;
  const static int OP_GTR = 5;

  int stmttype;
  int datatype;
  Node(){
    flag = 0;
    stmttype = datatype = -1;
  }
  virtual void print(){
  }
  virtual void visit(){
  }
  virtual ~Node(){
    for(std::vector<Node *>::iterator i = subtree.begin(); i != subtree.end(); ++i){
      if(*i != NULL){
        delete *i;
        *i = NULL;
      }
    }
  }
};
class ColumnNode: public Node{
  public:
  void print(){
    printf("COLUMN MAP: (name=>%s, type=>%s, length=>%d", str.c_str(), toString(datatype).c_str(), number);
    if(flag & 1 != 0){
      printf(", nullable=>FALSE");
    }
    printf(")\n");
  }
  virtual ~ColumnNode(){
    for(std::vector<Node *>::iterator i = subtree.begin(); i != subtree.end(); ++i){
      if(*i != NULL){
        delete *i;
        *i = NULL;
      }
    }
  }
};
class ColumnListNode: public Node{
  public:
  void print(){
    printf("Start of Column List\n");
    for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
      (*i)->print();
    }
    printf("End of Column List\n");
  }
  virtual ~ColumnListNode(){
    for(std::vector<Node *>::iterator i = subtree.begin(); i != subtree.end(); ++i){
      if(*i != NULL){
        delete *i;
        *i = NULL;
      }
    }
  }
};

class StmtNode: public Node{
  public:
  void print(){
    switch(stmttype){
      case CREATE_DATABASE:
        printf("CREATE DATABASE %s\n", str.c_str());
        break;
      case USE_DATABASE:
        printf("USE DATABASE %s\n", str.c_str());
        break;
      case DROP_DATABASE:
        printf("DROP DATABASE %s\n", str.c_str());
        break;
      case SHOW_DATABASE:
        printf("SHOW DATABASE %s\n", str.c_str());
        break;
      case CREATE_TABLE:
        printf("TABLE %s IS\n", str.c_str());
        printf("BEGIN\n");
        for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
          (*i)->print();
        }
        if(!primary.empty()){
          printf("KEY %s IS PRIMARY\n", primary.c_str());
        } 
        printf("END TABLE\n");
        break;
      case DROP_TABLE:
        printf("DROP TABLE %s\n", str.c_str());
        break;
      case SHOW_TABLE:
        printf("SHOW TABLE %s\n", str.c_str());
        break;
      default:
        break;
    }
  }
  void visit();
  
  virtual ~StmtNode(){
    for(std::vector<Node *>::iterator i = subtree.begin(); i != subtree.end(); ++i){
      if(*i != NULL){
        delete *i;
        *i = NULL;
      }
    }
  }
};

class StmtListNode: public Node{
  public:
  void print(){
    printf("BEGIN\n");
    for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
      (*i)->print();
    }
    printf("END SCRIPT\n");
  }
  void visit(){
    for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
      (*i)->visit();
    }
  }
  virtual ~StmtListNode(){
    for(std::vector<Node *>::iterator i = subtree.begin(); i != subtree.end(); ++i){
      if(*i != NULL){
        delete *i;
        *i = NULL;
      }
    }
  }
};

#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    CREATE = 258,
    TABLE = 259,
    PRIMARY = 260,
    KEY = 261,
    NOT = 262,
    NULLSIGN = 263,
    INT_INPUT = 264,
    VARCHAR_INPUT = 265,
    NUMBER = 266,
    IDENTIFIER = 267,
    DATABASE = 268,
    DROP = 269,
    SHOW = 270,
    USE = 271,
    FLOAT_INPUT = 272,
    STRING_INPUT = 273
  };
#endif

#endif

