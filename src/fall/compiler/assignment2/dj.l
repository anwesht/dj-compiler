/** Created by atuladhar on 9/13/16.
  * Pledge: I pledge my Honor that I have not cheated, and will not cheat, on this assignment
  * Name: Anwesh Tuladhar
  */

%option yylineno
%option noyywrap

%{
  #define DEBUG 1
  typedef YYTOKENTYPE Token;
  Token scanned(Token t);
%}

digit   [0-9]
alpha   [a-zA-Z]
wspace  [ \t\n\r]+
id      [a-zA-Z][a-zA-Z0-9]*

%%

"class"                     {return scanned(CLASS);}
"extends"                   {return scanned(EXTENDS);}
"main"                      {return scanned(MAIN);}
"nat"                       {return scanned(NATTYPE);}
"Object"                    {return scanned(OBJECT);}
"printNat"                  {return scanned(PRINTNAT);}
"readNat"                   {return scanned(READNAT);}
"if"                        {return scanned(IF);}
"else"                      {return scanned(ELSE);}
"for"                       {return scanned(FOR);}
"null"                      {return scanned(NUL);}
"new"                       {return scanned(NEW);}
"this"                      {return scanned(THIS);}
{id}                        {return scanned(ID);}
{digit}+                    {return scanned(NATLITERAL);}
"+"                         {return scanned(PLUS);}
"-"                         {return scanned(MINUS);}
"*"                         {return scanned(TIMES);}
"=="                        {return scanned(EQUALITY);}
">"                         {return scanned(GREATER);}
"||"                        {return scanned(OR);}
"!"                         {return scanned(NOT);}
"="                         {return scanned(ASSIGN);}
"."                         {return scanned(DOT);}
";"                         {return scanned(SEMICOLON);}
"{"                         {return scanned(LBRACE);}
"}"                         {return scanned(RBRACE);}
"("                         {return scanned(LPAREN);}
")"                         {return scanned(RPAREN);}
"//"                        { char c = input();
                                /* Consume input till next line or EOF */
                                while(c != '\n' && c != EOF) c = input();
                            }
{wspace}                    { /*skip whitespace*/ }
<<EOF>>                     {return scanned(ENDOFFILE);}
.                           { // Incase of error: continues tokenizing from the next character.
                              if(DEBUG) printf("\n");
                              printf("Lex error on line %d: Illegal character %s\n",
                                     yylineno,yytext);
                              /* To Consume input till a white space or EOF, uncomment following code.*/
                              /*
                              char c = input();
                              while(c != '\n' && c != '\t' && c != ' ' && c != EOF){
                                c=input();
                              }
                              */
                            }

%%

/* process and return scanned token t */
Token scanned(Token t) {
  if(DEBUG==0) return t;
  switch(t) {
    case CLASS:         printf("CLASS "); return t;
    case ID:            printf("ID(%s) ", yytext); return t;
    case EXTENDS:       printf("EXTENDS "); return t;
    case MAIN:          printf("MAIN "); return t;
    case NATTYPE:       printf("NATTYPE "); return t;
    case OBJECT:        printf("OBJECT "); return t;
    case NATLITERAL:    printf("NATLITERAL(%s) ", yytext); return t;
    case PRINTNAT:      printf("PRINTNAT "); return t;
    case READNAT:       printf("READNAT "); return t;
    case IF:            printf("IF "); return t;
    case ELSE:          printf("ELSE "); return t;
    case FOR:           printf("FOR "); return t;
    case PLUS:          printf("PLUS "); return t;
    case MINUS:         printf("MINUS "); return t;
    case TIMES:         printf("TIMES "); return t;
    case EQUALITY:      printf("EQUALITY "); return t;
    case GREATER:       printf("GREATER "); return t;
    case OR:            printf("OR "); return t;
    case NOT:           printf("NOT "); return t;
    case ASSIGN:        printf("ASSIGN "); return t;
    case NUL:           printf("NUL "); return t;
    case NEW:           printf("NEW "); return t;
    case THIS:          printf("THIS "); return t;
    case DOT:           printf("DOT "); return t;
    case SEMICOLON:     printf("SEMICOLON "); return t;
    case LBRACE:        printf("LBRACE "); return t;
    case RBRACE:        printf("RBRACE "); return t;
    case LPAREN:        printf("LPAREN "); return t;
    case RPAREN:        printf("RPAREN "); return t;
    case ENDOFFILE:     printf("ENDOFFILE\n"); return t;
    default:            printf("ERROR: invalid token in scanned().\n"); exit(-1);
  }
}

