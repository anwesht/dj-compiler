/**
 * Created by atuladhar on 10/20/16.
 * Pledge: I pledge my Honor that I have not cheated, and will not cheat, on this assignment
 * Name: Anwesh Tuladhar
 */

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "ast.h"
#include "symtbl.h"
#include "typecheck.h"

typedef enum
{
  false,
  true
} bool;

static TokenType token;     /* holds current token */
static TokenType nextToken; /* holds the lookahead token */

/* function prototypes for recursive calls */
ASTree* parsePGM(void);

ASTree* parseClassDeclList(void);
ASTree* parseClassDecl(void);

ASTree* parseVarDeclList(void);
ASTree* parseVarDecl(void);
ASTree* parseTypeDecl(void);

ASTree* parseFieldDecl(ASTree *astTypeDecl, ASTree *astId);
ASTree *parseMethodDecl(ASTree *astTypeDecl, ASTree *astId);

ASTree* parseExprList(void);
ASTree* parseExpr(void);
ASTree* parseSimpleExpr(void);
ASTree* parseSimpleArithmeticExpr(void);

ASTree* parseTerm(void);
ASTree* parseFactor(void);

ASTree* parseId(void);
ASTree* parseObject(void);
ASTree* parseSuper(void);

ASTree* parseIdExpr(void);
ASTree* parseDotIdExpr(ASTree *e);
ASTree* parseNatType(void);
ASTree* parseNatLiteralExpr(void);

ASTree* parse(void);

int getLineNo(void);

/** Print Error Message
  * @param message => The error message
  */
static void syntaxError(char *message) {
  printf("\n>>> ");
  printf("Syntax error at line %d: %s", getLineNo(), message);
  printToken(token);
  printf("\n");
}

/** Peek at the current token
  * @return => Current TokenType's tok member.
  */
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

/** Returns the current token's line number */
int getLineNo() {
  return token.lineNo;
}

/** Update the lineNumber of provided ASTree
  * to that of current token.
  * @param t => The ASTree to update
  */
void updateLineNo(ASTree *t) {
  if(t != NULL){
    t->lineNumber = getLineNo();
  }
}

/** Get the lookahead Token
  * @returns => lookahead TokenType's tok member.
  */
Token lookahead() {
  return nextToken.tok;
}

/** Prints the current token
  * Use for debugging.
  */
void printCurrentToken(){
    printf("\n    Current Token Is: ");
    printToken(token);
    printf("\n");
}

/** Consumes the current token if the current token is the expected one.
  * @param t => the expected token
  */
static void consume(Token t)
{
  if (t == peek()) {
    token = nextToken;
    nextToken = getToken();
  }
  else {
    syntaxError("unexpected token -> ");
    exit(-1);
  }
}

ASTree* parsePGM(){
  ASTree *astPGM = newAST(PROGRAM, NULL, 0, NULL, getLineNo());
  ASTree *astClassDeclList = parseClassDeclList();

  appendToChildrenList(astPGM, astClassDeclList);       //All Class Declarations
  consume(MAIN);
  consume(LBRACE);
  appendToChildrenList(astPGM, parseVarDeclList());     //Main Locals
  appendToChildrenList(astPGM, parseExprList());        //Main Expression List
  updateLineNo(astPGM);                                 //End of Program
  consume(RBRACE);
  return astPGM;
}

ASTree* parseClassDeclList(){
  ASTree *astClassDeclList = newAST(CLASS_DECL_LIST, NULL, 0, NULL, getLineNo());
  while(peek() == CLASS) {
    appendToChildrenList(astClassDeclList, parseClassDecl());   //add Class Decls
  }
  updateLineNo(astClassDeclList);
  return astClassDeclList;
}

ASTree* parseFieldDecl(ASTree *astTypeDecl, ASTree *astId) {
  ASTree *astVarDecl = newAST(VAR_DECL, NULL, 0, NULL, getLineNo());
  appendToChildrenList(astVarDecl, astTypeDecl);    //Type of var
  appendToChildrenList(astVarDecl, astId);          //varName
  updateLineNo(astVarDecl);                         //End of VarDecl
  consume(SEMICOLON);
  return astVarDecl;
//  appendToChildrenList(astVarDeclList, astVarDecl);
}

ASTree *parseMethodDecl(ASTree *astTypeDecl, ASTree *astId) {
  ASTree *astMethodDecl = newAST(METHOD_DECL, NULL, 0, NULL, getLineNo());
  appendToChildrenList(astMethodDecl, astTypeDecl);         //Return type of method
  appendToChildrenList(astMethodDecl, astId);               //Method Name
  consume(LPAREN);
  appendToChildrenList(astMethodDecl, parseTypeDecl());     //Argument Type
  appendToChildrenList(astMethodDecl, parseId());           //Argument Name
  consume(RPAREN);
  consume(LBRACE);
  appendToChildrenList(astMethodDecl, parseVarDeclList());  //Method Locals
  appendToChildrenList(astMethodDecl, parseExprList());     //Method Expressions
  updateLineNo(astMethodDecl);                              //End of MethodDecl
  consume(RBRACE);
//  appendToChildrenList(astMethodDeclList, astMethodDecl);
  return astMethodDecl;
}

ASTree* parseClassDecl(){
  consume(CLASS);
  ASTree *astClassDecl = newAST(CLASS_DECL, NULL, 0, NULL, getLineNo());
  appendToChildrenList(astClassDecl, parseId());
  consume(EXTENDS);
  appendToChildrenList(astClassDecl, parseSuper());
  consume(LBRACE);
  ASTree *astVarDeclList = newAST(VAR_DECL_LIST, NULL, 0, NULL, getLineNo());
  ASTree *astMethodDeclList = newAST(METHOD_DECL_LIST, NULL, 0, NULL, getLineNo());


  /** VarDecl OR MethodDecl -> Depending on the lookahead token. */
  ASTree *astTypeDecl = NULL;
  ASTree *astId = NULL;
  while((peek() == ID || peek() == OBJECT || peek() == NATTYPE) && (lookahead() == ID)) {
    /*ASTree *astTypeDecl = parseTypeDecl();
    ASTree *astId = parseId();*/
    astTypeDecl = parseTypeDecl();
    astId = parseId();

    /** VarDecl */
    if(peek() == SEMICOLON) {
      appendToChildrenList(astVarDeclList, parseFieldDecl(astTypeDecl, astId));
    } else if (peek() == LPAREN){
      appendToChildrenList(astMethodDeclList, parseMethodDecl(astTypeDecl, astId));
      break;
    } else {
      syntaxError("unexpected token -> ");
      exit(-1);
    }
  }
  while((peek() == ID || peek() == OBJECT || peek() == NATTYPE) && (lookahead() == ID)) {
    astTypeDecl = parseTypeDecl();
    astId = parseId();
    /** Method Decl */
    if(peek() == LPAREN){
      appendToChildrenList(astMethodDeclList, parseMethodDecl(astTypeDecl, astId));
    } else {
      syntaxError("unexpected token -> ");
      exit(-1);
    }
  }

  appendToChildrenList(astClassDecl, astVarDeclList);       //Class Fields
  appendToChildrenList(astClassDecl, astMethodDeclList);    //Class Methods
  updateLineNo(astMethodDeclList);                          //End of Methods
  updateLineNo(astClassDecl);                               //End of Class
  consume(RBRACE);
  return astClassDecl;
}

ASTree* parseId(){
  ASTree *astId = newAST(AST_ID, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
  consume(ID);
  return astId;
}

ASTree* parseObject(){
  ASTree *astObj = newAST(OBJ_TYPE, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
  consume(OBJECT);
  return astObj;
}

ASTree* parseSuper(){
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
      syntaxError("unexpected token -> ");
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
  ASTree *astVarDeclList = newAST(VAR_DECL_LIST, NULL, 0, NULL, getLineNo());

  while((peek() == ID || peek() == OBJECT || peek() == NATTYPE) && (lookahead() == ID)) {
    appendToChildrenList(astVarDeclList, parseVarDecl());
  }
  updateLineNo(astVarDeclList);
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
  if(peek() == RBRACE || peek() == ENDOFFILE) {
    syntaxError("Expression List empty.\nunexpected token -> ");
    exit(-1);
  }
  while(peek() != RBRACE) {
    ASTree *expr = parseExpr();
    consume(SEMICOLON);
    appendToChildrenList(astExprList, expr);
  }
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
      appendToChildrenList(e, parseExprList());
      consume(RBRACE);
      consume(ELSE);
      consume(LBRACE);
      appendToChildrenList(e, parseExprList());
      updateLineNo(e);
      consume(RBRACE);
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
      updateLineNo(e);
      consume(RBRACE);
      return e;

    default:
      e = parseSimpleExpr();
  }

  if(peek() == OR) {
    while(peek() == OR){
      consume(OR);
      ASTree *orExpr = newAST(OR_EXPR, e, 0, NULL, getLineNo());
      appendToChildrenList(orExpr, parseSimpleExpr());
      e = orExpr;
    }
  }
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
  if(peek() == ASSIGN){
    if(se->typ == ID_EXPR) {
      ASTree *lhs = se->children->data;
      consume(ASSIGN);
      ASTree *astAssignExpr = newAST(ASSIGN_EXPR, lhs, 0, NULL, getLineNo());
      ASTree *rhs = parseExpr();
      appendToChildrenList(astAssignExpr, rhs);
      se = astAssignExpr;
    } else if (se->typ == DOT_ID_EXPR){
      ASTree *lhsExpr = se->children->data;
      ASTree *lhsId = se->childrenTail->data;
      consume(ASSIGN);
      ASTree *astDotAssignExpr = newAST(DOT_ASSIGN_EXPR, lhsExpr, 0, NULL, getLineNo());
      appendToChildrenList(astDotAssignExpr, lhsId);
      ASTree *rhs = parseExpr();
      appendToChildrenList(astDotAssignExpr, rhs);
      se = astDotAssignExpr;
    } else {
      syntaxError("Invalid LHS to the assignment expression.");
      exit(-1);
    }
  }
  return se;
}

ASTree* parseSimpleArithmeticExpr() {
  ASTree *se = parseTerm();
  while ((peek() == PLUS)||(peek() == MINUS)) {
    if(peek() == PLUS) {
      ASTree *astPlusExpr = newAST(PLUS_EXPR, se, 0, NULL, getLineNo());
      consume(PLUS);
      appendToChildrenList(astPlusExpr, parseTerm());
      se = astPlusExpr;
    } else if(peek() == MINUS) {
      ASTree *astMinusExpr = newAST(MINUS_EXPR, se, 0, NULL, getLineNo());
      consume(MINUS);
      appendToChildrenList(astMinusExpr, parseTerm());
      se = astMinusExpr;
    }
  }
  return se;
}

ASTree* parseTerm() {
  ASTree *t = parseFactor();

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
      if(peek() == LPAREN ) {
        consume(LPAREN);
        ASTree *astId = e->children->data;
        ASTree *astMethodCall = newAST(METHOD_CALL_EXPR, astId, 0, NULL, getLineNo());
        appendToChildrenList(astMethodCall, parseExpr());
        consume(RPAREN);
        e = astMethodCall;
      }
      break;

    case NUL:
      e = newAST(NULL_EXPR, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
      consume(NUL);
      break;

    case THIS:
      e = newAST(THIS_EXPR, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
      consume(THIS);
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
      while(peek() == NOT) {
        consume(NOT);
        ASTree *astNotExpr = newAST(NOT_EXPR, e, 0, NULL, getLineNo());
        appendToChildrenList(astNotExpr, parseFactor());
        e = astNotExpr;
      }
      break;

    case PRINTNAT:
      consume(PRINTNAT);
      e = newAST(PRINT_EXPR, NULL, 0, NULL, getLineNo());
      appendToChildrenList(e, parseExpr());
      break;

    case READNAT:
      e = newAST(READ_EXPR, NULL, 0, NULL, getLineNo());
      consume(READNAT);
      consume(LPAREN);
      consume(RPAREN);
      break;

    case NEW:
      consume(NEW);
      ASTree *astNewExpr = newAST(NEW_EXPR, NULL, 0, NULL, getLineNo());
      if(peek() == ID) {
        appendToChildrenList(astNewExpr, parseId());
      } else if (peek() == OBJECT) {
        appendToChildrenList(astNewExpr, parseObject());
      } else {
        syntaxError("unexpected token -> ");
        exit(-1);
      }
      consume(LPAREN);
      consume(RPAREN);
      e = astNewExpr;
      break;

    default:
      syntaxError("unexpected token -> ");
      exit(-1);
  }

  while(peek() == DOT ) {
    consume(DOT);
    if(peek() == ID && lookahead() == LPAREN){
      /** Dot method call expr */
      ASTree *astDotMethodCallExpr = newAST(DOT_METHOD_CALL_EXPR, e, 0, NULL, getLineNo());
      appendToChildrenList(astDotMethodCallExpr, parseId());
      consume(LPAREN);
      appendToChildrenList(astDotMethodCallExpr, parseExpr());
      consume(RPAREN);
      e = astDotMethodCallExpr;
    } else if(peek() == ID) {
      /** Dot id expr */
      ASTree *astDotIdExpr = newAST(DOT_ID_EXPR, e, 0, NULL, getLineNo());
      appendToChildrenList(astDotIdExpr, parseId());
      e = astDotIdExpr;
    }
  }
  return e;
}

/*ASTree* parseDotIdExpr(ASTree *e) {
  consume(DOT);
  ASTree *astDotIdExpr = newAST(DOT_ID_EXPR, e, 0, NULL, getLineNo());
  appendToChildrenList(astDotIdExpr, parseId());
  return astDotIdExpr;
}*/

ASTree* parseNatLiteralExpr() {
  ASTree *astNatLiteralExpr = newAST(NAT_LITERAL_EXPR, NULL, getNatAttribute(), getIdAttribute(), getLineNo());
  consume(NATLITERAL);
  return astNatLiteralExpr;
}

ASTree* parseIdExpr() {
  ASTree *astIdExpr = newAST(ID_EXPR, parseId(), 0, NULL, getLineNo());
  return astIdExpr;
}

/** the primary function of the parser
  * Function parse returns the newly constructed Abstract Syntax Tree
  */
ASTree* parse(void) {
  ASTree *t;
  token = getToken();
  nextToken = getToken();
  t = parsePGM();
  if (token.tok != ENDOFFILE) {
    syntaxError("Code ends before EOF\nunexpected token -> ");
    exit(-1);
  }
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
  if (fp == NULL) {
    printf("Error Opening File : %s \n", argv[1] );
    exit(-1);
  }

  bool DEBUG = false;
  /* parse the input program */
  ASTree *pgmAST = parse();
  /* Print AST */
  if(DEBUG) printAST(pgmAST);

  setupSymbolTables(pgmAST);
  typecheckProgram();
  /** Close the file */
  fclose(fp);

  return 0;
}