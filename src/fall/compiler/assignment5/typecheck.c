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
void validateClasses(void);
void validateClassTypes(int, ClassDecl);
void validateClassVarListTypes(ClassDecl, VarDecl*, int);
void validateVarListTypes(VarDecl*, int);
void validateMethodListTypes(int, ClassDecl);
void validateVarNamesInSuperClasses(ClassDecl);
void validateVarNameInSuperClasses(ClassDecl, char*);
void validateMethodOverride(ClassDecl, MethodDecl);
void validateDAG(ClassDecl, int);

int typeExpr(ASTree *t, int, int);
int typeExprs(ASTree *t, int, int);
int typeAssignExpr(ASTree*, int, int);
int getTypeOfVarInLocalsST(VarDecl*, int, char*);
int getTypeOfVarInClass(ClassDecl, char*);
int typeId(ASTree*, int, int);
int typeNewExpr(ASTree*);

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

  /* Validate types of all classes */
  validateClasses();

  /* Validate Main var declarations */
  validateVarListTypes(mainBlockST, numMainBlockLocals);

  /* Validate Main Expr List */
  typeExprs(mainExprs, -1, -1);
}

/** Checks if sub is a subtype of super
  * @param sub => param representing subtype
  * @param super =>  param representing supertype
  * @returns => bool representing true or false
  */
int isSubtype(int sub, int super) {
  bool isSubtype = false;
  if(sub <= -3) return isSubtype;
  if(sub == super) return true;
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
    3. Validates Class Hierarchy is a DAG
    4. Validates the types of all fields of a class
    5. Validates all method locals and return types
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
  /* 3. Validate DAG */
  validateDAG(classDecl, 0);
  /* 4. Validate the types of all fields of a class */
  validateClassVarListTypes(classDecl, classDecl.varList, classDecl.numVars);
  /* 5. Validate all method locals and return types */
  validateMethodListTypes(classNum, classDecl);
}

/** Checks for cycles in the class hierarchy
  * @param classDecl => class to validate the hierarchy
  * @param distanceToSuper => distance to the super classes
  * @throws => Cyclic Class Hierarchy Error
  */
void validateDAG (ClassDecl classDecl, int distanceToSuper) {
  if(classDecl.superclass == 0) return;  //Reached the summit. is acyclic.
  if(distanceToSuper >= numClasses) {
    throwError("Class Hierarchy is cyclic.", classDecl.classNameLineNumber);
  }
  distanceToSuper += 1;
  validateDAG(classesST[classDecl.superclass], distanceToSuper);
}

void validateClassVarListTypes(ClassDecl classDecl, VarDecl *varList, int numVars) {
  validateVarListTypes(varList, numVars);
  /* Check variable names in super class. no overriding fields. */
  validateVarNamesInSuperClasses(classDecl);    //extra loop through var list.
}

void validateVarNamesInSuperClasses(ClassDecl classDecl) {
  int i;
  VarDecl *varList = classDecl.varList;
  for(i = 0; i < classDecl.numVars; i += 1) {
    validateVarNameInSuperClasses(classesST[classDecl.superclass], varList[i].varName);
  }
}

/** Recursively checks for varName in super classes.
  * We have already verified that the class hierarchy is a DAG, so no infinite loops will occur.
  * @param superClass => The super class in which to check for varName
  * @param varName => The variable name in current class to which to look for.
  * @throws => Variable redefined error.
  */
void validateVarNameInSuperClasses(ClassDecl superClass, char *varName) {
  int i;
  VarDecl *varList = superClass.varList;
  for(i = 0; i < superClass.numVars; i += 1) {
    if(strcmp(varName, varList[i].varName) == 0){
      printf("Variable %s is already defined in Super class in line: %d.", varName,
       varList[i].varNameLineNumber);
      throwError("Variable redefined.", varList[i].varNameLineNumber);
    }
  }
  if(superClass.superclass > 0) {   // Look for the variable in all classes
    validateVarNameInSuperClasses(classesST[superClass.superclass], varName);
  }
}

void validateVarListTypes(VarDecl *varList, int numVars) {
  /*VarDecl *varList = classDecl.varList;
  int numVars = classDecl.numVars;*/
  int i, j;
  for(i = 0; i < numVars; i += 1) {
    VarDecl currentVar = varList[i];
    if(currentVar.type < -1) {
      printf("Variable %s has unknown type.", currentVar.varName);
      throwError("Invalid Type Error.", currentVar.typeLineNumber);
    }
    /* Check unique variable name */
    for(j = i + 1; j < numVars; j += 1) {
      if(strcmp(currentVar.varName, varList[j].varName) == 0){
        printf("Variable %s is already defined in line: %d.", currentVar.varName,
         currentVar.varNameLineNumber);
        throwError("Variable redefined.", varList[j].varNameLineNumber);
      }
    }
  }
}

void validateMethodListTypes(int classNum, ClassDecl classDecl) {
  MethodDecl *methodList = classDecl.methodList;
  int numMethods = classDecl.numMethods;
  int i, j;
  for(i = 0; i < numMethods; i += 1) {
    MethodDecl currentMethod = methodList[i];
    /* Check return type */
    if(methodList[i].returnType < -1) {
      throwError("Invalid Type Error.", currentMethod.returnTypeLineNumber);
    }

    /* Check unique method name */
    for(j = i + 1; j < numMethods; j += 1) {
      if(strcmp(currentMethod.methodName, methodList[j].methodName) == 0){
        printf("Method %s is already defined in line: %d.", currentMethod.methodName,
         currentMethod.methodNameLineNumber);
        throwError("Method redefined.", methodList[j].methodNameLineNumber);
      }
    }

    /* Check param type */
    if(currentMethod.paramType < -1) {
      throwError("Invalid Type Error.", currentMethod.paramTypeLineNumber);
    }

    /* Check types of all locals. */
    validateClassVarListTypes(classDecl, currentMethod.localST, currentMethod.numLocals);

    /* Check param name and local variable names are unique. */
    for(j = 0; j < currentMethod.numLocals; j += 1) {
      if(strcmp(currentMethod.paramName, currentMethod.localST[j].varName) == 0){
        printf("Param name %s is same as variable in line: %d.", currentMethod.paramName,
         currentMethod.localST[j].varNameLineNumber);
        throwError("Variable redefined.", currentMethod.localST[j].varNameLineNumber);
      }
    }

    /* Check for method override */
    validateMethodOverride(classesST[classDecl.superclass], currentMethod);

    /* Validate Method Expr List */
    typeExprs(currentMethod.bodyExprs, classNum, i);
  }
}

void validateMethodOverride(ClassDecl superClass, MethodDecl currentMethodDecl) {
  int i;
  MethodDecl *methodList = superClass.methodList;
  for(i = 0; i < superClass.numMethods; i += 1) {
    MethodDecl superMethod = methodList[i];
    if(strcmp(currentMethodDecl.methodName, superMethod.methodName) == 0) { //method overriden
      if(superMethod.returnType != currentMethodDecl.returnType) {
        printf("Overriding method %s in line %d", superMethod.methodName, superMethod.returnTypeLineNumber);
        throwError("Overridden method has different return type.", currentMethodDecl.returnTypeLineNumber);
      }
      if(superMethod.paramType != currentMethodDecl.paramType) {
        printf("Overriding method %s in line %d", superMethod.methodName, superMethod.paramTypeLineNumber);
        throwError("Overridden method has different param type.", currentMethodDecl.paramTypeLineNumber);
      }
    }
  }
  if(superClass.superclass > 0) {   // Look for method override in super
    validateMethodOverride(classesST[superClass.superclass], currentMethodDecl);
  }
}

int typeExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  if(t == NULL) throwError("Nothing to type check.", -1);
  switch(t->typ) {
    case NAT_LITERAL_EXPR:
      printf("Type NatLiteral Expr: %d | natVal = %d | in line number: %u\n", t->typ, t->natVal, t->lineNumber );
      return -1;

    case THIS_EXPR:
      printf("Type This Expr: classContainingExpr: %d | natVal = %d | in line number: %u\n", classContainingExpr, t->natVal, t->lineNumber );
      if(classContainingExpr <= 0) {
        throwError("Use of this is not allowed here.", t->lineNumber);
      }
      return classContainingExpr;

    case NEW_EXPR:
      return typeNewExpr(t->children->data);

    case READ_EXPR:
      return -1;

    case PRINT_EXPR:
      return -1;

    case NULL_EXPR:
      return -2;

    case ASSIGN_EXPR:
      return typeAssignExpr(t, classContainingExpr, methodContainingExpr);

    case ID_EXPR:
    {
      ASTree *astId = t->children->data;
      return typeExpr(astId, classContainingExpr, methodContainingExpr);
    }
    case AST_ID:
      return typeId(t, classContainingExpr, methodContainingExpr);

    default:
      if(t->idVal == NULL) {
        printf("Type Checking: %d | natVal = %d | in line number: %u\n", t->typ, t->natVal, t->lineNumber );
      } else {
        printf("Type Checking: %d | idVal = %s | in line number: %u\n", t->typ, t->idVal, t->lineNumber );
      }
      return 0;
  }
}

int typeExprs(ASTree *t, int classContainingExprs, int methodContainingExprs) {
  ASTList *currentNode = t->children;
  while(currentNode != NULL && currentNode->data != NULL) {
    typeExpr(currentNode->data, classContainingExprs, methodContainingExprs);
    currentNode = currentNode->next;
  }
  return -1;
}

int typeNewExpr(ASTree *astClassName) {
  int typeOfNew;
  if(astClassName->typ == OBJ_TYPE) {
    typeOfNew = 0;
  } else {
    typeOfNew = classNameToNumber(astClassName->idVal);
  }
  if(typeOfNew == -3) {
    throwError("Invalid Type for new expression", astClassName->lineNumber);
  }
  printf("Type New Expr: %d | id = %s | in line number: %u\n", typeOfNew, astClassName->idVal, astClassName->lineNumber );
  return typeOfNew;
}

int typeId(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  char *varName = t->idVal;
  if(classContainingExpr == 0) {
    throwError("Internal Compiler error. Class Containing Expr cannot be 0", t->lineNumber);
  }

  int typeOfId;
  if(classContainingExpr < 0) {
    /* Find varName in main locals*/
    typeOfId = getTypeOfVarInLocalsST(mainBlockST, numMainBlockLocals, varName);
    if(typeOfId == -3) {
      printf("Implicit declaration of variable '%s' in Main.", varName);
      throwError("Implicit Declaration of variable is not allowed in DJ.", t->lineNumber);
    }
    return typeOfId;
  } else {
    MethodDecl currentMethod = classesST[classContainingExpr].methodList[methodContainingExpr];
    /* Check if varName is param */
    if(strcmp(varName, currentMethod.paramName) == 0){
      typeOfId = currentMethod.paramType;
      return typeOfId;
    }
    /* Find varName in method locals */
    typeOfId = getTypeOfVarInLocalsST(currentMethod.localST, currentMethod.numLocals, varName);
    if(typeOfId > -3){
      return typeOfId;
    }
    /* Find varName in class/super class fields */
    typeOfId = getTypeOfVarInClass(classesST[classContainingExpr], varName);
    if(typeOfId == -3) {
      printf("Implicit declaration of variable '%s' in Method.", varName);
      throwError("Implicit Declaration of variable is not allowed in DJ.", t->lineNumber);
    }
    return typeOfId;
  }
}

int getTypeOfVarInClass(ClassDecl currentClass, char *varName) {
  int i;
  VarDecl *varList = currentClass.varList;
  for(i = 0; i < currentClass.numVars; i += 1) {
    if(strcmp(varName, varList[i].varName) == 0){
      return varList[i].type;
    }
  }
  if(currentClass.superclass > 0) {   // Look for the variable in all classes
    return getTypeOfVarInClass(classesST[currentClass.superclass], varName);
  }
  return -3;
}

int getTypeOfVarInLocalsST(VarDecl *varList, int numVars, char *varName) {
  int i;
  for(i = 0; i < numVars; i += 1) {
    if(strcmp(varName, varList[i].varName) == 0){
      return varList[i].type;
    }
  }
  printf("%s Variable not found in locals.\n", varName);
  return -3;
}

int typeAssignExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  ASTList *assignNode = t->children;
  int typeLhs = typeExpr(assignNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of LHS = %d\n", typeLhs);
  assignNode = assignNode->next;
  int typeRhs = typeExpr(assignNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of RHS = %d\n", typeRhs);
  printf("%d", isSubtype(typeRhs, typeLhs));
  if(!isSubtype(typeRhs, typeLhs)) {
    throwError("Error is Assignment. RHS is not a sub-type of LHS.", assignNode->data->lineNumber);
  }
  printf("type of assignment is: %d\n", typeLhs);
  return typeLhs;
}


