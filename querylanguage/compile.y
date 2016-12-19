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
%token CREATE TABLE PRIMARY KEY IDENTIFIER DATABASE DROP SHOW USE 
%token DATABASES SELECT INSERT INTO DELETE FROM WHERE VALUES
%token NUMBER CONSTSTR 
%token AND IS NOT NULLSIGN
%token EQU NEQ LEQ GEQ LES GTR
%token INT_INPUT VARCHAR_INPUT FLOAT_INPUT STRING_INPUT
%%

ALL: StmtList
{
  $$ = $1;
  $$->print();
  /*
  $$->visit();
  */
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
  $$->stmttype = Node::INSERT;
  $$->str = $3->str;
  $$->subtree.assign($5->subtree.begin(), $5->subtree.end());
} | DELETE FROM IDENTIFIER WHERE WhereClauseList ';'
{
  $$ = new StmtNode();
  $$->stmttype = Node::DELETE;
  $$->str = $3->str;
  $$->subtree.assign($5->subtree.begin(), $5->subtree.end());
};

Rows: '(' Row ')' ',' Rows
{
  $$->subtree.assign($5->subtree.begin(), $5->subtree.end());
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
  $$->subtree.assign($3->subtree.begin(), $3->subtree.end());
  $$->subtree.push_back($1);
}

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
}

WhereClauseList: WhereClause
{

} | WhereClause AND WhereClauseList
{

}

WhereClause: ColumnAccess Op Value
{

} | ColumnAccess Op ColumnAccess
{

} | ColumnAccess IS NULLSIGN
{

} | ColumnAccess IS NOT NULLSIGN
{

}

ColumnAccess: IDENTIFIER
{
  $$ = new Node();
  $$->str = $1->str;
  $$->flag = $$->flag & '\xFD';
  
} | IDENTIFIER '.' IDENTIFIER
{
  $$ = new Node();
  $$->primary = $1->str;
  $$->str = $3->str;
  $$->flag = $$->flag | '\x02';
}

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
