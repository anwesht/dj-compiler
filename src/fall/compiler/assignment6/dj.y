/* DJ PARSER */
/**
 * Created by atuladhar on 10/17/16.
 * Pledge: I pledge my Honor that I have not cheated, and will not cheat, on this assignment
 * Name: Anwesh Tuladhar
 */

%code provides {
  #include <stdio.h>
  #include "lex.yy.c"
  #include "ast.h"

  /* Define the Type for AST node (i.e. type of the Semantic Values.) */
  #define YYSTYPE ASTree*

  /* Define the root node of AST */
  ASTree *pgmAST;

  /* Function for printing generic syntax-error messages */
  void yyerror(const char *str) {
    printf("Syntax error on line %d at token %s\n", yylineno, yytext);
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
      {
        pgmAST = $1;
        return 0;
      }
    ;

pgmList : classDeclList MAIN LBRACE varDeclList exprList RBRACE
          {
            $$ = newAST(PROGRAM, $1, 0, NULL, yylineno);
            appendToChildrenList ($$, $4);
            appendToChildrenList ($$, $5);
          }
        ;

classDeclList : classDeclList classDecl
                {
                  appendToChildrenList ($$, $2);
                }
              |
                {
                  $$ = newAST(CLASS_DECL_LIST, NULL, 0, NULL, yylineno);
                }
              ;

classDecl : CLASS id EXTENDS super LBRACE varDeclList RBRACE
            {
              $$ = newAST(CLASS_DECL, $2, 0, NULL, yylineno);
              appendToChildrenList ($$, $4);
              appendToChildrenList ($$, $6);
              appendToChildrenList ($$, newAST(METHOD_DECL_LIST, NULL, 0, NULL, yylineno));
            }
          | CLASS id EXTENDS super LBRACE varDeclList methodDeclList RBRACE
            {
              $$ = newAST(CLASS_DECL, $2, 0, NULL, yylineno);
              appendToChildrenList ($$, $4);
              appendToChildrenList ($$, $6);
              appendToChildrenList ($$, $7);
            }
          ;

id : ID
     {
       //printf("In id: %s : %u\n", yytext, yylineno);
       $$ = newAST(AST_ID, NULL, 0, yytext, yylineno);
     }
   ;

dotIdExpr : expr DOT id
        {
          $$ = newAST(DOT_ID_EXPR, $1, 0, yytext, yylineno);
          appendToChildrenList ($$, $3);
        }
      ;

object : OBJECT
         {
           $$ = newAST(OBJ_TYPE, NULL, 0, NULL, yylineno);
         }
       ;

natType : NATTYPE
          {
            $$ = newAST(NAT_TYPE, NULL, 0, NULL, yylineno);
          }
        ;

super : id
        {
          $$ = $1;
        }
      | object
        {
          $$ = $1;
        }
      ;

varDeclList : varDeclList varDecl
              {
                appendToChildrenList ($1, $2);
              }
             |
              {
                $$ = newAST(VAR_DECL_LIST, NULL, 0, NULL, yylineno);
              }
             ;

varDecl : typeDecl id SEMICOLON
          {
            $$ = newAST(VAR_DECL, NULL, 0, NULL, yylineno);
            appendToChildrenList ($$, $1);
            appendToChildrenList ($$, $2);
          }
        ;

methodDeclList : methodDeclList methodDecl
                 {
                   appendToChildrenList ($1, $2);
                 }
               | methodDecl
                 {
                   $$ = newAST(METHOD_DECL_LIST, NULL, 0, NULL, yylineno);
                   appendToChildrenList ($$, $1);
                 }
               ;

methodDecl : typeDecl id LPAREN typeDecl id RPAREN LBRACE varDeclList exprList RBRACE
             {
               $$ = newAST(METHOD_DECL, $1, 0, NULL, yylineno);
               appendToChildrenList ($$, $2);
               appendToChildrenList ($$, $4);
               appendToChildrenList ($$, $5);
               appendToChildrenList ($$, $8);
               appendToChildrenList ($$, $9);
             }
           ;

typeDecl : natType
           {
             $$ = $1;
           }
         | object
           {
             $$ = $1;
           }
         | id
           {
             $$ = $1;
           }
         ;

exprList : exprList expr SEMICOLON
           {
             appendToChildrenList ($1, $2);
           }
         | expr SEMICOLON
           {
             $$ = newAST(EXPR_LIST, $1, 0, NULL, yylineno);
           }
         ;

expr : arithmeticExpr  { $$ = $1; }
     | cmpExpr         { $$ = $1; }
     | NOT expr
       {
         $$ = newAST(NOT_EXPR, $2, 0, NULL, yylineno);
       }
     | assignExpr      { $$ = $1; }
     | dotAssignExpr   { $$ = $1; }
     | constructorExpr { $$ = $1; }
     | ifElseExpr      { $$ = $1; }
     | forExpr         { $$ = $1; }
     | printNatExpr    { $$ = $1; }
     | readNatExpr     { $$ = $1; }
     | factor          { $$ = $1; }
     | NUL
       {
         $$ = newAST(NULL_EXPR, NULL, 0, NULL, yylineno);
       }
     | THIS
       {
         $$ = newAST(THIS_EXPR, NULL, 0, NULL, yylineno);
       }
     | object { $$ = $1; }
     ;

arithmeticExpr : expr PLUS expr
                 {
                   $$ = newAST(PLUS_EXPR, $1, 0, NULL, yylineno);
                   appendToChildrenList ($$, $3);
                 }
               | expr MINUS expr
                 {
                   $$ = newAST(MINUS_EXPR, $1, 0, NULL, yylineno);
                   appendToChildrenList ($$, $3);
                 }
               | expr TIMES expr
                 {
                   $$ = newAST(TIMES_EXPR, $1, 0, NULL, yylineno);
                   appendToChildrenList ($$, $3);
                 }
               ;

cmpExpr : expr EQUALITY expr
          {
            $$ = newAST(EQUALITY_EXPR, $1, 0, NULL, yylineno);
            appendToChildrenList ($$, $3);
          }
        | expr GREATER expr
          {
            $$ = newAST(GREATER_THAN_EXPR, $1, 0, NULL, yylineno);
            appendToChildrenList ($$, $3);
          }
        | expr OR expr
          {
            $$ = newAST(OR_EXPR, $1, 0, NULL, yylineno);
            appendToChildrenList ($$, $3);
          }
        ;

assignExpr : id ASSIGN expr
             {
               $$ = newAST(ASSIGN_EXPR, $1, 0, NULL, yylineno);
               appendToChildrenList ($$, $3);
             }
           ;

dotAssignExpr : expr DOT id ASSIGN expr
             {
               $$ = newAST(DOT_ASSIGN_EXPR, $1, 0, NULL, yylineno);
               appendToChildrenList ($$, $3);
               appendToChildrenList ($$, $5);
             }
           ;

constructorExpr : NEW id LPAREN RPAREN
                  {
                    $$ = newAST(NEW_EXPR, $2, 0, NULL, yylineno);
                  }
                | NEW object LPAREN RPAREN
                  {
                    $$ = newAST(NEW_EXPR, $2, 0, NULL, yylineno);
                  }
                ;

ifElseExpr : IF LPAREN expr RPAREN LBRACE exprList RBRACE ELSE LBRACE exprList RBRACE
             {
               $$ = newAST(IF_THEN_ELSE_EXPR, $3, 0, NULL, yylineno);
               appendToChildrenList ($$, $6);
               appendToChildrenList ($$, $10);
             }
           ;

forExpr : FOR LPAREN expr SEMICOLON expr SEMICOLON expr RPAREN LBRACE exprList RBRACE
          {
            $$ = newAST(FOR_EXPR, $3, 0, NULL, yylineno);
            appendToChildrenList ($$, $5);
            appendToChildrenList ($$, $7);
            appendToChildrenList ($$, $10);
          }
        ;

printNatExpr : PRINTNAT LPAREN expr RPAREN
               {
                 $$ = newAST(PRINT_EXPR, $3, 0, NULL, yylineno);
               }
             ;

readNatExpr : READNAT LPAREN RPAREN
              {
                $$ = newAST(READ_EXPR, NULL, 0, NULL, yylineno);
              }
            ;

factor : NATLITERAL
         {
           $$ = newAST(NAT_LITERAL_EXPR, NULL, atoi(yytext), NULL, yylineno);
         }
       | idExpr { $$ = $1; }
       | dotIdExpr { $$ = $1; }
       | methodCallExpr { $$ = $1; }
       | dotMethodCallExpr { $$ = $1; }
       | LPAREN expr RPAREN { $$ = $2; }
       ;

dotMethodCallExpr : expr DOT id LPAREN expr RPAREN
                    {
                      $$ = newAST(DOT_METHOD_CALL_EXPR, $1, 0, NULL, yylineno);
                      appendToChildrenList ($$, $3);
                      appendToChildrenList ($$, $5);
                    }
                  ;

methodCallExpr : id LPAREN expr RPAREN
                 {
                   $$ = newAST(METHOD_CALL_EXPR, $1, 0, NULL, yylineno);
                   appendToChildrenList ($$, $3);
                 }
               ;

idExpr : id
         {
           $$ = newAST(ID_EXPR, $1, 0, NULL, yylineno);
         }
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
  yyparse();
  printAST(pgmAST);
  return 0;
}
