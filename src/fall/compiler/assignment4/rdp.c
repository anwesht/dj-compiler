#include <stdio.h>
#include <stdlib.h>     //free(), size_t
#include <ctype.h>      //isdigit(), isalpha()
#include <unistd.h>     //NULL
#include <string.h>     //strcmp()
#include "lexer.h"
#include "ast.h"

//static Token token; /* holds current token */
//static Token nextToken; /* holds the lookahead token */
static TokenType token; /* holds current token */
static TokenType nextToken; /* holds the lookahead token */

/* function prototypes for recursive calls */
//static TreeNode * stmt_sequence(void);
static ASTree * pgmList(void);
ASTree* parseClassDeclList(void);
ASTree* parseClassDecl(void);
ASTree* parseId(void);
ASTree* parseObject(void);
ASTree* parseSuper(void);
ASTree* parseTypeDecl(void);
ASTree* parseVarDeclList(void);
ASTree* parseVarDecl(void);
ASTree* parseExprList(void);
//ASTree* parseExpr(ASTree* e);
ASTree* parseExpr();

ASTree* parseSimpleExpr();
ASTree* parseSimpleArithmeticExpr();
ASTree* parseTerm();
ASTree* parseFactor();
ASTree* parseIdExpr();
ASTree* parseNatLiteralExpr();

ASTree* parseNatType(void);

int getLineNo();

static void syntaxError(char * message)
{ /*fprintf(listing,"\n>>> ");
  fprintf(listing,"Syntax error at line %d: %s",getLineNo(),message);*/
  printf("\n>>> ");
  printf("Syntax error at line %d: %s",getLineNo(), message);
//  printToken(token, tokenString);
  printToken(token);
//  exit(-1);
//  Error = TRUE;
}

/** Peek at the current token */
Token peek() {
  return token.tok;
}

/** Get idAttribute of current token */
char* getIdAttribute() {
  if(token.tok == ID) {
    return token.str;
  } else {
    return NULL;
  }
}

/** Get natAttribute of current token */
int getNatAttribute() {
  if(token.tok == NATLITERAL) {
    return atoi(token.str);
  } else {
    return 0;
  }
}

int getLineNo() {
  return token.lineNo;
}

Token lookahead() {
  return nextToken.tok;
}

void printCurrentToken(){
    printf("\n    Current Token Is: ");
//    printToken(token, tokenString);
    printToken(token);
    printf("\n");
}

static void consume(Token t)
{
  if (t == peek()) {
    token = nextToken;
    nextToken = getToken();
  }
  else {
      syntaxError("unexpected token -> ");
//    printToken(token, tokenString);
    exit(-1);
//    fprintf(listing, "      ");
  }
}

ASTree* pgmList(){
  ASTree *pgm = newAST(PROGRAM, NULL, 0, NULL, getLineNo());
  ASTree *classDeclList = parseClassDeclList();
  appendToChildrenList(pgm, classDeclList);
  return pgm;
}

ASTree* parseClassDeclList(){
  ASTree *astClassDeclList = newAST(CLASS_DECL_LIST, NULL, 0, NULL, getLineNo());
  while(peek() == CLASS) {
    appendToChildrenList(astClassDeclList, parseClassDecl());
  }
  return astClassDeclList;
}

ASTree* parseClassDecl(){
  consume(CLASS);
  ASTree *astClassDecl = newAST(CLASS_DECL, NULL, 0, NULL, getLineNo());
  appendToChildrenList(astClassDecl, parseId());
  consume(EXTENDS);
  appendToChildrenList(astClassDecl, parseSuper());
  consume(LBRACE);
  printf("After LBRACE.");
  printCurrentToken();
  ASTree *astVarDeclList = newAST(VAR_DECL_LIST, NULL, 0, NULL, getLineNo());
  ASTree *astMethodDeclList = newAST(METHOD_DECL_LIST, NULL, 0, NULL, getLineNo());


  //VarDecl OR MethodDecl
  printf("peek is");
  printCurrentToken();
  while((peek() == ID || peek() == OBJECT || peek() == NATTYPE) && (lookahead() == ID)) {
//    appendToChildrenList(astVarDeclList, parseVarDecl());
//    ASTree *astVarDecl = newAST(VAR_DECL, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
    ASTree *astTypeDecl = parseTypeDecl();
    ASTree *astId = parseId();

    printf("here");
    printCurrentToken();
    if(peek() == SEMICOLON) {
      printf("IN var decl");
      ASTree *astVarDecl = newAST(VAR_DECL, NULL, 0, NULL, getLineNo());
      appendToChildrenList(astVarDecl, astTypeDecl);
      appendToChildrenList(astVarDecl, astId);
      consume(SEMICOLON);
      appendToChildrenList(astVarDeclList, astVarDecl);
    } else if(peek() == LPAREN){
      ASTree *astMethodDecl = newAST(METHOD_DECL, NULL, 0, NULL, getLineNo());
      appendToChildrenList(astMethodDecl, astTypeDecl);
      appendToChildrenList(astMethodDecl, astId);
      consume(LPAREN);
      appendToChildrenList(astMethodDecl, parseTypeDecl());
      appendToChildrenList(astMethodDecl, parseId());
      consume(RPAREN);
      consume(LBRACE);
      appendToChildrenList(astMethodDecl, parseVarDeclList());
      //append expr list.
      appendToChildrenList(astMethodDecl, parseExprList());
      printf("After children expr list");
      printCurrentToken();
      consume(RBRACE);
      appendToChildrenList(astMethodDeclList, astMethodDecl);
    } else {
        syntaxError("unexpected token -> ");
      exit(-1);
    }
  }

  appendToChildrenList(astClassDecl, astVarDeclList);
  appendToChildrenList(astClassDecl, astMethodDeclList);
  consume(RBRACE);
  return astClassDecl;
}

ASTree* parseId(){
//  return newAST(AST_ID, NULL, 0, tokenString, getLineNo());
//  ASTree *astId = newAST(AST_ID, NULL, 0, tokenString, getLineNo());
  ASTree *astId = newAST(AST_ID, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
  consume(ID);
  return astId;
}

ASTree* parseObject(){
//  return newAST(AST_ID, NULL, 0, tokenString, getLineNo());
  ASTree *astObj = newAST(OBJ_TYPE, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
  consume(OBJECT);
  return astObj;
}

ASTree* parseSuper(){
//  return newAST(AST_ID, NULL, 0, tokenString, getLineNo());
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

ASTree* parseTypeDecl() {
  ASTree *astTypeDecl;
  switch (peek()) {
    case ID:
      astTypeDecl = parseId();
      break;
    case OBJECT:
      astTypeDecl = parseObject();
      break;
    case NATTYPE:
      astTypeDecl = parseNatType();
      break;
    default:
      syntaxError("Compiler Error. ");
      exit(-1);
  }
  return astTypeDecl;
}

ASTree* parseNatType() {
  ASTree *astNatType = newAST(NAT_TYPE, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
  consume(NATTYPE);
  return astNatType;
}

ASTree* parseVarDeclList(){
//  ASTree *astVarDeclList = newAST(VAR_DECL_LIST, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
  ASTree *astVarDeclList = newAST(VAR_DECL_LIST, NULL, 0, NULL, getLineNo());

  while((peek() == ID || peek() == OBJECT || peek() == NATTYPE) && (lookahead() == ID)) {
    appendToChildrenList(astVarDeclList, parseVarDecl());
  }
  return astVarDeclList;
}

ASTree* parseVarDecl() {
  ASTree *astVarDecl = newAST(VAR_DECL, NULL, 0, NULL, getLineNo());
  appendToChildrenList(astVarDecl, parseTypeDecl());
  appendToChildrenList(astVarDecl, parseId());
  consume(SEMICOLON);
  return astVarDecl;
}

ASTree* parseExprList() {
  ASTree *astExprList = newAST(EXPR_LIST, NULL, 0, NULL, getLineNo());
  while(peek() != RBRACE) {
//    appendToChildrenList(astExprList, parseExpr());
    ASTree *expr = parseExpr();
    consume(SEMICOLON);
    appendToChildrenList(astExprList, expr);
    printf("after expr");
//    switch(peek()) {
//      case
//    }
//    consume(SEMICOLON);
  }
  printf("after expr list");
  printCurrentToken();
  return astExprList;
}
ASTree* parseExpr() {
  ASTree *e = NULL;
  switch(peek()) {
    case IF:
      consume(IF);
      e = newAST(IF_THEN_ELSE_EXPR, NULL, 0, NULL, getLineNo());
      consume(LPAREN);
      appendToChildrenList(e, parseExpr());
      consume(RPAREN);
      consume(LBRACE);
      printf("\nIF EXPRESSION LIST:\n");
      appendToChildrenList(e, parseExprList());
      consume(RBRACE);
      consume(ELSE);
      consume(LBRACE);
      printf("\nELSE EXPRESSION LIST\n");
      appendToChildrenList(e, parseExprList());
      consume(RBRACE);
//      consume(SEMICOLON);
      return e;

    case FOR:
      e = newAST(FOR_EXPR, NULL, 0, NULL, getLineNo());
      consume(FOR);
      consume(LPAREN);
      appendToChildrenList(e, parseExpr());
      consume(SEMICOLON);
      appendToChildrenList(e, parseExpr());
      consume(SEMICOLON);
      appendToChildrenList(e, parseExpr());
      consume(RPAREN);
      consume(LBRACE);
      appendToChildrenList(e, parseExprList());
      consume(RBRACE);
//      consume(SEMICOLON);
      return e;

     case PRINTNAT:
      e = newAST(PRINT_EXPR, parseExpr(), 0, NULL, getLineNo());
//      consume(SEMICOLON);
      return e;

     case READNAT:
      e = newAST(READ_EXPR, NULL, 0, NULL, getLineNo());
      consume(READNAT);
//      consume(SEMICOLON);
      return e;

     default:
      e = parseSimpleExpr();
      printf("SIMPLE EXPR RECEIVED IS: %u", e->typ);
  }

/*  if(peek() == EQUALITY) {
    consume(EQUALITY);
    ASTree *astEqualityExpr = newAST(EQUALITY_EXPR, e, 0, NULL, getLineNo());
    appendToChildrenList(astEqualityExpr, parseSimpleExpr());
    e = astEqualityExpr;
  } else if(peek() == GREATER) {
    consume(GREATER);
    ASTree *astGreaterThanExpr = newAST(GREATER_THAN_EXPR, e, 0, NULL, getLineNo());
    appendToChildrenList(astGreaterThanExpr, parseSimpleExpr());
    e = astGreaterThanExpr;
  }*/

  while(peek() == OR){
    consume(OR);
    if(e->typ == PLUS_EXPR) {
      printf("child is plus expr: ");
    } else if(e->typ == MINUS_EXPR) {
      printf("child is MInus expr: ");
    } else if (e->typ == ID_EXPR) {
      printf("child is ID expre.");
    }
    else {
      printf("child is unknown expr: %u", e->typ);
    }
    ASTree *orExpr = newAST(OR_EXPR, e, 0, NULL, getLineNo());
//    appendToChildrenList(orExpr, parseExpr());
    appendToChildrenList(orExpr, parseSimpleExpr());
    e = orExpr;
  }

  while(peek() == ASSIGN) {
    consume(ASSIGN);
    printf("consuming assign");
    printCurrentToken();
    ASTree *lhs;
    if(e->typ == ID_EXPR) {
      lhs = e->children->data;
    } else {
      lhs = e;
    }
    ASTree *astAssignExpr = newAST(ASSIGN_EXPR, lhs, 0, NULL, getLineNo());
    appendToChildrenList(astAssignExpr, parseExpr());
    e = astAssignExpr;
//    consume(SEMICOLON);
    printf("end of assign");
//    return e;
  }

  printf("\n\nafter assign; RETURNUNG %u \n\n", e->typ);
  printCurrentToken();
    /*if(peek() != SEMICOLON) {
//      appendToChildrenList(e, parseExpr());
      parseExpr(e);
    }*/
//  consume(SEMICOLON);
//  }
  return e;
}
ASTree* parseSimpleExpr() {
  ASTree* se = parseSimpleArithmeticExpr();
  if(peek() == EQUALITY) {
      consume(EQUALITY);
      ASTree *astEqualityExpr = newAST(EQUALITY_EXPR, se, 0, NULL, getLineNo());
      appendToChildrenList(astEqualityExpr, parseSimpleArithmeticExpr());
      se = astEqualityExpr;
  } else if(peek() == GREATER) {
      consume(GREATER);
      ASTree *astGreaterThanExpr = newAST(GREATER_THAN_EXPR, se, 0, NULL, getLineNo());
      appendToChildrenList(astGreaterThanExpr, parseSimpleArithmeticExpr());
      se = astGreaterThanExpr;
  }
  return se;
}

ASTree* parseSimpleArithmeticExpr() {
  ASTree *se = parseTerm();
  printf("after parse term:");
  printCurrentToken();
  while ((peek() == PLUS)||(peek() == MINUS)) {
//    ASTree *op = NULL;
    if(peek() == PLUS) {
    printf("INSIDE PLUS LOOP");
      ASTree *astPlusExpr = newAST(PLUS_EXPR, se, 0, NULL, getLineNo());
      consume(PLUS);
      appendToChildrenList(astPlusExpr, parseTerm());
      se = astPlusExpr;
    } else if( peek() == MINUS){
        printf("INSIDE MINUS LOOP");
      ASTree *astMinusExpr = newAST(MINUS_EXPR, se, 0, NULL, getLineNo());
      consume(MINUS);
      appendToChildrenList(astMinusExpr, parseTerm());
      se = astMinusExpr;
    }
  }
  printf("after plus minus: %u", se->typ);
  printCurrentToken();
  return se;
}

ASTree* parseTerm() {
  ASTree *t = parseFactor();

  // check DOT
  /*while(peek() == DOT ) {
    consume(DOT);
    if(peek() == ID && lookahead() == LPAREN){
      // This is dot method call expr
      ASTree *astDotMethodCallExpr = newAST(DOT_METHOD_CALL_EXPR, t, 0, NULL, getLineNo());
      appendToChildrenList(astDotMethodCallExpr, parseId());
      consume(LPAREN);
      appendToChildrenList(astDotMethodCallExpr, parseExpr());
      consume(RPAREN);
      t = astDotMethodCallExpr;
    } else if(peek() == ID) {
      // This is dot id Expr
      ASTree *astDotIdExpr = newAST(DOT_ID_EXPR, t, 0, NULL, getLineNo());
      appendToChildrenList(astDotIdExpr, parseId());
      t = astDotIdExpr;
    }
    printf("\nCurrent token at end of DOT Loop\n");
    printCurrentToken();
  }*/

  while (peek() == TIMES) {
    ASTree *astTimesExpr = newAST(TIMES_EXPR, t, 0, NULL, getLineNo());
    consume(TIMES);
    appendToChildrenList(astTimesExpr, parseFactor());
    t = astTimesExpr;
  }

  return t;
}

ASTree* parseFactor() {
  ASTree *e = NULL;
  switch(peek()){
    case NATLITERAL:
      e = parseNatLiteralExpr();
      break;

    case ID:
      e = parseIdExpr();
      break;

    case NUL:
      e = newAST(NULL_EXPR, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
      break;

    case THIS:
      e = newAST(THIS_EXPR, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
      break;

    case OBJECT:
      e = parseObject();
      break;

    case LPAREN :
      consume(LPAREN);
      e = parseExpr();
      consume(RPAREN);
      break;

    case NOT:
      consume(NOT);
      e = newAST(NOT_EXPR, NULL, 0, NULL, getLineNo());
      appendToChildrenList(e, parseExpr());
      break;

    default:
        syntaxError("unexpected token -> ");
      exit(-1);
  }

    // check DOT
    while(peek() == DOT ) {
      consume(DOT);
      if(peek() == ID && lookahead() == LPAREN){
        // This is dot method call expr
        ASTree *astDotMethodCallExpr = newAST(DOT_METHOD_CALL_EXPR, e, 0, NULL, getLineNo());
        appendToChildrenList(astDotMethodCallExpr, parseId());
        consume(LPAREN);
        appendToChildrenList(astDotMethodCallExpr, parseExpr());
        consume(RPAREN);
        e = astDotMethodCallExpr;
      } else if(peek() == ID) {
        // This is dot id Expr
        ASTree *astDotIdExpr = newAST(DOT_ID_EXPR, e, 0, NULL, getLineNo());
        appendToChildrenList(astDotIdExpr, parseId());
        e = astDotIdExpr;
      }
      printf("\nCurrent token at end of DOT Loop\n");
      printCurrentToken();
    }
  return e;
}

ASTree* parseDotIdExpr(ASTree *e) {
  consume(DOT);
  ASTree *astDotIdExpr = newAST(DOT_ID_EXPR, e, 0, NULL, getLineNo());
  appendToChildrenList(astDotIdExpr, parseId());
  return astDotIdExpr;
}

ASTree* parseNatLiteralExpr() {
  ASTree *astNatLiteralExpr = newAST(NAT_LITERAL_EXPR, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
  consume(NATLITERAL);
  return astNatLiteralExpr;
}

ASTree* parseIdExpr() {
  ASTree *astIdExpr = newAST(ID_EXPR, parseId(), 0, NULL, getLineNo());
  return astIdExpr;
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
  nextToken = getToken();
  t = pgmList();
  if (token.tok != ENDOFFILE)
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