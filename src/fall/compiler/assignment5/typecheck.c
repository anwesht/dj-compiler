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
int typeCompExpr(ASTree*, int, int);
int typeBinaryExpr(ASTree*, int, int);
int typeDotIdExpr(ASTree*, int, int);
int typeDotAssignExpr(ASTree*, int, int);
int typeNotExpr(ASTree*, int, int);
int typeMethodCallExpr(ASTree*, int, int);
int typeDotMethodCallExpr(ASTree*, int, int);
int typeIfThenElseExpr(ASTree*, int, int);
int typeForExpr(ASTree*, int, int);

MethodDecl getMethodDeclInClass(ClassDecl, char*, int);

//todo: Set the staticClassNum and staticMemberNum in ast!!!

static void _throwError(char *message, int errorLine) {
  printf(RED"\nERROR >>> "NORMAL);
  printf("%s\n", message);
  printf("  Error in Line Number: %d", errorLine);
  printf("\n");
}

static void throwError(char *message, int errorLine) {
  _throwError(message, errorLine);
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
    int typeOfBodyExprs = typeExprs(currentMethod.bodyExprs, classNum, i);
    if(!isSubtype(typeOfBodyExprs, currentMethod.returnType)) {
      throwError("Incompatible return expression.",
        currentMethod.bodyExprs->childrenTail->data->lineNumber);
    }
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

    case NOT_EXPR:
      return typeNotExpr(t, classContainingExpr, methodContainingExpr);

    case AST_ID:
      return typeId(t, classContainingExpr, methodContainingExpr);

    case ID_EXPR:
    {
      ASTree *astId = t->children->data;
      return typeExpr(astId, classContainingExpr, methodContainingExpr);
    }

    case DOT_ID_EXPR:
      return typeDotIdExpr(t, classContainingExpr, methodContainingExpr);

    case ASSIGN_EXPR:
      return typeAssignExpr(t, classContainingExpr, methodContainingExpr);

    case EQUALITY_EXPR:
    case GREATER_THAN_EXPR:
    case OR_EXPR:
      return typeCompExpr(t, classContainingExpr, methodContainingExpr);

    case PLUS_EXPR:
    case MINUS_EXPR:
    case TIMES_EXPR:
      return typeBinaryExpr(t, classContainingExpr, methodContainingExpr);

    case DOT_ASSIGN_EXPR:
      return typeDotAssignExpr(t, classContainingExpr, methodContainingExpr);

    case METHOD_CALL_EXPR:
      return typeMethodCallExpr(t, classContainingExpr, methodContainingExpr);

    case DOT_METHOD_CALL_EXPR:
      return typeDotMethodCallExpr(t, classContainingExpr, methodContainingExpr);

    case IF_THEN_ELSE_EXPR:
      return typeIfThenElseExpr(t, classContainingExpr, methodContainingExpr);

    case FOR_EXPR:
      return typeForExpr(t, classContainingExpr, methodContainingExpr);

    default:
      if(t->idVal == NULL) {
        printf("Type Checking: %d | natVal = %d | in line number: %u\n", t->typ, t->natVal, t->lineNumber );
      } else {
        printf("Type Checking: %d | idVal = %s | in line number: %u\n", t->typ, t->idVal, t->lineNumber );
      }
      return 0;
  }
}

/** Type checks the provided expressions list.
  * @param t => AST of the expression list.
  * @param classContainingExprs => class number of the containing class
  * @param methodContainingExprs => method number of the containing method
  * @returns => The type of the last expression in the expression list.
  */
int typeExprs(ASTree *t, int classContainingExprs, int methodContainingExprs) {
  ASTList *currentNode = t->children;
  int typeOfExprs;
  while(currentNode != NULL && currentNode->data != NULL) {
    typeOfExprs = typeExpr(currentNode->data, classContainingExprs, methodContainingExprs);
    currentNode = currentNode->next;
  }
  printf("Type of exprs is: %d in class: %d, method: %d\n\n\n", typeOfExprs,
   classContainingExprs, methodContainingExprs);
  return typeOfExprs;
}

/** Type checks dot expression id.
  * @param t => AST of the expression list.
  * @param classContainingExpr => class number of the containing class
  * @param methodContainingExpr => method number of the containing method
  * @returns => The type of the field name if found.
  * @throws => Dot operation not allowed error.
            => Field name not defined error.
  */
int typeDotIdExpr(ASTree *t, int classContainingExpr, int methodContainingExpr){
  ASTList *dotIdNode = t->children;
  int typeOfExpr = typeExpr(dotIdNode->data, classContainingExpr, methodContainingExpr);

  if(typeOfExpr < 0){
    throwError("Dot operation is not allowed for this type.", t->lineNumber);
  }

  dotIdNode = dotIdNode->next;
  char *fieldName = dotIdNode->data->idVal;
  ClassDecl currentClass;

  typeOfExpr = getTypeOfVarInClass(classesST[typeOfExpr], fieldName);

  if(typeOfExpr == -3) {
    printf("Field name '%s' not defined.", fieldName);
    throwError("Undefined Field Name.", t->lineNumber );
  }
  return typeOfExpr;
}

int typeCompExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  ASTList *compNode = t->children;
  int typeLeftOperand = typeExpr(compNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of Left operand = %d\n", typeLeftOperand);
  compNode = compNode->next;
  int typeRightOperand = typeExpr(compNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of Right OPerand = %d\n", typeRightOperand);
  if(!isSubtype(typeLeftOperand, typeRightOperand) || !isSubtype(typeRightOperand, typeLeftOperand)) {
    throwError("Type mismatch.", compNode->data->lineNumber);
  }
  return -1;
}

int typeBinaryExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  ASTList *binaryNode = t->children;
  int typeLeftOperand = typeExpr(binaryNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of Left operand = %d\n", typeLeftOperand);
  binaryNode = binaryNode->next;
  int typeRightOperand = typeExpr(binaryNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of Right OPerand = %d\n", typeRightOperand);
  if(!isSubtype(typeLeftOperand, typeRightOperand) || !isSubtype(typeRightOperand, typeLeftOperand)) {
    throwError("Error in Binary expression. Type mismatch.", binaryNode->data->lineNumber);
  }
  return join(typeLeftOperand, typeRightOperand);
}

/** Type checks New Expression
  * @param astClassName => The AST representing the class name of new expression.
  * @returns => The type number of a valid new expression.
  * @throws => Invalid Type error.
  */
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

/** Type check AST_ID. Looks for the variable name either in main locals
    or method locals and class and super class fields.
  * @param t=> AST of the ID.
  * @param classContainingExpr => class number of the containing class
  * @param methodContainingExpr => method number of the containing method
  * @returns => The type of the ID if valid.
  * @throws => Implicit Declaration of variable error
  */
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

/** Checks for the variable name in fields of the current class and its superclasses recursively.
  * @param currentClass => The symbol table of the class in which to look.\
  * @param varName => The variable name to look for
  * @returns => The type of varName OR -3(which is an illegal type in DJ) if not found.
  */
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

/** Checks for the variable name in the provided VarDecl list (Symbol table)
  * @param varList => The symbol table in which to search for varName
  * @numVars => The number of variables in the varList
  * @param varName => The variable to type check
  * @returns => the type of varName OR -3(which is an illegal type in DJ) if not found.
  */
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

/** Type check assignment expression
  * @param t => AST for assignment expression
  * @param classContainingExpr => class number of the containing class
  * @param methodContainingExpr => method number of the containing method
  * @returns => type number of the assignment expr = the type of lhs expression
  * @throws => RHS is not a sub-type error
  */
int typeAssignExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  ASTList *assignNode = t->children;
  int typeOfLhs = typeExpr(assignNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of LHS = %d\n", typeOfLhs);
  assignNode = assignNode->next;
  int typeOfRhs = typeExpr(assignNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of RHS = %d\n", typeOfRhs);
  printf("%d", isSubtype(typeOfRhs, typeOfLhs));
  if(!isSubtype(typeOfRhs, typeOfLhs)) {
    throwError("Error is Assignment. RHS is not a sub-type of LHS.", assignNode->data->lineNumber);
  }
  printf("type of assignment is: %d\n", typeOfLhs);
  return typeOfLhs;
}

int typeDotAssignExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  ASTList *dotAssignNode = t->children;
  int typeOfLhs = typeExpr(dotAssignNode->data, classContainingExpr, methodContainingExpr);

  if(typeOfLhs < 0){
    throwError("Dot operation is not allowed for this type.", t->lineNumber);
  }

  dotAssignNode = dotAssignNode->next;
  char *fieldName = dotAssignNode->data->idVal;
  ClassDecl currentClass;

  typeOfLhs = getTypeOfVarInClass(classesST[typeOfLhs], fieldName);

  if(typeOfLhs == -3) {
    printf("Field name '%s' not defined.", fieldName);
    throwError("Undefined Field Name.", t->lineNumber );
  }

  dotAssignNode = dotAssignNode->next;
  int typeOfRhs = typeExpr(dotAssignNode->data, classContainingExpr, methodContainingExpr);

  if(!isSubtype(typeOfRhs, typeOfLhs)) {
    throwError("Error is Assignment. RHS is not a sub-type of LHS.", dotAssignNode->data->lineNumber);
  }
  printf("type of assignment is: %d\n", typeOfLhs);
  return typeOfLhs;
}

int typeNotExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  ASTList *notNode = t->children;
  int typeOfExpr = typeExpr(notNode->data, classContainingExpr, methodContainingExpr);
  if(typeOfExpr != -1) {
    throwError("Cannot find complement of a none Nat Type.", notNode->data->lineNumber);
  }
  return typeOfExpr;
}

/** Type checks a method call
  * @param t => AST for method call expression
  * @param classContainingExpr => class number of the containing class
  * @param methodContainingExpr => method number of the containing method
  * @returns => The return type of the method called.
  * @throws => Incompatible type error
  */
int typeMethodCallExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  printf("TYPE CHECKING METHOD CALL!!!!!!!!!!!!!!!!!!!!!\n\n\n\n");
  ASTList *methodCallNode = t->children;
  char *methodName = methodCallNode->data->idVal;
  /* Check if method exists in current class/super classes */
  MethodDecl methodDecl = getMethodDeclInClass(classesST[classContainingExpr],
    methodName, methodCallNode->data->lineNumber);

  methodCallNode = methodCallNode->next;
  int typeOfParam = typeExpr(methodCallNode->data, classContainingExpr, methodContainingExpr);
  printf("Type of Param is : %d\n", typeOfParam);
  printf("Expected Type of Param is : %d\n", methodDecl.paramType);

  if(methodDecl.paramType != typeOfParam) {
    throwError("Incompatible type of parameter in method call.", methodCallNode->data->lineNumber);
  }
  return methodDecl.returnType;
}

int typeDotMethodCallExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  ASTList *dotMethodNode = t->children;
  int typeOfExpr = typeExpr(dotMethodNode->data, classContainingExpr, methodContainingExpr);

  if(typeOfExpr < 0){
    throwError("Dot operation is not allowed for this type.", t->lineNumber);
  }

  dotMethodNode = dotMethodNode->next;
  char *methodName = dotMethodNode->data->idVal;
  /* Check if method exists in current class/super classes */
  MethodDecl methodDecl = getMethodDeclInClass(classesST[typeOfExpr],
   methodName, dotMethodNode->data->lineNumber);

  dotMethodNode = dotMethodNode->next;
  int typeOfParam = typeExpr(dotMethodNode->data, classContainingExpr, methodContainingExpr);
  printf("Type of Param is : %d\n", typeOfParam);
  printf("Expected Type of Param is : %d\n", methodDecl.paramType);

  if(methodDecl.paramType != typeOfParam) {
    throwError("Incompatible type of parameter in method call.", dotMethodNode->data->lineNumber);
  }
  return methodDecl.returnType;
}

/** Searches for the given method in given class and its super classes.
  * @param currentClass => the current class in which to look for.
  * @param methodName => the method to look for
  * @returns => MethodDecl of the method if found.
  * @throws => Method not found error.
  */
MethodDecl getMethodDeclInClass(ClassDecl currentClass, char *methodName, int lineNumber) {
  int i;
  MethodDecl *methodList = currentClass.methodList;
  for(i = 0; i < currentClass.numMethods; i += 1) {
    if(strcmp(methodName, methodList[i].methodName) == 0){
      return methodList[i];
    }
  }
  if(currentClass.superclass > 0) {   // Look for the method in all super classes
    return getMethodDeclInClass(classesST[currentClass.superclass], methodName, lineNumber);
  } else {
    _throwError("Method not found", lineNumber);
    exit(-1);
  }
}

int typeIfThenElseExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  ASTList *ifThenElseNode = t->children;
  int typeOfExpr = typeExpr(ifThenElseNode->data, classContainingExpr, methodContainingExpr);

  if(typeOfExpr != -1){
    throwError("Expected type Nat.", ifThenElseNode->data->lineNumber);
  }

  ifThenElseNode = ifThenElseNode->next;
  int typeOfIf = typeExprs(ifThenElseNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of if = %d\n", typeOfIf);

  ifThenElseNode = ifThenElseNode->next;
  int typeOfElse = typeExprs(ifThenElseNode->data, classContainingExpr, methodContainingExpr);
  printf("The type of Else = %d\n", typeOfElse);
  printf("%d", isSubtype(typeOfIf, typeOfElse));

  if(!isSubtype(typeOfIf, typeOfElse) || !isSubtype(typeOfIf, typeOfElse)) {
    throwError("Error in If then Else expression. Type mismatch.", ifThenElseNode->data->lineNumber);
  }
  return join(typeOfIf, typeOfElse);
}

int typeForExpr(ASTree *t, int classContainingExpr, int methodContainingExpr) {
  ASTList *forNode = t->children;
  int typeOfExpr = typeExpr(forNode->data, classContainingExpr, methodContainingExpr);

  forNode = forNode->next;
  typeOfExpr = typeExpr(forNode->data, classContainingExpr, methodContainingExpr);

  if(typeOfExpr != -1){
    throwError("Expected type Nat.", forNode->data->lineNumber);
  }

  forNode = forNode->next;
  typeOfExpr = typeExpr(forNode->data, classContainingExpr, methodContainingExpr);

  forNode = forNode->next;
  typeOfExpr = typeExprs(forNode->data, classContainingExpr, methodContainingExpr);
  return -1;
}



