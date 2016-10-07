/* DJ PARSER */

%code provides {
  #include "lex.yy.c"
 
  /* Function for printing generic syntax-error messages */
  void yyerror(const char *str) {
    printf("Syntax error on line %d at token %s\n",yylineno,yytext);
    printf("(This version of the compiler exits after finding the first ");
    printf("syntax error.)\n");
    exit(-1);
  }
}

%token CLASS ID EXTENDS MAIN NATTYPE OBJECT
%token NATLITERAL PRINTNAT READNAT IF ELSE FOR
%token PLUS MINUS TIMES EQUALITY GREATER OR NOT
%token ASSIGN NUL NEW THIS DOT SEMICOLON
%token LBRACE RBRACE LPAREN RPAREN ENDOFFILE

%start pgm

%right ASSIGN

%%

pgm : pgmList ENDOFFILE
      { return 0; }
    ;

pgmList : classDeclList varDeclList exprList
        ;

classDeclList : classDeclList classDecl
              |
              ;

varDeclList : varDeclList varDecl
            |
            ;

exprList : exprList expr
         |
         ;

classDecl : CLASS ID EXTENDS super LBRACE RBRACE
          ;

super : ID
      | OBJECT
      ;

varDecl : NATTYPE ID SEMICOLON
        ;

expr : NATLITERAL PLUS NATLITERAL
     ;


%%

int main(int argc, char **argv) {
  if(argc!=2) {
    printf("Usage: dj-parse filename\n");
    exit(-1);
  }
  yyin = fopen(argv[1],"r");
  if(yyin==NULL) {
    printf("ERROR: could not open file %s\n",argv[1]);
    exit(-1);
  }
  /* parse the input program */
  return yyparse();
}
