%{
#include <stdio.h>
#include "lex.yy.c"
#include "userdef.h"
#include "../utils/base.h"
#define YYSTYPE Node*
int yyparse(void);
void yyerror(char* s);
int yywrap();
extern Node* setRoot(Node *p);

%}
%token CREATE TABLE PRIMARY KEY NOT NULLSIGN INTEGER VARCHAR NUMBER IDENTIFIER DATABASE DROP SHOW USE FLOAT STRING
%%

ALL: StmtList
{
  $$ = $1;
  setRoot($$);
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
} | Column{
  $$ = new ColumnListNode();
  $$->subtree.push_back($1);
} | PrimaryColumn{
  $$ = new ColumnListNode();
  $$->subtree.push_back($1);
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

Type: INTEGER {
  $$ = new Node();
  $$->datatype = Node::INTEGER;
} | VARCHAR {
  $$ = new Node();
  $$->datatype = Node::VARCHAR;
} | FLOAT {
  $$ = new Node();
  $$->datatype = Node::FLOAT;
} | STRING {
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
