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
%left OR
%nonassoc EQUALITY GREATER
%left PLUS MINUS
%left TIMES
%right NOT
%left DOT


%%

pgm : pgmList ENDOFFILE
      { return 0; }
    ;

pgmList : classDeclList main
        ;

main : MAIN LBRACE varDeclList exprList RBRACE
     ;

classDeclList : classDeclList classDecl
              |
              ;

varDeclList : varDeclList varDecl
            |
            ;

exprList : exprList expr SEMICOLON
         | expr SEMICOLON
         ;

classDecl : CLASS ID EXTENDS super LBRACE RBRACE
          ;

super : ID
      | OBJECT
      ;

varDecl : typeDecl ID SEMICOLON
        ;

typeDecl : NATTYPE
         | OBJECT
         | ID
         ;

expr : cmpExpr
     | arithmeticExpr
     | assignExpr
     | printNatExpr
     | readNatExpr
     | factor
     | NUL
     | THIS
     | OBJECT
     | NOT expr
     ;

cmpExpr : expr EQUALITY expr
        | expr GREATER expr
        | expr OR expr
        ;

arithmeticExpr : expr PLUS expr
               | expr MINUS expr
               | expr TIMES expr
               ;

factor : NATLITERAL
       | ID
       | expr DOT ID
       | dotMethodCallExpr
       | LPAREN expr RPAREN
       ;

assignExpr : expr ASSIGN expr
           ;

methodCallExpr : ID LPAREN args RPAREN
               ;

args : expr;

dotMethodCallExpr : expr DOT methodCallExpr
                  | methodCallExpr
                  ;

printNatExpr : PRINTNAT LPAREN expr RPAREN
             ;

readNatExpr : READNAT LPAREN RPAREN
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

  /*
  expr : LPAREN expr RPAREN
       | expr plusminus term
       | term
       | assignExpr
       | printNatExpr
       ;

  term : term TIMES factor
       | factor
       ;

  plusminus : PLUS
            | MINUS
            ;

=====
expr : LPAREN expr RPAREN
     | dotIdExpr
     | expr op expr
     | assignExpr
     | methodCallExpr
     | dotMethodCallExpr
     | printNatExpr
     | factor
     ;

     ===

     expr : LPAREN expr RPAREN
          | dotIdExpr
          | expr op expr
          | assignExpr
          | methodCallExpr
          | dotMethodCallExpr
          | printNatExpr
          | factor
          ;
  */

  /*
  varDecl : NATTYPE ID
          ;

  expr : expr plusminus term
       | term
       | assignExpr
       | printNatExpr
       ;

  term : term TIMES factor
       | factor
       ;

  plusminus : PLUS
            | MINUS
            ;

  assignExpr : ID ASSIGN expr
             ;

  methodCallExpr : ID LPAREN expr RPAREN
                 ;

  dotIdExpr : expr DOT ID
            ;

  dotMethodCallExpr : expr DOT methodCallExpr
                    ;

  printNatExpr : PRINTNAT LPAREN expr RPAREN
               ;

  factor : NATLITERAL
         | ID
         | LPAREN expr RPAREN
         ;

  op : PLUS
     | MINUS
     | TIMES
     ;
  */

  /*
  pgm : pgmList ENDOFFILE
        { return 0; }
      ;

  pgmList : classDeclList main
          ;

  main : MAIN LBRACE varDeclList exprList RBRACE
       ;

  classDeclList : classDeclList classDecl
                |
                ;

  varDeclList : varDeclList varDecl SEMICOLON
              |
              ;

  exprList : exprList expr SEMICOLON
           |
           ;

  classDecl : CLASS ID EXTENDS super LBRACE RBRACE
            ;

  super : ID
        | OBJECT
        ;

  varDecl : NATTYPE ID
          ;

  expr : expr EQUALITY expr
       | expr GREATER expr
       | expr OR expr
       | expr plusminus term
       | term
       | dotIdExpr
       | assignExpr
       | methodCallExpr
       | dotMethodCallExpr
       | printNatExpr
       ;

  plusminus : PLUS
            | MINUS
            ;

  term : term TIMES factor
       | factor
       ;

  factor : NATLITERAL
         | ID
         | LPAREN expr RPAREN
         ;

  dotIdExpr : expr DOT ID
            ;

  assignExpr : ID ASSIGN expr
             ;

  methodCallExpr : ID LPAREN expr RPAREN
                 ;

  dotMethodCallExpr : expr DOT methodCallExpr
                    ;

  printNatExpr : PRINTNAT LPAREN expr RPAREN
               ;


  %%
  */
}
