#include <cstdio>
#include "userdef.h"
#include "../systemmanagement/SM_Manager.h"
#include "./QL_Manager.h"

extern int yyparse();
extern void yyset_in (FILE *  _in_str );

FileManager *fm;
BufPageManager *bpm;
RM_Manager *rm;
IX_Manager *ix;
SM_Manager *sm;
QL_Manager *qm;

int main(int argc, char* argv[]){
  // const int MAXBUF = 1024;
  // char buf[MAXBUF];
  // FILE *fin;
  // MyBitMap::initConst();
  // fm = new FileManager();
  // bpm = new BufPageManager(fm);
  // rm = new RM_Manager(fm, bpm);
  // ix = new IX_Manager(fm, bpm);
  // sm = new SM_Manager(ix, rm);
  // qm = new QL_Manager(sm, ix, rm);
  // if(argc < 2){
  //   fin = stdin;
  //   FILE *fout;
  //   int flag = 1;
  //   while(flag){
  //     fout = fopen("temp.tmp", "w");
  //     while(gets(buf) != NULL){
  //       if(strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0){
  //         flag = 0;
  //         break;
  //       }
  //       else{
  //         fprintf(fout, "%s\n", buf);
  //       }
  //     }
  //     fclose(fout);
  //     system("cat temp.tmp");
  //     fin = fopen("temp.tmp", "r");
  //     yyset_in(fin);
  //     yyparse();
  //     fclose(fin);
  //   }
  // }
  // else{
  //   fin = fopen(argv[1], "r");
  //   yyset_in(fin);
  //   yyparse();
  //   fclose(fin);
  // }
  FILE *fin;
  if(argc < 2)
    fin = stdin;
  else
    fin = fopen(argv[1], "r");
  yyset_in(fin);
  MyBitMap::initConst();
  fm = new FileManager();
  bpm = new BufPageManager(fm);
  rm = new RM_Manager(fm, bpm);
  ix = new IX_Manager(fm, bpm);
  sm = new SM_Manager(ix, rm);
  qm = new QL_Manager(sm, ix, rm);
  yyparse();
  fclose(fin);
  delete fm;
  delete bpm;
  delete rm;
  delete ix;
  delete sm;
  return 0;
}