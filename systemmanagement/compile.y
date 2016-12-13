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
%token CREATE TABLE PRIMARY KEY NOT NULLSIGN INT_INPUT VARCHAR_INPUT NUMBER IDENTIFIER DATABASE DROP SHOW USE FLOAT_INPUT STRING_INPUT
%%

ALL: StmtList
{
  $$ = $1;
  $$->visit();
  $$->print();
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
} | USE IDENTIFIER ';'
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
