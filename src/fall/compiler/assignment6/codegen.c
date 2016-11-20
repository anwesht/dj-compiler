//
// Created by Anwesh Tuladhar on 11/20/16.
//
#include "codegen.h"
#include "symtbl.h"
#include <stdarg.h>
#define MAX_DISM_ADDR 65535

/* Global for the DISM output file */
FILE *fout;

/** Write to output file.
 * @param label => the label of DISM instruction
 * @param dismFormat => String representing the DISM instruction.
 * @param comment => String representing the DISM instruction comment.
 * @param ...
 */
//void _write(const char *label, const char* dismFormat, const char* comment, ...){
void _write(char *label, const char* dismFormat, const char* comment, va_list args){
//  fprintf(fout, "        mov 1 1    ;  R[r1] <- 1 (move immediate value)");
//  va_list args;
//  va_start(args, comment);
//  fprintf(fout, "        ");
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
  return labelNumber;
}

/* Print a message and exit under an exceptional condition */
void internalCGerror(char *msg);

/* Using the global classesST, calculate the total number of fields,
 including inherited fields, in an object of the given type */
int getNumObjectFields(int type);

/* Generate code that increments the stack pointer */
void incSP();

/* Generate code that decrements the stack pointer */
void decSP(){
//  fprintf(fout, "        mov 1 1    ;  R[r1] <- 1 (move immediate value)");
  write("mov 1, 1", "R[r1] <- 1 (move immediate value)");
  write("sub 6, 6", "SP--");
  write("blt 5, 6 %d", "Branch if HP < SP", getNewLabelNumber());
  write("mov 1, 77", "error code 77 => out of stack memory");
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
void codeGenExpr(ASTree *t, int classNumber, int methodNumber);

/* Generate DISM code for an expression list, which appears in
 the given class and method (or main block).
 If classNumber < 0 then methodNumber may be anything and we assume
 we are generating code for the program's main block. */
void codeGenExprs(ASTree *expList, int classNumber, int methodNumber);

/* Generate DISM code as the prologue to the given method or main
 block. If classNumber < 0 then methodNumber may be anything and we
 assume we are generating code for the program's main block. */
void genPrologue(int classNumber, int methodNumber);

/* Generate DISM code as the epilogue to the given method or main
 block. If classNumber < 0 then methodNumber may be anything and we
 assume we are generating code for the program's main block. */
void genEpilogue(int classNumber, int methodNumber);

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
  decSP();
}

