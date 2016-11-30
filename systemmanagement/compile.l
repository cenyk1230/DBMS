%{
#include<string.h>  
#include "userdef.h"

extern Node* yylval;  
%}  
create "CREATE"
use "USE"
drop "DROP"
show "SHOW"
table "TABLE"
database "DATABASE"
primary "PRIMARY"
key "KEY"
not "NOT"
null "NULL"

comma ","
semicolon ";"

int "int"
varchar "varchar"
float "float"
string "string"

lp "("
rp ")"
number ([0-9])+
identifier [A-Za-z]([A-Za-z0-9_])*

whitespace ([ \n\t])+

%%  
{create}  {return CREATE;}
{use}     {return USE;}
{drop}    {return DROP;}
{show}    {return SHOW;}
{database}  {return DATABASE;}
{table}   {return TABLE;}
{primary} {return PRIMARY;}
{key}     {return KEY;}
{not}     {return NOT;}
{null}    {return NULLSIGN;}
{comma}   {return ',';}
{semicolon} {return ';';}
{int}       {return INTEGER;}
{varchar} {return VARCHAR;}
{float}   {return FLOAT;}
{string}  {return STRING;}
{lp}      {return '(';}
{rp}      {return ')';}
{number}  {yylval = new Node(); sscanf(yytext, "%d", &yylval->number); return NUMBER;}
{identifier}  {
  yylval = new Node();
  for(int i = 0; yytext[i]; ++i){
    yylval->str.push_back(yytext[i]);
  }
  return IDENTIFIER;
}
{whitespace}    ;

. {printf("Error");exit(1);}    
%% 
