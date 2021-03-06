/* DJ PARSER */
/** Created by atuladhar on 10/7/16.
  * Pledge: I pledge my Honor that I have not cheated, and will not cheat, on this assignment
  * Name: Anwesh Tuladhar
  */

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

main : MAIN body
     ;

body : LBRACE varDeclList exprList RBRACE

classDeclList : classDeclList classDecl
              |
              ;

classDecl : CLASS ID EXTENDS super LBRACE varDeclList RBRACE
          | CLASS ID EXTENDS super LBRACE varDeclList methodDeclList RBRACE
          ;

super : ID
      | OBJECT
      ;

varDeclList : varDeclList varDecl
            |
            ;

varDecl : typeDecl ID SEMICOLON
        ;

methodDeclList : methodDeclList methodDecl
               | methodDecl
               ;

methodDecl : typeDecl ID LPAREN typeDecl ID RPAREN body
           ;

typeDecl : NATTYPE
         | OBJECT
         | ID
         ;

exprList : exprList expr SEMICOLON
         | expr SEMICOLON
         ;

expr : arithmeticExpr
     | cmpExpr
     | NOT expr
     | assignExpr
     | constructorExpr
     | ifElseExpr
     | forExpr
     | printNatExpr
     | readNatExpr
     | factor
     | NUL
     | THIS
     | OBJECT
     ;

arithmeticExpr : expr PLUS expr
               | expr MINUS expr
               | expr TIMES expr
               ;

cmpExpr : expr EQUALITY expr
        | expr GREATER expr
        | expr OR expr
        ;

assignExpr : lhsExpr ASSIGN expr
           ;

lhsExpr : ID
        | expr DOT ID
        ;

constructorExpr : NEW ID LPAREN RPAREN
                | NEW OBJECT LPAREN RPAREN
                ;

ifElseExpr : IF LPAREN expr RPAREN LBRACE exprList RBRACE ELSE LBRACE exprList RBRACE
           ;

forExpr : FOR LPAREN expr SEMICOLON expr SEMICOLON expr RPAREN LBRACE exprList RBRACE
        ;

printNatExpr : PRINTNAT LPAREN expr RPAREN
             ;

readNatExpr : READNAT LPAREN RPAREN
            ;

factor : NATLITERAL
       | lhsExpr
       | dotMethodCallExpr
       | LPAREN expr RPAREN
       ;

dotMethodCallExpr : expr DOT methodCallExpr
                  | methodCallExpr
                  ;

methodCallExpr : ID LPAREN expr RPAREN
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
