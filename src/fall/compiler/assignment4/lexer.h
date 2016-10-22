#ifndef _LEXER_H_
#define _LEXER_H_

#include <stdio.h>
#include <stdlib.h>     //free(), size_t
#include <ctype.h>      //isdigit(), isalpha()
#include <unistd.h>     //NULL
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

/** State definitions for the scanner DFA */
typedef enum
{
  START, INASSIGN, INCOMMENT, INNUM, INID, INOR, DONE
} State;


/* MAXTOKENLEN is the maximum size of a token */
#define MAXTOKENLEN 256     // max length of an identifier

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN+1];

extern int pos, lineNo;            // position and line number tracker


/* function getToken returns the
 * next token in source file
 */
Token getToken(void);

//static void printToken(Token t, char* tokenString);
void printToken(Token t, char* tokenString);

#endif
