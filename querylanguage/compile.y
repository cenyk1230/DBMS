%{
#include <stdio.h>
#include "lex.yy.c"
#include "userdef.h"
#include "../utils/base.h"
#define YYSTYPE Node*
int yyparse(void);
void yyerror(char* s);
int yywrap();

%}
%token CREATE TABLE PRIMARY KEY IDENTIFIER DATABASE DROP SHOW USE ALTER
%token DATABASES SELECT INSERT INTO UPDATE DELETE FROM WHERE VALUES SET GROUP BY
%token CHECK IN FOREIGN REFER
%token NUMBER CONSTSTR 
%token AND IS NOT NULLSIGN
%token EQU NEQ LEQ GEQ LES GTR LIKE
%token AVG SUM MIN MAX
%token INT_INPUT VARCHAR_INPUT FLOAT_INPUT STRING_INPUT
%%

ALL: StmtList
{
  $$ = $1;
  //$$->print();
  
  $$->visit();
  
};

StmtList: Stmt StmtList
{
  $$ = $2;
  $$->subtree.push_back($1);
} | /*nothing*/ {
  $$ = new StmtListNode();
};

Stmt: CREATE DATABASE IDENTIFIER ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::CREATE_DATABASE;
  $$->str = $3->str;
} | SHOW DATABASES ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SHOW_DATABASE_ALL;
}| USE IDENTIFIER ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::USE_DATABASE;
  $$->str = $2->str;
} | DROP DATABASE IDENTIFIER ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::DROP_DATABASE;
  $$->str = $3->str;
} | SHOW DATABASE IDENTIFIER ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SHOW_DATABASE;
  $$->str = $3->str;
} | CREATE TABLE IDENTIFIER '(' ColumnList ')' ';'
{
  $$ = new StmtNode();
  $$->str = $3->str;
  $$->stmttype = Node::CREATE_TABLE;
  $$->subtree.assign($5->subtree.begin(), $5->subtree.end());
  $$->primary = $5->primary;
}  | DROP TABLE IDENTIFIER ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::DROP_TABLE;
  $$->str = $3->str;
} | SHOW TABLE IDENTIFIER ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SHOW_TABLE;
  $$->str = $3->str;
} | INSERT INTO IDENTIFIER VALUES Rows ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::INSERT_DATA;
  $$->str = $3->str;
  $$->subtree.assign($5->subtree.begin(), $5->subtree.end());
} | DELETE FROM IDENTIFIER WHERE WhereClauseList ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::DELETE_DATA;
  $$->str = $3->str;
  $$->subtree.assign($5->subtree.begin(), $5->subtree.end());
} | DELETE FROM IDENTIFIER ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::DELETE_DATA;
  $$->str = $3->str;
} | UPDATE IDENTIFIER SET ColumnAccess EQU Value WHERE WhereClauseList ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::UPDATE_DATA;
  $$->str = $2->str;
  $$->subtree.push_back($4);
  $$->subtree.push_back($6);
  $$->subtree.push_back($8);
} | UPDATE IDENTIFIER SET ColumnAccess EQU Value ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::UPDATE_DATA;
  $$->str = $2->str;
  $$->subtree.push_back($4);
  $$->subtree.push_back($6);
} | SELECT GroupList FROM IDENTIFIERLIST WHERE WhereClauseList ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SELECT_DATA;
  $$->subtree.push_back($2);
  $$->subtree.push_back($4);
  $$->subtree.push_back($6);
} | SELECT GroupList FROM IDENTIFIERLIST ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SELECT_DATA;
  $$->subtree.push_back($2);
  $$->subtree.push_back($4);
} | SELECT FROM IDENTIFIERLIST WHERE WhereClauseList ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SELECT_DATA_ALL;
  $$->subtree.push_back($3);
  $$->subtree.push_back($5);
} | SELECT '*' FROM IDENTIFIERLIST WHERE WhereClauseList ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SELECT_DATA_ALL;
  $$->subtree.push_back($4);
  $$->subtree.push_back($6);
} | SELECT FROM IDENTIFIERLIST ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SELECT_DATA_ALL;
  $$->subtree.push_back($3);
} | SELECT '*' FROM IDENTIFIERLIST ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SELECT_DATA_ALL;
  $$->subtree.push_back($4);
} | SELECT GroupList FROM IDENTIFIERLIST WHERE WhereClauseList GROUP BY IDENTIFIER ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SELECT_GROUP;
  $$->str = $9->str;
  $$->subtree.push_back($2);
  $$->subtree.push_back($4);
  $$->subtree.push_back($6);
} | SELECT GroupList FROM IDENTIFIERLIST GROUP BY IDENTIFIER ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::SELECT_GROUP;
  $$->str = $7->str;
  $$->subtree.push_back($2);
  $$->subtree.push_back($4);
} | ALTER TABLE IDENTIFIER CHECK KEY IDENTIFIER IN '(' Row ')' ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::CONSTRIANT_CHECK;
  $$->primary = $3->str;
  $$->str = $6->str;
  $$->subtree.assign($9->subtree.begin(), $9->subtree.end());
} | ALTER TABLE IDENTIFIER FOREIGN KEY IDENTIFIER REFER ColumnAccess ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::CONSTRIANT_FOREIGN;
  $$->primary = $3->str;
  $$->str = $6->str;
  $$->subtree.push_back($8);
};

Rows: '(' Row ')' ',' Rows
{
  $$ = $5;
  $$->subtree.push_back($2);
} | '(' Row ')' {
  $$ = new RowsNode();
  $$->subtree.push_back($2);
}

Row: Value
{
  $$ = new RowNode();
  $$->subtree.push_back($1);
} | Value ',' Row
{
  $$ = $3;
  $$->subtree.push_back($1);
} | NULLSIGN ',' Row
{
  $$ = $3;
  ValueNode *nullvalue = new ValueNode();
  nullvalue->datatype = Node::NULLDATA;
  $$->subtree.push_back(nullvalue);
} | NULLSIGN
{
  $$ = new RowNode();
  ValueNode *nullvalue = new ValueNode();
  nullvalue->datatype = Node::NULLDATA;
  $$->subtree.push_back(nullvalue);
};

Value: NUMBER
{
  $$ = new ValueNode();
  $$->number = $1->number;
  $$->datatype = Node::INTEGER;
} | CONSTSTR
{
  $$ = new ValueNode();
  $$->str = $1->str;
  $$->datatype = Node::STRING;
};

WhereClauseList: WhereClause
{
  $$ = new WhereListNode();
  $$->subtree.push_back($1);
} | WhereClause AND WhereClauseList
{
  $$ = $3;
  $$->subtree.push_back($1);
};

WhereClause: ColumnAccess Op Value
{
  $$ = new WhereNode();
  $$->datatype = $2->datatype;
  $$->subtree.push_back($1);
  $$->subtree.push_back($3);
  $$->flag = Node::setFlag($$->flag, 2, false);
  $$->flag = Node::setFlag($$->flag, 3, false);
} | ColumnAccess Op ColumnAccess
{
  $$ = new WhereNode();
  $$->datatype = $2->datatype;
  $$->subtree.push_back($1);
  $$->subtree.push_back($3);
  $$->flag = Node::setFlag($$->flag, 2, true);
  $$->flag = Node::setFlag($$->flag, 3, false);
} | ColumnAccess IS NULLSIGN
{
  $$ = new WhereNode();
  $$->subtree.push_back($1);
  $$->flag = Node::setFlag($$->flag, 3, true);
  $$->flag = Node::Node::setFlag($$->flag, 0, false);
} | ColumnAccess IS NOT NULLSIGN
{
  $$ = new WhereNode();
  $$->subtree.push_back($1);
  $$->flag = Node::setFlag($$->flag, 3, true);
  $$->flag = Node::setFlag($$->flag, 0, true);
};

IDENTIFIERLIST: IDENTIFIER
{
  $$ = new Node();
  $$->subtree.push_back($1);
} | IDENTIFIER ',' IDENTIFIERLIST
{
  $$ = $3;
  $$->subtree.push_back($1);
};

GroupList: GroupItem
{
  $$ = new Node();
  $$->subtree.push_back($1);
} | GroupItem ',' GroupList
{
  $$ = $3;
  $$->subtree.push_back($1);
};

GroupItem: ColumnAccess
{
  $$ = new Node();
  $$->datatype = Node::FUNC_NO;
  $$->str = $1->str;
  $$->primary = $1->primary;
  $$->flag = $1->flag;
} | FUNC '(' ColumnAccess ')'
{
  $$ = new Node();
  $$->datatype = $1->datatype;
  $$->str = $3->str;
  $$->primary = $3->primary;
  $$->flag = $3->flag;
};


ColumnAccess: IDENTIFIER
{
  $$ = new AccessNode();
  $$->str = $1->str;
  $$->flag = Node::setFlag($$->flag, 1, false);
  
} | IDENTIFIER '.' IDENTIFIER
{
  $$ = new AccessNode();
  $$->primary = $1->str;
  $$->str = $3->str;
  $$->flag = Node::setFlag($$->flag, 1, true);
};

ColumnList: Column ',' ColumnList // Use stack
{
  $$->subtree.assign($3->subtree.begin(), $3->subtree.end());
  $$->subtree.push_back($1);
  $$->primary = $3->primary;
} | Column{
  $$ = new ColumnListNode();
  $$->subtree.push_back($1);
} | PrimaryColumn{
  $$ = new ColumnListNode();
  $$->primary = $1->str;
};

Column: IDENTIFIER Type '(' NUMBER ')' {
  $$ = new ColumnNode();
  $$->datatype = $2->datatype;
  $$->str = $1->str;
  $$->number = $4->number;
} | IDENTIFIER Type '(' NUMBER ')' NOT NULLSIGN {
  $$ = new ColumnNode();
  $$->datatype = $2->datatype;
  $$->str = $1->str;
  $$->number = $4->number;
  $$->flag |= 1;
};

PrimaryColumn: PRIMARY KEY '(' IDENTIFIER ')' {
  $$ = new Node();
  $$->str = $4->str;
};

Type: INT_INPUT {
  $$ = new Node();
  $$->datatype = Node::INTEGER;
} | VARCHAR_INPUT {
  $$ = new Node();
  $$->datatype = Node::VARCHAR;
} | FLOAT_INPUT {
  $$ = new Node();
  $$->datatype = Node::FLOAT;
} | STRING_INPUT {
  $$ = new Node();
  $$->datatype = Node::STRING;
};

Op: EQU {
  $$ = new Node();
  $$->datatype = Node::OP_EQU;
} | NEQ {
  $$ = new Node();
  $$->datatype = Node::OP_NEQ;
} | LEQ {
  $$ = new Node();
  $$->datatype = Node::OP_LEQ;
} | GEQ {
  $$ = new Node();
  $$->datatype = Node::OP_GEQ;
} | LES {
  $$ = new Node();
  $$->datatype = Node::OP_LES;
} | GTR {
  $$ = new Node();
  $$->datatype = Node::OP_GTR;
} | LIKE {
  $$ = new Node();
  $$->datatype = Node::OP_LIKE; 
};

FUNC: AVG
{
  $$ = new Node();
  $$->datatype = Node::FUNC_AVG;
} | SUM
{
  $$ = new Node();
  $$->datatype = Node::FUNC_SUM;
} | MIN
{
  $$ = new Node();
  $$->datatype = Node::FUNC_MIN;
} | MAX
{
  $$ = new Node();
  $$->datatype = Node::FUNC_MAX;
}


%%
/*
int main()
{
    return yyparse();
}
*/
void yyerror(char* s)
{
    fprintf(stderr,"%s",s);
}
int yywrap()
{
    return 1;
}
