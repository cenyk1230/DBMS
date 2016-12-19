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
databases "DATABASES"
primary "PRIMARY"
key "KEY"
is "IS"
not "NOT"
null "NULL"
and "AND"

insert "INSERT"
into "INTO"
values "VALUES"

delete "DELETE"
select "SELECT"
from "FROM"
where "WHERE"


comma ","
semicolon ";"
dot   "."

int "int"
varchar "varchar"
float "float"
string "string"

equ "="
neq (<>)|(!=)
leq "<="
geq ">="
les "<"
gtr ">"

lp "("
rp ")"
number ([0-9])+
identifier [A-Za-z]([A-Za-z0-9_])*
conststr '([^'])*'

whitespace ([ \n\t])+

%%  
{create}  {return CREATE;}
{use}     {return USE;}
{drop}    {return DROP;}
{show}    {return SHOW;}
{database}  {return DATABASE;}
{databases} {return DATABASES;}
{table}   {return TABLE;}
{primary} {return PRIMARY;}
{key}     {return KEY;}
{is}      {return IS;}
{not}     {return NOT;}
{null}    {return NULLSIGN;}
{and}     {return AND;}

{insert}  {return INSERT;}
{into}    {return INTO;}
{values}  {return VALUES;}
{delete}  {return DELETE;}
{select}  {return SELECT;}
{from}    {return FROM;}
{where}   {return WHERE;}

{comma}   {return ',';}
{semicolon} {return ';';}
{dot}     {return '.';}
{int}       {return INT_INPUT;}
{varchar} {return VARCHAR_INPUT;}
{float}   {return FLOAT_INPUT;}
{string}  {return STRING_INPUT;}

{equ}     {return EQU;}
{neq}     {return NEQ;}
{leq}     {return LEQ;}
{geq}     {return GEQ;}
{les}     {return LES;}
{gtr}     {return GTR;}
{lp}      {return '(';}
{rp}      {return ')';}
{number}  {yylval = new Node(); sscanf(yytext, "%d", &yylval->number); return NUMBER;}
{conststr}  {
  yylval = new Node(); 
  for(int i = 0; yytext[i]; ++i){
    if(yytext[i] != '\'')
      yylval->str.push_back(yytext[i]);
  } 
  return CONSTSTR;
}
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
