/* DISM PARSER */

%code provides {
  #include <stdio.h>
  #include "lex.yy.c"

  /* Function for printing generic syntax-error messages */
  void yyerror(const char *str) {
    printf("Syntax error on line %d at token %s\n",yylineno,yytext);
    printf("Exiting simulator.\n");
    exit(-1);
  }
}

%token ADD SUB MUL MOV LOD STR JMP BEQ BLT RDN PTN HLT INT LABEL COLON
%token ENDOFFILE

%start pgm

%%

pgm : instlist ENDOFFILE 
      { return 0; }
    ;

instlist : instlist label COLON inst 
         | instlist inst 
         | 
         ;

inst : ADD intt intt intt 
     | SUB intt intt intt 
     | MUL intt intt intt 
     | MOV intt intt 
     | LOD intt intt intt 
     | STR intt intt intt 
     | JMP intt intt 
     | BEQ intt intt intt 
     | BLT intt intt intt 
     | RDN intt 
     | PTN intt 
     | HLT intt 
     ;

label : LABEL 
      ;

intt : INT 
     | label 
     ;

%%

int main(int argc, char **argv) {
  if(argc!=2) {
    printf("Usage: dism-parse filename\n");
    exit(-1);
  }
  yyin = fopen(argv[1],"r");
  if(yyin==NULL) {
    printf("ERROR: could not open file %s\n",argv[1]);
    exit(-1);
  }
  /* parse the program */
  return yyparse();
}
