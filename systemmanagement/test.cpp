#include <cstdio>
#include "userdef.h"

extern int yyparse();
extern void yyset_in (FILE *  _in_str );


int main(int argc, char* argv[]){
  FILE *fin;
  if(argc < 2)
    fin = stdin;
  else
    fin = fopen(argv[1], "r");
  yyset_in(fin);
  yyparse();
  fclose(fin);
  return 0;
}
