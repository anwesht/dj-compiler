//
// Created by Anwesh Tuladhar on 11/20/16.
//
#include "codegen.h"
#include "symtbl.h"
#include <stdarg.h>
#include <string.h>

#define MAX_DISM_ADDR 65535
#define RED    "\x1B[31m"
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
void codeGenIfThenElseExpr(const ASTree*, int, int);
void codeGenEqualityExpr(const ASTree*, int, int);
void codeGenGreaterThanExpr(const ASTree*, int, int);
void codeGenForExpr(const ASTree*, int, int);
void codeGenOrExpr(const ASTree*, int, int);
void codeGenNewExpr(const ASTree*);
void codeGenAssignExpr(const ASTree*, int, int);
void codeGenIdExpr(const ASTree*, int, int);
void codeGenDotAssignExpr(const ASTree *, int, int);
void codeGenDotIdExpr(const ASTree *, int, int);
void codeGenDotMethodCallExprs(const ASTree *, int, int);
void codeGenMethodCallExprs(const ASTree *, int, int);

void genPrologueMain(void);
void genEpilogueMain(void);
void genPrologue(int, int);
void genEpilogue(int, int);

void codeGenBinaryExpr(const ASTree *t, int classNumber, int methodNumber);

unsigned int getNewLabelNumber(void);
void setOrExprEscapeLabel(void);
void resetOrExprEscapeLabel(void);
int getOrExprEscapeLabel(void);
bool checkOrExprEscapeLabel(void);
int getOffsetOfVarInLocalsST(const VarDecl*, int, char *varName);
int getOffsetOfVarInClass(int, char *varName);


/* Global for the DISM output file */
FILE *fout;
unsigned int orExprEscapeLabel = 0;

void setOrExprEscapeLabel(){
  if(orExprEscapeLabel == 0) {
    orExprEscapeLabel = getNewLabelNumber();
  }
}

void resetOrExprEscapeLabel(){
  orExprEscapeLabel = 0;
}

int getOrExprEscapeLabel(){
  return orExprEscapeLabel;
}

bool checkOrExprEscapeLabel(){
  if(getOrExprEscapeLabel() == 0) {
      return true;
  } else {
    return false;
  }
}

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
unsigned int labelNumber = 1;

/* Get and increment label number */
unsigned int getNewLabelNumber() {
  return labelNumber++;
}

/* Get current value of label number*/
unsigned int getLabelNumber() {
  return labelNumber - 1;
}

/* Print a message and exit under an exceptional condition */
void _internalCGerror(char *msg){
  printf(RED"\nError generating code.>>> "NORMAL);
  printf("\n  %s\n", msg);
  printf("\n");
}

void internalCGerror(char *msg){
  _internalCGerror(msg);
  exit(-1);
}

/* Using the global classesST, calculate the total number of fields,
 including inherited fields, in an object of the given type */
int getNumObjectFields(int type);

/* Generate code that increments the stack pointer */
void incSP(){
  write("mov 1 1", "R[r1] <- 1 (move immediate value)");
  write("add 6 6 1", "SP++");
}

/* Generate code that increments the heap pointer by i */
void incHP(int i){
  write("mov 1 %d", "R[r1] <- Heap pointer increment value", i);
  write("add 5 5 1", "Increment heap pointer");
  write("blt 5 6 #%d", "Branch if HP < SP", getNewLabelNumber());
  write("mov 1 77", "error code 77 => out of stack memory");
  write("hlt 1", " out of stack memory! (SP < HP)");
  writeWithLabel("#%d: mov 0 0", "Landing for incHP", getLabelNumber());
}

/* Generate code that decrements the stack pointer */
//todo move the checking part to a static location using jump
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
      codeGenNewExpr(t);
      break;

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
      codeGenIdExpr(t, classNumber, methodNumber);
      break;

    case DOT_ID_EXPR:
      codeGenDotIdExpr(t, classNumber, methodNumber);
      break;

    case ASSIGN_EXPR:
      codeGenAssignExpr(t, classNumber, methodNumber);
      break;

    case EQUALITY_EXPR:
      codeGenEqualityExpr(t, classNumber, methodNumber);
      break;

    case OR_EXPR:
      codeGenOrExpr(t, classNumber, methodNumber);
      break;

    case GREATER_THAN_EXPR:
      codeGenGreaterThanExpr(t, classNumber, methodNumber);
      break;

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
      codeGenDotAssignExpr(t, classNumber, methodNumber);
      break;

    case METHOD_CALL_EXPR:
//      codeGenMethodCallExprs(t, classNumber, methodNumber);
//      break;

    case DOT_METHOD_CALL_EXPR:
      codeGenDotMethodCallExprs(t, classNumber, methodNumber);
      break;

    case IF_THEN_ELSE_EXPR:
      codeGenIfThenElseExpr(t, classNumber, methodNumber);
      break;

    case FOR_EXPR:
      codeGenForExpr(t, classNumber, methodNumber);
      break;

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
  printf("generating code Exprs class: %d, method: %d", classNumber, methodNumber);

  ASTList *currentNode = expList->children;
  while(true) {
    codeGenExpr(currentNode->data, classNumber, methodNumber);
    currentNode = currentNode->next;
    /* This saves the result of evaluating only the final expression of the list. */
    if(currentNode != NULL && currentNode->data != NULL){
      incSP();
    } else {
      break;
    }
  }
}

void genPrologueMain() {
  printf("generating main block prologue\n");
  write("mov 7 %d", "initialize FP", MAX_DISM_ADDR);
  write("mov 6 %d", "initialize SP", MAX_DISM_ADDR);
  write("mov 5 1", "initialize HP");
  write("mov 0 0", "ALLOCATE STACK SPACE FOR MAIN LOCALS");
  write("mov 0 0", "BEGIN METHOD/MAIN-BLOCK BODY");
  int i;
  for(i = 0; i < numMainBlockLocals; i += 1) {
//      write("mov 1 0", "Initializing Main Locals");
      write("str 6 0 0", "M[SP] <- R[r1]");
  }
  write("mov 1 %d", "R[r1] <- number of main block locals", numMainBlockLocals);
  write("sub 6 6 1", "Move SP after main locals");
  //todo need to check out of memory stuff.
}

/* Generate DISM code as the prologue to the given method or main
 block. If classNumber < 0 then methodNumber may be anything and we
 assume we are generating code for the program's main block. */
void genPrologue(int classNumber, int methodNumber) {
  MethodDecl currentMethod = classesST[classNumber].methodList[methodNumber];

  printf("genPrologue some class\n");
  /* 1. Store old frame pointer in stack.*/
  write("str 6 0 7", "M[SP] <- Value of old FP");

  /* 2. Update Frame pointer to new FP */
  write("mov 1 5", "R[r1] <- 5 (immediate value)");   //todo CHeck 5 vs 6 vs 7
  write("add 7 6 1", "R[r7] (FP) <- R[r6 (SP) + 5]");

  /* 3. Store all method locals */
  int i;
  int numLocals = currentMethod.numLocals;
  for(i = 0; i < numLocals; i += 1) {
//    write("mov 1 0", "Initializing Main Locals");
    write("str 6 -%d 0", "M[SP] <- R[r1]", ++i); // +1 for Old FP, after which we have not updated SP
  }
  /* Update SP */
  write("mov 1 %d", "R[r1] <- number of method locals", ++numLocals);
  write("sub 6 6 1", "Move SP after method locals");
}

/* Generate DISM code as the epilogue to the given method or main
 block. If classNumber < 0 then methodNumber may be anything and we
 assume we are generating code for the program's main block. */
void genEpilogue(int classNumber, int methodNumber){
  printf("gen Epilogue some class\n");
  MethodDecl currentMethod = classesST[classNumber].methodList[methodNumber];

  /* Load method result (i.e. current top of stack) */
  write("lod 1 6 1", "R[r1] <- M[SP + 1] (result of the method)");

  /* Restore stack pointer to current FP . */
  write("add 6 7 0", "R[r6](SP) <- R[r7] (FP)");

  /* Save return address to jump to later */
  write("lod 2 7 0", "R[r1] <- R[r7] (return address/FP)");
//  write("add 2 7 0", "R[r1] <- R[r7] (return address/FP)");

  /* Write result to return address */
  write("str 6 0 1", "M[SP] <- R[r1] (result of the method)");
  decSP();

  /* Restore FP (old FP should hold the address of OLD FP)*/
  write("lod 7 7 -5", "R[r7](FP) <- M[FP - 5] (Old FP)");
  write("ptn 2", "return address");
  /* Jump to return address */
  write("jmp 2 0", "jump to return address(R[r2])");
//  write("hlt 1", "jump to return address(R[r2])");
}

void genEpilogueMain() {
  printf("generating main block epilogue\n");
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
  genPrologueMain();
  codeGenExprs(mainExprs, -1, -1);
  genEpilogueMain();

  /* CodeGen all methods of all classes with label*/
  int i, j;
  printf("Num classes : %d", numClasses);
  for(i = 1; i < numClasses; i += 1) {
    ClassDecl currentClass = classesST[i];
    for(j = 0; j < currentClass.numMethods; j += 1) {
      MethodDecl currentMethod = currentClass.methodList[j];
      writeWithLabel("#c%dm%d: mov 0 0", "Class method landing", i, j);
      /* genPrologue */
      genPrologue(i, j);
      /* genBody */
      codeGenExprs(currentMethod.bodyExprs, i, j);
      /* genEpilogue */
      genEpilogue(i, j);
    }
  }
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

void codeGenIfThenElseExpr(const ASTree *t, int classNumber, int methodNumber) {
  ASTList *ifThenElseNode = t->children;
  int elseLabel = getNewLabelNumber();
  int endLabel = getNewLabelNumber();
  codeGenExpr(ifThenElseNode->data, classNumber, methodNumber);
  write("lod 1 6 1", "Load ifThenElse test expr");
  write("beq 1 0 #%d", "if R[r1] == 0 goto elseLabel", elseLabel);
  incSP();
  ifThenElseNode = ifThenElseNode->next;
  codeGenExprs(ifThenElseNode->data, classNumber, methodNumber);
  write("jmp 0 #%d", "Jump to endLabel", endLabel);
  writeWithLabel("#%d: mov 0 0", "elseLabel Landing", elseLabel);
  incSP();
  ifThenElseNode = ifThenElseNode->next;
  codeGenExprs(ifThenElseNode->data, classNumber, methodNumber);
  writeWithLabel("#%d: mov 0 0", "endLabel Landing", endLabel);
}

void codeGenEqualityExpr(const ASTree *t, int classNumber, int methodNumber) {
  codeGenBinaryExpr(t, classNumber, methodNumber);
  int trueLabel = getNewLabelNumber();
  int endLabel = getNewLabelNumber();
  write("beq 1 2 #%d", "If R[r1] = R[r2] then PC <- trueLabel", trueLabel);
  write("str 6 2 0", "M[SP] <- 0, i.e. False");
  write("jmp 0 #%d", "Jump to endLabel", endLabel);

  writeWithLabel("#%d: mov 0 0", "trueLabel Landing for equality expr", trueLabel);
  write("mov 1 1", "R[r1] <- 1 (move immediate value)");
  write("str 6 2 1", "M[SP+2] <- 1, i.e. True");
  writeWithLabel("#%d: mov 0 0", "endLabel Landing for equality test", endLabel);
  incSP();
}

void codeGenGreaterThanExpr(const ASTree *t, int classNumber, int methodNumber) {
  codeGenBinaryExpr(t, classNumber, methodNumber);
  int falseLabel = getNewLabelNumber();
  int endLabel = getNewLabelNumber();
  // R[r1] > R[r2] ==> R[r1] < R[r2] and R[r1] == R[r2] = false
  write("blt 1 2 #%d", "If R[r1] < R[r2] then PC <- falseLabel", falseLabel);
  write("beq 1 2 #%d", "If R[r1] == R[r2] then PC <- falseLabel", falseLabel);
  write("mov 1 1", "R[r1] <- 1 (move immediate value)");
  write("str 6 2 1", "M[SP] <- 1, i.e. True");
  write("jmp 0 #%d", "Jump to endLabel", endLabel);

  writeWithLabel("#%d: mov 0 0", "falseLabel Landing for greater than expr", falseLabel);
  write("str 6 2 0", "M[SP+2] <- 0, i.e. False");

  writeWithLabel("#%d: mov 0 0", "endLabel Landing for greater than test", endLabel);
  incSP();
}

void codeGenForExpr(const ASTree *t, int classNumber, int methodNumber) {
  ASTList *forNode = t->children;
  int loopLabel = getNewLabelNumber();
  int loopEndLabel = getNewLabelNumber();
  /* CodeGen loop initializer */
  codeGenExpr(forNode->data, classNumber, methodNumber);
//  write("lod 1 6 1", "Load loop initializer");
  //todo: incSp() ??

  writeWithLabel("#%d: mov 0 0", "Loop Label.", loopLabel);
  /* CodeGen loop test */
  forNode = forNode->next;
  codeGenExpr(forNode->data, classNumber, methodNumber);
  write("lod 1 6 1", "R[r1] <- loop test outcome");
  write("beq 1 0 #%d", "Loop test failed. Goto end of loop.", loopEndLabel);

  /* CodeGen loop body */
  ASTList *forExprBodyNode = t->childrenTail;
  codeGenExprs(forExprBodyNode->data, classNumber, methodNumber);

  //incSP() => result of loop body not required any more??
  incSP();
  /* CodeGen loop update */
  forNode = forNode->next;
  codeGenExpr(forNode->data, classNumber, methodNumber);
  write("jmp 0 #%d", "Jump to start of loop", loopLabel);

  writeWithLabel("#%d: mov 0 0", "End of loop landing", loopEndLabel);
}

void codeGenOrExpr(const ASTree *t, int classNumber, int methodNumber) {
  ASTList *orExprNode = t->children;
  bool createEscapeLabel = checkOrExprEscapeLabel();
  /* Set the escape label*/
  setOrExprEscapeLabel();
  codeGenExpr(orExprNode->data, classNumber, methodNumber);

  write("lod 1 6 1", "R[r1] <- e1 of or expr");
  write("blt 0 1 #%d", "Short Circuit OR if true.(i.e. if R[r0] < M[SP + 1])", getOrExprEscapeLabel());

  //If branch not taken, the result of expr1 is not used.???
  incSP();

  orExprNode = orExprNode->next;
  codeGenExpr(orExprNode->data, classNumber, methodNumber);

  if(createEscapeLabel == true){
    writeWithLabel("#%d: mov 0 0", "Escape label for short circuit or expr", getOrExprEscapeLabel());
    resetOrExprEscapeLabel();
  }
}

int getNumberOfFieldsInClass(int currentClassNum) {
  ClassDecl currentClass = classesST[currentClassNum];
  int numFields = currentClass.numVars;

  if(currentClass.superclass > 0) {   // Look for the variable in all classes
    numFields += getNumberOfFieldsInClass(currentClass.superclass);
  }
  return numFields;
}

/**
 * Top of stack = address of new object/pointer to new object
 * @param t
 */
void codeGenNewExpr(const ASTree *t) {
  ASTList *newNode = t->children;
  int newClassNumber = classNameToNumber(newNode->data->idVal);
  int numFields = getNumberOfFieldsInClass(newClassNumber);
  printf("number of fields in current class is: %d \n", numFields);
  int i;
  for(i = 0; i < numFields; i += 1){
    write("str 5 %d 0", "Reserve location for field", i);
  }
  write("mov 1 %d", "R[r1] <- class type tag", newClassNumber);
  write("str 5 %d 1", "Class type tag", numFields);
  incHP(numFields+1);
  write("mov 1 1", "R[r1] <- immediate value 1");
  write("sub 1 5 1", "R[r1] <- M[HP - 1]");
  write("str 6 0 1", "M[SP] <- Pointer to new object");
  //debug print.
//  write("ptn 1", "debug: pointer to new object");
  decSP();
}

void codeGenAssignExpr(const ASTree *t, int classNumber, int methodNumber){
  ASTList *assignNode = t->children;
  char *varName = assignNode->data->idVal;
  int varOffset = -1; //offset to the variable from FP
  assignNode = assignNode->next;
  codeGenExpr(assignNode->data, classNumber, methodNumber);
  /* This is a main variable. */
  if(classNumber < 0) {
    varOffset = getOffsetOfVarInLocalsST(mainBlockST, numMainBlockLocals,varName);
    write("lod 1 6 1", "R[r1] <- rvalue of RHS of assign expr(M[SP + 1]])");
    write("str 7 -%d 1", "M[FP - varOffset] <- R[r1] (rvalue of RHS of assign expr)", varOffset);
    //debug
//    write("ptn 1", "debug: rvalue of RHS of assign expr.");

  } else {
    printf("local in class!!!");
//    write("ptn 7", "debug: local in call. FP");
    MethodDecl currentMethod = classesST[classNumber].methodList[methodNumber];
    /* Check if varName is a param */
    if(strcmp(varName, currentMethod.paramName) == 0){
      varOffset = 4;
    }
    /* Check if varName in method locals */
    if(varOffset == -1){
      varOffset = getOffsetOfVarInLocalsST(currentMethod.localST, currentMethod.numLocals,varName);
      varOffset += 5;
    }

    /* The variable is in the stack. */
    if(varOffset != -1) {
      write("lod 1 6 1", "R[r1] <- rvalue of RHS of assign expr(M[SP + 1]])");
      write("str 7 -%d 1", "M[FP - varOffset] <- R[r1] (rvalue of RHS of assign expr)", varOffset);
    } else {
    /* Check if varName in fields of class. Variable is in the heap*/
      varOffset = getOffsetOfVarInClass(classNumber, varName);
      /* Load address of this object(i.e. the dynamic caller object) */
      write("lod 1 7 -1", "R[r1] <- M[FP -1] (address of e");
      /* Load the RHS value */
      write("lod 2 6 1", "R[r2] <- rvalue of assign expr(M[SP + 1]])");
      // First member has number = 0. need to offset +1.
      write("str 1 -%d 2", "M[addr of obj - varOffset] <- R[r1] (rvalue of RHS of dot assign expr)", ++varOffset);
    }
  }
//  decSP();
}

int getOffsetOfVarInClass(int currentClassNum, char *varName){
  ClassDecl currentClass = classesST[currentClassNum];
  int i;
  VarDecl *varList = currentClass.varList;
  for(i = 0; i < currentClass.numVars; i += 1) {
    if(strcmp(varName, varList[i].varName) == 0) {
      printf("var offset in class is: %d \n", i);
      return i;
    }
  }
  if(currentClass.superclass > 0) {
    return getOffsetOfVarInClass(currentClass.superclass, varName);
  } else {
    _internalCGerror("Var not found in any class.");
    exit(-1);
  }
}

/**
 * Find the offset to a variable name in the provided symbol table.
 * @param varList => The symbol table in which to search
 * @param numVars => The number of variables in the symbol table
 * @param varName => The name of the variable to look for.
 * @return => offset to the variable in the symbol table.
 * @throws => Local vaiable not found error (Should not happen as AST already type checked.)
 */
int getOffsetOfVarInLocalsST(const VarDecl *varList, int numVars, char *varName) {
  int i;
  printf("search for var: %s\n", varName);
  printf("numVars : %d\n", numVars);

  for(i = 0; i < numVars; i += 1) {
    printf("current var: %s\n", varList[i].varName);
    if(strcmp(varName, varList[i].varName) == 0) {
      printf("found matching var.\n");
      return i;
    }
  }
  /*_internalCGerror("Local Variable not found");
  exit(-1);*/
  return -1;
}

void codeGenIdExpr(const ASTree *t, int classNumber, int methodNumber) {
  ASTList * idExprNode = t->children;
  char *varName = idExprNode->data->idVal;
  int varOffset = -1;
  if(classNumber < 0){
    varOffset = getOffsetOfVarInLocalsST(mainBlockST, numMainBlockLocals, varName);
    if(varOffset == -1) {
      _internalCGerror("Var not found.");
      exit(-1);
    }
    write("lod 1 7 -%d", "R[r1] <- rvalue of variable", varOffset);
    write("str 6 0 1", "M[SP] <- R[r1] (rvalue of variable)");
    //debug
//    write("ptn 1", "debug: rvalue of variable");
  } else {
    printf("id expr in class!!!");
//    write("ptn 7", "debug: id expr in class. FP");
    printf("local in class!!!\n");
//    write("ptn 7", "debug: local in call. FP");
    MethodDecl currentMethod = classesST[classNumber].methodList[methodNumber];
    /* Check if varName is a param */
    printf("Var name is: %s", varName);
    printf(" name is: %s", currentMethod.paramName);
    if(strcmp(varName, currentMethod.paramName) == 0){
      varOffset = 4;
    }
    /* Check if varName in method locals */
    if(varOffset == -1){
      varOffset = getOffsetOfVarInLocalsST(currentMethod.localST, currentMethod.numLocals,varName);
      varOffset += (varOffset != -1) ? 5 : 0;   // 5 fields added in stack for method frames
    }

    /* The variable is in the stack. */
    if(varOffset != -1) {
      write("lod 1 7 -%d", "R[r1] <- rvalue of variable (M[FP - offset])", varOffset);
      write("str 6 0 1", "M[SP] <- R[r1] (rvalue of variable)", varOffset);
    } else {
      /* Check if varName in fields of class. Variable is in the heap*/
      varOffset = getOffsetOfVarInClass(classNumber, varName);
      /* Load address of this object(i.e. the dynamic caller object) */
      write("lod 1 7 -1", "R[r1] <- M[FP -1] (address of e");
      /* Load the RHS value */
      // First member has number = 0. need to offset +1.
      write("lod 2 1 -%d", "M[addr of obj - varOffset] <- rvalue of field", ++varOffset);
      /* Store to top of stack*/
      write("str 6 0 2", "M[SP] <- R[r2] (rvalue of field)");
    }
  }
  decSP();
}

/*void codeGenIdExprLvalue(const ASTree *t, int classNumber, int methodNumber) {
  ASTList * idExprNode = t->children;
  char *varName = idExprNode->data->idVal;
  int varOffset = -1;
  if(classNumber < 0){
    varOffset = getOffsetOfVarInLocalsST(mainBlockST, numMainBlockLocals, varName);
    if(varOffset == -1) {
      _internalCGerror("Var not found.");
      exit(-1);
    }
//    write("lod 1 7 -%d", "R[r1] <- rvalue of variable", varOffset);
    write("sub 1 7 %d", "Calculate lvalue of variable", varOffset);
    write("str 6 0 1", "M[SP] <- (lvalue of variable)");
    //debug
//    write("ptn 1", "debug: rvalue of variable");
  } else {
    printf("id expr in class!!!");
//    write("ptn 7", "debug: id expr in class. FP");
    printf("local in class!!!\n");
//    write("ptn 7", "debug: local in call. FP");
    MethodDecl currentMethod = classesST[classNumber].methodList[methodNumber];
    *//* Check if varName is a param *//*
    printf("Var name is: %s", varName);
    printf(" name is: %s", currentMethod.paramName);
    if(strcmp(varName, currentMethod.paramName) == 0){
      varOffset = 4;
    }
    *//* Check if varName in method locals *//*
    if(varOffset == -1){
      varOffset = getOffsetOfVarInLocalsST(currentMethod.localST, currentMethod.numLocals,varName);
      varOffset += (varOffset != -1) ? 5 : 0;   // 5 fields added in stack for method frames
    }

    *//* The variable is in the stack. *//*
    if(varOffset != -1) {
//      write("lod 1 7 -%d", "R[r1] <- rvalue of variable (M[FP - offset])", varOffset);
      write("sub 1 7 %d", "Calculate lvalue of variable", varOffset);
      write("str 6 0 1", "M[SP] <- R[r1] (rvalue of variable)", varOffset);
    } else {
      *//* Check if varName in fields of class. Variable is in the heap*//*
      varOffset = getOffsetOfVarInClass(classNumber, varName);
      *//* Load address of this object(i.e. the dynamic caller object) *//*
      write("lod 1 7 -1", "R[r1] <- M[FP -1] (address of e");
      *//* Load the RHS value *//*
      // First member has number = 0. need to offset +1.
      write("lod 2 1 -%d", "M[addr of obj - varOffset] <- rvalue of field", ++varOffset);
      *//* Store to top of stack*//*
      write("str 6 0 2", "M[SP] <- R[r2] (rvalue of field)");
    }
  }
  decSP();
}*/

void codeGenDotAssignExpr(const ASTree *t, int classNumber, int methodNumber) {
  /* Right associative. CodeGen RHS first */
  ASTList *dotAssignNode = t->childrenTail;

  codeGenExpr(dotAssignNode->data, classNumber, methodNumber);  //Top of stack = rvalue of RHS

  dotAssignNode = t->children;

  codeGenExpr(dotAssignNode->data, classNumber, methodNumber);  //Top of stack = address of e1.

  //use static class num??
  int staticClassNum = t->staticClassNum;
  int staticMemberNum = t->staticMemberNum;

  /* Get type of object */
  write("lod 1 6 1", "R[r1] <- address of e");
  write("lod 2 6 2", "R[r2] <- rvalue of RHS");
  // First member has number = 0. need to offset +1.
  write("str 1 -%d 2", "M[addr of obj - varOffset] <- R[r1] (rvalue of RHS of dot assign expr)", ++staticMemberNum);
  incSP();  // Top of stack now has rvalue of RHS.
  //debug
//  write("ptn 1", "debug: addr of LHS obj.");
//  write("ptn 2", "debug: rvalue of RHS of dot assign expr.");
}

void codeGenDotIdExpr(const ASTree *t, int classNumber, int methodNumber){
  ASTList *dotIdNode = t->children;
  codeGenExpr(dotIdNode->data, classNumber, methodNumber);  //Top of stack = address of e1.
  int staticMemberNum = t->staticMemberNum;
  write("lod 1 6 1", "R[r1] <- address of e");
  write("lod 1 1 -%d", "R[r2] <- M[addr of obj - varOffset] (rvalue of member)", ++staticMemberNum);
  write("str 6 1 1", "M[SP+1] <- R[r1] (rvalue of member)");
  // No need to decSP()
}

void codeGenDotMethodCallExprs(const ASTree *t, int classNumber, int methodNumber) {
  ASTList *dotMethodCallNode = t->children;
  int returnLabel = getNewLabelNumber();
  printf("return label is: %d", returnLabel);
  /* 1. Push #returnLabel to stack */
  write("mov 1 #%d", "R[r1] <- #returnLabel", returnLabel);
//  write("ptn 1", "debug: return address");
  write("str 6 0 1", "push #returnLabel to stack");

  decSP();
  /* 2. Push the dynamic calling object's address to stack*/
  /* Note: if new expr => top of stack is address in heap
   *       if id expr => it has to be obj type, therefore rvalue = address of obj in heap */
  codeGenExpr(dotMethodCallNode->data, classNumber, methodNumber);  //top of stack = dynamic calling obj.


  /*todo Check dynamic caller obj is not null */
  // checkNullDereference();

  int staticClassNum = t->staticClassNum;
  int staticMemberNum = t->staticMemberNum;
  /* 3. Push static class number to stack */
  write("mov 1 %d", "R[r1] <- static class number", staticClassNum);
  write("str 6 0 1", "M[SP] <- R[r1] (static class number)");

  decSP();

  /* 4. Push static member number to stack */
  write("mov 1 %d", "R[r1] <- static method number", staticMemberNum);
  write("str 6 -1 1", "M[SP] <- R[r1] (static class number)");

  decSP();
  /* codeGen method argument */
  dotMethodCallNode = t->childrenTail;

  /* 5. Push dynamic incoming argument to stack */
  codeGenExpr(dotMethodCallNode->data, classNumber, methodNumber);

  //todo update SP by num.
//  write("mov 1 4", "R[r1] <- 4"); // from nos. 2 to 5 inclusive = 4 places to shift
//  write("sub 6 6 1", "Move SP after static info");

  /*todo call dynamic dispatcher/vtable */
  // call dispatcher
  write("jmp 0 #c%dm%d", "jump to method.", staticClassNum, staticMemberNum);
  writeWithLabel("#%d: mov 0 0", "Return label landing for method", returnLabel);

}