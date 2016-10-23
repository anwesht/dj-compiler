/** Created by atuladhar on 10/20/16.
  * Pledge: I pledge my Honor that I have not cheated, and will not cheat, on this assignment
  * Name: Anwesh Tuladhar
  */
#ifndef _LEXER_H_
#define _LEXER_H_

#include <stdio.h>
#include <stdlib.h>     //free(), size_t
#include <ctype.h>      //isdigit(), isalpha()
#include <string.h>     //strcmp()

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/** Token definition for DJ */
typedef enum
{
  CLASS, ID, EXTENDS, MAIN, NATTYPE, OBJECT,
  NATLITERAL, PRINTNAT, READNAT, IF, ELSE, FOR,
  PLUS, MINUS, TIMES, EQUALITY, GREATER, OR, NOT,
  ASSIGN, NUL, NEW, THIS, DOT, SEMICOLON,
  LBRACE, RBRACE, LPAREN, RPAREN, ENDOFFILE,
  ERROR
} Token;

/** Data structure to store information related to tokens. */
typedef struct
{  char* str;       //The actual string/semantic value.
   Token tok;       //Token
   int lineNo;      //position of token in file
} TokenType;

/** State definitions for the scanner DFA */
typedef enum
{
  START, INASSIGN, INCOMMENT, INNUM, INID, INOR, DONE
} State;

#define MAXTOKENLEN 256     // max length of an identifier token
#define MAXRESERVED 13      // Number of reserved words.

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];

extern int pos, lineNo;            // position and line number tracker
extern FILE *fp;                           // file to be tokenized.

/** function getToken returns the
  * next token in source file
  */
TokenType getToken(void);

/** Print the provided token
  * @param t => Token to print
  */
void printToken(TokenType t);

#endif
