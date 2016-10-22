#include <stdio.h>
#include <stdlib.h>     //free(), size_t
#include <ctype.h>      //isdigit(), isalpha()
#include <unistd.h>     //NULL
#include <string.h>     //strcmp()
#include "lexer.c"
#include "ast.c"

static Token token; /* holds current token */

/* function prototypes for recursive calls */
//static TreeNode * stmt_sequence(void);
static ASTree * pgmList(void);
ASTree* parseClassDeclList(void);
ASTree* parseClassDecl(void);
ASTree* parseId(void);
ASTree* parseObject(void);
ASTree* parseSuper(void);
ASTree* parseVarDeclList(void);

static void syntaxError(char * message)
{ /*fprintf(listing,"\n>>> ");
  fprintf(listing,"Syntax error at line %d: %s",lineno,message);*/
  printf("\n>>> ");
  printf("Syntax error at line %d: %s",lineNo, message);
  printToken(token, tokenString);
//  exit(-1);
//  Error = TRUE;
}

Token peek() {
  return token;
}

void printCurrentToken(){
    printf("\n    Current Token Is: ");
    printToken(token, tokenString);
    printf("\n");
}

static void consume(Token t)
{
  if (t == peek()) {
    token = getToken();
  }
  else {
    syntaxError("unexpected token -> ");
//    printToken(token, tokenString);
    exit(-1);
//    fprintf(listing, "      ");
  }
}

ASTree* pgmList(){
  ASTree *pgm = newAST(PROGRAM, NULL, 0, NULL, lineNo);
  ASTree *classDeclList = parseClassDeclList();
  appendToChildrenList(pgm, classDeclList);
  return pgm;
}

ASTree* parseClassDeclList(){
  ASTree *classDeclList = newAST(CLASS_DECL_LIST, NULL, 0, NULL, lineNo);
  while(peek() == CLASS) {
    appendToChildrenList(classDeclList, parseClassDecl());
  }
  return classDeclList;
}

ASTree* parseClassDecl(){
  consume(CLASS);
  ASTree *classDecl = newAST(CLASS_DECL, NULL, 0, NULL, lineNo);
  appendToChildrenList(classDecl, parseId());
  consume(EXTENDS);
  appendToChildrenList(classDecl, parseSuper());
  consume(LBRACE);
  appendToChildrenList(classDecl, parseVarDeclList());
  consume(RBRACE);
  return classDecl;
}

ASTree* parseId(){
//  return newAST(AST_ID, NULL, 0, tokenString, lineNo);
  ASTree *astId = newAST(AST_ID, NULL, 0, tokenString, lineNo);
  consume(ID);
  return astId;
}

ASTree* parseObject(){
//  return newAST(AST_ID, NULL, 0, tokenString, lineNo);
  ASTree *astObj = newAST(OBJ_TYPE, NULL, 0, NULL, lineNo);
  consume(OBJECT);
  return astObj;
}

ASTree* parseSuper(){
//  return newAST(AST_ID, NULL, 0, tokenString, lineNo);
  switch(peek()) {
    case ID:
      return parseId();
    case OBJECT:
      return parseObject();
    default:
      syntaxError("unexpected token -> ");
      exit(-1);
  }
}

ASTree* parseVarDeclList(){
//  return newAST(VAR_DECL_LIST, NULL, 0, NULL, lineNo);
//  consume(ID);
  return newAST(VAR_DECL_LIST, NULL, 0, NULL, lineNo);
}

/*TreeNode * stmt_sequence(void)
{ TreeNode * t = statement();
  TreeNode * p = t;
  while ((token!=ENDFILE) && (token!=END) &&
         (token!=ELSE) && (token!=UNTIL))
  { TreeNode * q;
    match(SEMI);
    q = statement();
    if (q!=NULL) {
      if (t==NULL) t = p = q;
      else *//* now p cannot be NULL either *//*
      { p->sibling = q;
        p = q;
      }
    }
  }
  return t;
}

TreeNode * statement(void)
{ TreeNode * t = NULL;
  switch (token) {
    case IF : t = if_stmt(); break;
    case REPEAT : t = repeat_stmt(); break;
    case ID : t = assign_stmt(); break;
    case READ : t = read_stmt(); break;
    case WRITE : t = write_stmt(); break;
    default : syntaxError("unexpected token -> ");
              printToken(token,tokenString);
              token = getToken();
              break;
  } *//* end case *//*
  return t;
}

TreeNode * if_stmt(void)
{ TreeNode * t = newStmtNode(IfK);
  match(IF);
  if (t!=NULL) t->child[0] = exp();
  match(THEN);
  if (t!=NULL) t->child[1] = stmt_sequence();
  if (token==ELSE) {
    match(ELSE);
    if (t!=NULL) t->child[2] = stmt_sequence();
  }
  match(END);
  return t;
}

TreeNode * repeat_stmt(void)
{ TreeNode * t = newStmtNode(RepeatK);
  match(REPEAT);
  if (t!=NULL) t->child[0] = stmt_sequence();
  match(UNTIL);
  if (t!=NULL) t->child[1] = exp();
  return t;
}

TreeNode * assign_stmt(void)
{ TreeNode * t = newStmtNode(AssignK);
  if ((t!=NULL) && (token==ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  match(ASSIGN);
  if (t!=NULL) t->child[0] = exp();
  return t;
}

TreeNode * read_stmt(void)
{ TreeNode * t = newStmtNode(ReadK);
  match(READ);
  if ((t!=NULL) && (token==ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  return t;
}

TreeNode * write_stmt(void)
{ TreeNode * t = newStmtNode(WriteK);
  match(WRITE);
  if (t!=NULL) t->child[0] = exp();
  return t;
}

TreeNode * exp(void)
{ TreeNode * t = simple_exp();
  if ((token==LT)||(token==EQ)) {
    TreeNode * p = newExpNode(OpK);
    if (p!=NULL) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
    }
    match(token);
    if (t!=NULL)
      t->child[1] = simple_exp();
  }
  return t;
}

TreeNode * simple_exp(void)
{ TreeNode * t = term();
  while ((token==PLUS)||(token==MINUS))
  { TreeNode * p = newExpNode(OpK);
    if (p!=NULL) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
      match(token);
      t->child[1] = term();
    }
  }
  return t;
}

TreeNode * term(void)
{ TreeNode * t = factor();
  while ((token==TIMES)||(token==OVER))
  { TreeNode * p = newExpNode(OpK);
    if (p!=NULL) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
      match(token);
      p->child[1] = factor();
    }
  }
  return t;
}

TreeNode * factor(void)
{ TreeNode * t = NULL;
  switch (token) {
    case NUM :
      t = newExpNode(ConstK);
      if ((t!=NULL) && (token==NUM))
        t->attr.val = atoi(tokenString);
      match(NUM);
      break;
    case ID :
      t = newExpNode(IdK);
      if ((t!=NULL) && (token==ID))
        t->attr.name = copyString(tokenString);
      match(ID);
      break;
    case LPAREN :
      match(LPAREN);
      t = exp();
      match(RPAREN);
      break;
    default:
      syntaxError("unexpected token -> ");
      printToken(token, tokenString);
      token = getToken();
      break;
    }
  return t;
}*/

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
ASTree* parse(void)
{ ASTree *t;
//  token = getToken();
  token = getToken();
  t = pgmList();
  if (token != ENDOFFILE)
    syntaxError("Code ends before file\n");
  return t;
}

int main( int argc, char **argv )
{
  if( argc != 2)
  {
    printf("Usage: dj-ast <file_name> \n");
    exit(-1);
  }

  char *fileName = argv[1];
  char c;

  printf("file name: %s \n", fileName );
/** Open given file in read mode */

  fp = fopen( fileName, "r" );

  if (fp == NULL)
  {
    printf("Error Opening File : %s \n", argv[1] );
    exit(-1);
  }

 /* parse the input program */
  printf("Lexer Output....\n");
  ASTree *pgmAST = parse();
  printf("\n\nAST Output....\n\n");
  printAST(pgmAST);
  return 0;

///** Scan till end of file */
//
//  while (!isEOF) {
//    getToken();
//  }
/** Close the file */

  fclose(fp);

  return 0;
}

//int main(int argc, char **argv) {
//  if(argc!=2) {
//    printf("Usage: dj-parse filename\n");
//    exit(-1);
//  }
//  yyin = fopen(argv[1],"r");
//  if(yyin==NULL) {
//    printf("ERROR: could not open file %s\n",argv[1]);
//    exit(-1);
//  }
//  /* parse the input program */
//  yyparse();
//  printAST(pgmAST);
//  return 0;
//}