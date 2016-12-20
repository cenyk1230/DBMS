#ifndef USERDEF_H_
#define USERDEF_H_
#include <string>
#include <vector>
#include "../systemmanagement/SM_Manager.h"

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

  std::string OpToString(int type){
    std::string res;
    switch(type){
      case OP_EQU:
        res = "=";
        break;
      case OP_NEQ:
        res = "!=";
        break;
      case OP_LEQ:
        res = "<=";
        break;
      case OP_GEQ:
        res = ">=";
        break;
      case OP_LES:
        res = "<";
        break;
      case OP_GTR:
        res = ">";
        break;
      default:
        res = "-*- error -*-";
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
      
    bit 1:
      0 -- direct access to column (current table)
      1 -- full access 

    bit 2:
      0 -- Right operand is value
      1 -- Right operand is column access

    bit 3:
      0 -- Where clause is logical statement
      1 -- Where clause is null judgement
  */
  char flag;



  const static int INTEGER = 0;
  const static int VARCHAR = 1;
  const static int FLOAT = 2;
  const static int STRING = 3;
  const static int NULLDATA = 4;

  const static int CREATE_DATABASE = 0;
  const static int USE_DATABASE = 1;
  const static int DROP_DATABASE = 2;
  const static int SHOW_DATABASE = 3;
  const static int CREATE_TABLE = 4;
  const static int DROP_TABLE = 5;
  const static int SHOW_TABLE = 6;
  const static int SHOW_DATABASE_ALL = 7;
  const static int INSERT = 8;
  const static int DELETE = 9;

  const static int OP_EQU = 0;
  const static int OP_NEQ = 1;
  const static int OP_LEQ = 2;
  const static int OP_GEQ = 3;
  const static int OP_LES = 4;
  const static int OP_GTR = 5;

  int stmttype;
  int datatype;

  inline static char setFlag(char flag, int bit, bool val){
    char res;
    bit = bit % 8;
    if(val == true){
      res = flag | (1 << bit);
    }
    else{
      res = flag & (~(1 << bit));
    }
    return res;
  }

  inline static bool getFlag(char flag, int bit){
    bit = bit % 8;
    return ((flag >> bit) & 1) == 1;
  }

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
    if((flag & 1) != 0){
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

class WhereListNode: public Node{
  public:
  void print(){
    printf("Where ");
    for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
      if(i != subtree.rbegin())
        printf("AND ");
      (*i)->print();
    }
  }
  virtual ~WhereListNode(){
    for(std::vector<Node *>::iterator i = subtree.begin(); i != subtree.end(); ++i){
      if(*i != NULL){
        delete *i;
        *i = NULL;
      }
    }
  }
};

class WhereNode: public Node{
  public:
  void print(){
    if(getFlag(flag, 3)){
      if(getFlag(flag, 0)){
        subtree[0]->print();
        printf(" is NOT null\n");
      }
      else{
        subtree[0]->print();
        printf(" is null\n");
      }
    }
    else{
      if(getFlag(flag, 2)){
        subtree[0]->print();
        printf(" %s ", OpToString(datatype).c_str());
        subtree[1]->print();
        printf("\n");
      }
      else{
        subtree[0]->print();
        printf(" %s ", OpToString(datatype).c_str());
        subtree[1]->print();
        printf("\n");
      }
    }
  }
  virtual ~WhereNode(){
    for(std::vector<Node *>::iterator i = subtree.begin(); i != subtree.end(); ++i){
      if(*i != NULL){
        delete *i;
        *i = NULL;
      }
    }
  }
};

class RowsNode: public Node{
  public:
  void print(){
    printf("Start of Row List\n");
    for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
      (*i)->print();
    }
    printf("End of Row List\n");
  }
  virtual ~RowsNode(){
    for(std::vector<Node *>::iterator i = subtree.begin(); i != subtree.end(); ++i){
      if(*i != NULL){
        delete *i;
        *i = NULL;
      }
    }
  }
};

class RowNode: public Node{
  public:
  void print(){
    for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
      (*i)->print();
      printf("|");
    }
    printf("\n");
  }
  
  
  virtual ~RowNode(){
    for(std::vector<Node *>::iterator i = subtree.begin(); i != subtree.end(); ++i){
      if(*i != NULL){
        delete *i;
        *i = NULL;
      }
    }
  }
};

class AccessNode: public Node{
  public:
  void print(){
    if(getFlag(flag, 1)){
      printf("%s of table %s", str.c_str(), primary.c_str());
    }
    else{
      printf("%s", str.c_str());
    }
  }
};

class ValueNode: public Node{
  public:
  void print(){
    switch(datatype){
      case INTEGER:
        printf("%d", number);
        break;
      case STRING:
        printf("%s", str.c_str());
        break;
      default:
        break;
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
      case SHOW_DATABASE_ALL:
        printf("SHOW DATABASES\n");
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
        
      case INSERT:
        printf("Insert into table \"%s\"\nRecords shown as follow:\n", str.c_str());
        printf("BEGIN\n");
        for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
          (*i)->print();
        }
        printf("END\n");
        break;
      case DELETE:
        printf("Delete from table \"%s\"\n", str.c_str());
        printf("Where ");
        for(std::vector<Node *>::reverse_iterator i = subtree.rbegin(); i != subtree.rend(); ++i){
          if(i != subtree.rbegin())
            printf("AND ");
          (*i)->print();
        }
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
    IDENTIFIER = 262,
    DATABASE = 263,
    DROP = 264,
    SHOW = 265,
    USE = 266,
    DATABASES = 267,
    SELECT = 268,
    INSERT = 269,
    INTO = 270,
    DELETE = 271,
    FROM = 272,
    WHERE = 273,
    VALUES = 274,
    NUMBER = 275,
    CONSTSTR = 276,
    AND = 277,
    IS = 278,
    NOT = 279,
    NULLSIGN = 280,
    EQU = 281,
    NEQ = 282,
    LEQ = 283,
    GEQ = 284,
    LES = 285,
    GTR = 286,
    INT_INPUT = 287,
    VARCHAR_INPUT = 288,
    FLOAT_INPUT = 289,
    STRING_INPUT = 290
  };
#endif

#endif

