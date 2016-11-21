//
// Created by Anwesh Tuladhar on 11/20/16.
//
#include "codegen.h"
#include "symtbl.h"
#include <stdarg.h>

#define MAX_DISM_ADDR 65535
#define RED   "\x1B[31m"
#define NORMAL "\x1B[0m"

typedef enum
{
  false,
  true
} bool;

void codeGenNatLitExpr(ASTree*);
void codeGenPrintExpr(ASTree*, int, int);
void codeGenReadExpr(void);
void codeGenPlusExpr(ASTree*, int, int);
void codeGenMinusExpr(ASTree*, int, int);
void codeGenTimesExpr(ASTree*, int, int);

void genPrologueMain(void);
void genEpilogueMain(void);

void codeGenBinaryExpr(const ASTree *t, int classNumber, int methodNumber);

/* Global for the DISM output file */
FILE *fout;

/** Write to output file.
 * @param label => the label of DISM instruction
 * @param dismFormat => String representing the DISM instruction.
 * @param comment => String representing the DISM instruction comment.
 * @param ...
 */
void _write(char *label, const char* dismFormat, const char* comment, va_list args){
  fprintf(fout, "%s", label);
  vfprintf(fout, dismFormat, args);
  fprintf(fout, "    ; %s\n", comment);
}

void write(const char* dismFormat, const char* comment, ...) {
  va_list args;
  va_start(args, comment);
  _write("        ", dismFormat, comment, args);
}

void writeWithLabel(const char* dismFormat, const char* comment, ...){
  va_list args;
  va_start(args, comment);
  _write("", dismFormat, comment, args);
}

/* Global to remember the next unique label number to use */
unsigned int labelNumber = 0;

/* Get and increment label number */
unsigned int getNewLabelNumber() {
  return labelNumber++;
}

/* Get current value of label number*/
unsigned int getLabelNumber() {
  return labelNumber - 1;
}

/* Print a message and exit under an exceptional condition */
void internalCGerror(char *msg){
  printf(RED"\nError generating code.>>> "NORMAL);
  printf("\n  %s\n", msg);
  printf("\n");
}

/* Using the global classesST, calculate the total number of fields,
 including inherited fields, in an object of the given type */
int getNumObjectFields(int type);

/* Generate code that increments the stack pointer */
void incSP(){
  write("mov 1 1", "R[r1] <- 1 (move immediate value)");
  write("add 6 6 1", "SP++");
}

/* Generate code that decrements the stack pointer */
void decSP(){
//  fprintf(fout, "        mov 1 1    ;  R[r1] <- 1 (move immediate value)");
  write("mov 1 1", "R[r1] <- 1 (move immediate value)");
  write("sub 6 6 1", "SP--");
  write("blt 5 6 #%d", "Branch if HP < SP", getNewLabelNumber());
  write("mov 1 77", "error code 77 => out of stack memory");
  write("hlt 1", " out of stack memory! (SP < HP)");
  writeWithLabel("#%d: mov 0 0", "Landing for decSP", getLabelNumber());
}

/* Output code to check for a null value at the top of the stack.
 If the top stack value (at M[SP+1]) is null (0), the DISM code
 output will halt. */
void checkNullDereference();

/* Generate DISM code for the given single expression, which appears
 in the given class and method (or main block).
 If classNumber < 0 then methodNumber may be anything and we assume
 we are generating code for the program's main block. */
void codeGenExpr(ASTree *t, int classNumber, int methodNumber){
  if(t == NULL) internalCGerror("Nothing to Code Gen.");

  switch(t->typ) {
    case NAT_LITERAL_EXPR:
      codeGenNatLitExpr(t);
      break;
    case THIS_EXPR:

    case NEW_EXPR:

    case READ_EXPR:
      codeGenReadExpr();
      break;

    case PRINT_EXPR:
      codeGenPrintExpr(t, classNumber, methodNumber);
      break;

    case NULL_EXPR:

    case NOT_EXPR:
    case AST_ID:
    case ID_EXPR:

    case DOT_ID_EXPR:

    case ASSIGN_EXPR:

    case EQUALITY_EXPR:
    case OR_EXPR:

    case GREATER_THAN_EXPR:

    case PLUS_EXPR:
      codeGenPlusExpr(t, classNumber, methodNumber);
      break;

    case MINUS_EXPR:
      codeGenMinusExpr(t, classNumber, methodNumber);
      break;

    case TIMES_EXPR:
      codeGenTimesExpr(t, classNumber, methodNumber);
      break;

    case DOT_ASSIGN_EXPR:

    case METHOD_CALL_EXPR:

    case DOT_METHOD_CALL_EXPR:

    case IF_THEN_ELSE_EXPR:

    case FOR_EXPR:

    default:
      printf("Unknown Expression.");
      exit(-1);
  }
}

/* Generate DISM code for an expression list, which appears in
 the given class and method (or main block).
 If classNumber < 0 then methodNumber may be anything and we assume
 we are generating code for the program's main block. */
void codeGenExprs(ASTree *expList, int classNumber, int methodNumber) {
  ASTList *currentNode = expList->children;
  while(currentNode != NULL && currentNode->data != NULL) {
    codeGenExpr(currentNode->data, classNumber, methodNumber);
    currentNode = currentNode->next;
  }
}

void genPrologueMain();

/* Generate DISM code as the prologue to the given method or main
 block. If classNumber < 0 then methodNumber may be anything and we
 assume we are generating code for the program's main block. */
void genPrologue(int classNumber, int methodNumber) {
  /* Main block prologue */
  if(classNumber < 0) {
    genPrologueMain();
  } else {
    printf("genPrologue some class");
  }

}

void genPrologueMain() {
  write("mov 7 %d", "initialize FP", MAX_DISM_ADDR);
  write("mov 6 %d", "initialize SP", MAX_DISM_ADDR);
  write("mov 0 0", "ALLOCATE STACK SPACE FOR MAIN LOCALS");
  write("mov 0 0", "BEGIN METHOD/MAIN-BLOCK BODY");
  int i;
  for(i = 0; i < numMainBlockLocals; i += 1) {
      write("mov 1 0", "Initializing Main Locals");
      write("str 6 0 1", "M[SP] <- R[r1]");
  }
}

/* Generate DISM code as the epilogue to the given method or main
 block. If classNumber < 0 then methodNumber may be anything and we
 assume we are generating code for the program's main block. */
void genEpilogue(int classNumber, int methodNumber){
  if(classNumber < 0){
    genEpilogueMain();
  } else {
    printf("gen Epilogue some class");
  }
}

void genEpilogueMain() {
  write("hlt 0", "NORMAL TERMINATION AT END OF MAIN BLOCK");
}

/* Generate DISM code for the given method or main block.
 If classNumber < 0 then methodNumber may be anything and we assume
 we are generating code for the program's main block. */
void genBody(int classNumber, int methodNumber);

/* Map a given (1) static class number, (2) a method number defined
 in that class, and (3) a dynamic object's type to:
 (a) the dynamic class number and (b) the dynamic method number that
 actually get called when an object of type (3) dynamically invokes
 method (2).
 This method assumes that dynamicType is a subtype of staticClass. */
void getDynamicMethodInfo(int staticClass, int staticMethod,
                          int dynamicType, int *dynamicClassToCall, int *dynamicMethodToCall);

/* Emit code for the program's vtable, beginning at label #VTABLE.
 The vtable jumps (i.e., dispatches) to code based on
 (1) the dynamic calling object's address (at M[SP+4]),
 (2) the calling object's static type (at M[SP+3]), and
 (3) the static method number (at M[SP+2]). */
void genVTable();

void generateDISM(FILE *outputFile) {
  /* Set global output file pointer */
  fout = outputFile;
  genPrologue(-1, -1);
  codeGenExprs(mainExprs, -1, -1);
  genEpilogue(-1, -1);
}

void codeGenNatLitExpr(ASTree *t) {
  write("mov 1 %d", "R[r1] <- value of Nat", t->natVal);
  write("str 6 0 1", "M[SP] <- R[r1] (a nat literal)");
  decSP();
}

void codeGenPrintExpr(ASTree *t, int classNumber, int methodNumber){
  ASTList *printNatNode = t->children;
  /* Push expr to top of stack. */
  codeGenExpr(printNatNode->data, classNumber, methodNumber);
  write("lod 1 6 1", "R[r1] <- M[SP + 1]");
  write("ptn 1", "print nat");
}

void codeGenReadExpr() {
  write("rdn 1", "R[r1] <- Input nat from stdin");
  write("str 6 0 1", "M[SP] <- R[r1]");
  decSP();
}

void codeGenPlusExpr(ASTree *t, int classNumber, int methodNumber){
  codeGenBinaryExpr(t, classNumber, methodNumber);
  write("add 1 1 2", "R[r1] <- R[r1] + R[r2]");
  write("str 6 2 1", "M[SP + 2] <- R[r1]");
  incSP();
}

void codeGenMinusExpr(ASTree *t, int classNumber, int methodNumber){
  codeGenBinaryExpr(t, classNumber, methodNumber);
  write("sub 1 1 2", "R[r1] <- R[r1] - R[r2]");
  write("str 6 2 1", "M[SP + 2] <- R[r1]");
  incSP();
}

void codeGenTimesExpr(ASTree *t, int classNumber, int methodNumber){
  codeGenBinaryExpr(t, classNumber, methodNumber);
  write("mul 1 1 2", "R[r1] <- R[r1] * R[r2]");
  write("str 6 2 1", "M[SP + 2] <- R[r1]");
  incSP();
}

void codeGenBinaryExpr(const ASTree *t, int classNumber, int methodNumber) {
  ASTList *binaryExprNode = t->children;
  codeGenExpr(binaryExprNode->data, classNumber, methodNumber);
  binaryExprNode = binaryExprNode->next;
  codeGenExpr(binaryExprNode->data, classNumber, methodNumber);
  write("lod 1 6 2", "R[r1] <- Value of expr1");
  write("lod 2 6 1", "R[r2] <- Value of expr2");
}