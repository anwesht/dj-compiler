/**
 * Created by atuladhar on 11/7/16.
 * Pledge: I pledge my Honor that I have not cheated, and will not cheat, on this assignment
 * Name: Anwesh Tuladhar
 */

#include <stdio.h>
#include <string.h>
#include "symtbl.h"

#define RED   "\x1B[31m"
#define NORMAL "\x1B[0m"


typedef enum
{
	false,
	true
} bool;

int isSubtype(int, int);
int join(int, int);
void checkUniqueClassNames(void);
void validateClasses(void);
void validateClassTypes(int, ClassDecl);
void validateVarListTypes(ClassDecl, VarDecl*, int);
void validateMethodListTypes(ClassDecl);

static void throwError(char *message, int errorLine) {
  printf(RED"\nERROR >>> "NORMAL);
  printf("%s\n", message);
  printf("  Error in Line Number: %d", errorLine);
  printf("\n");
  exit(-1);
}

void typecheckProgram(){
  printf("\n\n########################\n");
  printf("Type Checking Program\n");
  printf("########################\n");
  int i;
  for(i = 1 ; i < numClasses; i += 1) {
    printf("is Sub Type %s  = %d\n" ,classesST[i].className, isSubtype(i, 2));
  }
  printf("Is Subtype: %d\n",isSubtype(-1, -1));
  printf("Is Subtype: %d\n",isSubtype(-1, 0));
  printf("Is Subtype: %d\n",isSubtype(-2, 0));
  printf("Is Subtype: %d\n",isSubtype(-2, 1));
  printf("Is Subtype: %d\n",isSubtype(-2, 2));
  printf("Is Subtype: %d\n",isSubtype(-2, -1));

  printf("join 3 and 4: %d\n", join(3, 4));

  validateClasses();
}

/** Checks if sub is a subtype of super
  * @param sub => param representing subtype
  * @param super =>  param representing supertype
  * @returns => bool representing true or false
  */
int isSubtype(int sub, int super) {
  bool isSubtype = false;
  if(sub <= -3) return isSubtype;

  switch(sub) {
    case -2:
      if((super == -2 || 0) || super >= 0 ) {
        isSubtype = true;
      }
      break;

    case -1:
      if(super == -1) isSubtype = true;
      break;

    case 0:
      if(super == 0) isSubtype = true;
      break;

    default:
    {
      int currentClass = sub;
      int i;
      for(i = 0; i < numClasses; i += 1) {
        if((currentClass = classesST[currentClass].superclass) == super) {
          isSubtype = true;
          break;
        }
      }
    }
  }
  return isSubtype;
}

/** Finds the Least Upper bound of two classes\
  * @param t1 => type1 to join
  * @param t2 => type2 to join
  * @returns => The Least Upper Bound/join of t1 and t2
  */
int join(int t1, int t2) {
  if(isSubtype(t1, t2)) {
    return t2;
  } else if (isSubtype(t2, t1)) {
    return t1;
  } else {
    return join(classesST[t1].superclass, t2);
  }
}

/** Checks for uniqueness of all defined classes
  * @throws => Class redefined error.
  */
void validateClasses() {
  int i, j;
  for(i = 1; i <= numClasses; i += 1) {
    ClassDecl currentClass = classesST[i];
    for(j = i + 1; j <= numClasses; j += 1) {
      if(strcmp(currentClass.className, classesST[j].className) == 0){
        printf("Class %s is already defined in line: %d.", currentClass.className, currentClass.classNameLineNumber);
        throwError("Class redefined.", classesST[j].classNameLineNumber);
      }
    }
    validateClassTypes(i, currentClass);
  }
}

/** Checks if all the 'types' used within a class is valid.
  * Performs the following checks:
    1. Super class types are valid classes.
    2. Class type is not the same as super class type
    3. Validates the types of all fields of a class
    4. Validates all method locals and return types
  * @param classDecl => The Class to verify.
  * @throws => Invalid Type Error
  */
void validateClassTypes(int classNum, ClassDecl classDecl) {
  /* 1. Validate Super class type */
  if(classDecl.superclass < 0) {
    throwError("Invalid Type Error", classDecl.superclassLineNumber);
  }
  /* 2. Validate class doesn't extend itself */
  if(classNum == classDecl.superclass) {
    throwError("Class Extends Itself.", classDecl.superclassLineNumber);
  }
  /* 3. Validate the types of all fields of a class */
//  validateVarListTypes(classDecl);
  validateVarListTypes(classDecl, classDecl.varList, classDecl.numVars);
  /* 4. Validate all method locals and return types */
  validateMethodListTypes(classDecl);
}

void validateVarListTypes(ClassDecl classDecl, VarDecl *varList, int numVars) {
  /*VarDecl *varList = classDecl.varList;
  int numVars = classDecl.numVars;*/
  int i;
  for(i = 0; i < numVars; i += 1) {
    if(varList[i].type < -1) {
      printf("Variable %s has unknown type.", varList[i].varName);
      throwError("Invalid Type Error.", varList[i].typeLineNumber);
    }
    // todo: check unique variable name
  }
}

void validateMethodListTypes(ClassDecl classDecl) {
  MethodDecl *methodList = classDecl.methodList;
  int numMethods = classDecl.numMethods;
  int i;
  for(i = 0; i < numMethods; i += 1) {
    /* Check return type */
    if(methodList[i].returnType < -1) {
      throwError("Invalid Type Error.", methodList[i].returnTypeLineNumber);
    }
    /* todo: Check unique method name */
    
    /* Check param type */
    if(methodList[i].paramType < -1) {
      throwError("Invalid Type Error.", methodList[i].paramTypeLineNumber);
    }

    /* todo: Check param name and local variable names are unique. */

    /* Check types of all locals. */
    validateVarListTypes(classDecl, methodList[i].localST, methodList[i].numLocals);
  }
}

int typeExpr(ASTree *t, int classContainingExpr, int methodContainingExpr);

int typeExprs(ASTree *t, int classContainingExprs, int methodContainingExprs);

