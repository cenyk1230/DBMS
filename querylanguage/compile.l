%{
#include<string.h>  
#include "userdef.h"

extern Node* yylval;  
%}  
create (CREATE)|(create)
use (USE)|(use)
drop (DROP)|(drop)
show (SHOW)|(show)
table (TABLE)|(table)
database (DATABASE)|(database)
databases (DATABASES)|(databases)
primary (PRIMARY)|(primary)
key (KEY)|(key)
is (IS)|(is)
not (NOT)|(not)
null (NULL)|(null)
and (AND)|(and)

insert "INSERT"
into "INTO"
values "VALUES"

delete "DELETE"
update "UPDATE"
select "SELECT"
from "FROM"
where "WHERE"
set (SET)|(set)
group (GROUP)|(group)
by  (BY)|(by)
alter (ALTER)|(alter)

check (CHECK)|(check)
in (IN)|(in)
foreign (FOREIGN)|(foreign)
refer (REFERENCES)|(references)

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
like (LIKE)|(like)

avg (AVG)|(avg)
sum (SUM)|(sum)
min (MIN)|(min)
max (MAX)|(max)

lp "("
rp ")"
star "*"
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
{update}  {return UPDATE;}
{select}  {return SELECT;}
{from}    {return FROM;}
{where}   {return WHERE;}
{set}     {return SET;}
{group}   {return GROUP;}
{by}      {return BY;}
{alter}   {return ALTER;}

{check}   {return CHECK;}
{in}   {return IN;}
{foreign}   {return FOREIGN;}
{refer}   {return REFER;}

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
{like}    {return LIKE;}

{avg}     {return AVG;}
{sum}     {return SUM;}
{min}     {return MIN;}
{max}     {return MAX;}

{lp}      {return '(';}
{rp}      {return ')';}
{star}    {return '*';}
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

. {printf("Error\n");exit(1);}    
%% 
