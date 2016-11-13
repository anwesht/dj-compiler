/**
 * Created by atuladhar on 11/6/16.
 * Pledge: I pledge my Honor that I have not cheated, and will not cheat, on this assignment
 * Name: Anwesh Tuladhar
 */
#include <stdio.h>
#include <string.h>
#include "symtbl.h"

typedef enum
{
  false,
  true
} bool;

typedef struct Map {
  char *name;
} Map;

static Map *classNameToNumberMap;
void printClassesToNumberMap();

static void throwError(char *message) {
  printf("\n>>> ");
  printf("%s", message);
  printf("\n");
  exit(-1);
}

/** Calloc: A wrapper function to call calloc.
  * @param nitems => Number of items to be allocated
  * @param size => Size of each item for which the memory is allocated
  * @returns memPrt => Pointer to the allocated memory
  * @throws Memory allocation error and exits if there is a problem in memory allocation
  */
void* Calloc(size_t nitems, size_t size) {
  void *memPtr = calloc(nitems, size);
  if(memPtr == NULL) throwError("Error allocating memory.");
  return memPtr;
}

int classNameToNumber(char *className) {
  int returnNum = -3, i;
  for(i = 0; i < numClasses; i += 1) {
    if(strcmp(className, classNameToNumberMap[i].name) == 0) {
      returnNum = i;
      break;
    }
  }
  return returnNum;
}

int getLengthOfList(ASTList *currentNode){
  int length = 0;

  while (currentNode != NULL && currentNode->data != NULL) {
    length += 1;
    currentNode = currentNode->next;
  }
  return length;
}

void setNumMainBlockLocals(ASTList *mainVarDeclList) {
  numMainBlockLocals = getLengthOfList(mainVarDeclList);
}

void setNumClasses(ASTList *classDeclList) {
  numClasses = getLengthOfList(classDeclList);
  numClasses += 1;  // For Object Class.
}

void setupClassesToNumberMap(ASTList *currentNode) {
  ASTree *classDecl = NULL;
  classNameToNumberMap = (Map*) Calloc(numClasses+1, sizeof(Map));
  char *className = "Object";

  classNameToNumberMap[0].name = className;
  int classNum = 1;
  while (currentNode != NULL && currentNode->data != NULL) {
    classDecl = currentNode->data;
    char *className = classDecl->children->data->idVal;

    char *copyStr = (char*)Calloc(1, (strlen(className) + 1));
    strcpy(copyStr, className);

    classNameToNumberMap[classNum].name = copyStr;

    classNum += 1;
    currentNode = currentNode->next;
  }
}

int getTypeNumber(ASTree *astTypeDecl) {
  switch(astTypeDecl->typ) {
    case NAT_TYPE:
      return -1;

    case OBJ_TYPE:
      return 0;

    default:
      return classNameToNumber(astTypeDecl->idVal);
  }
}

void setVarDecl(ASTList *varDeclNode, VarDecl *stEntry) {
  stEntry->type = getTypeNumber(varDeclNode->data);
  stEntry->typeLineNumber = varDeclNode->data->lineNumber;

  varDeclNode = varDeclNode->next;

  char *varName = varDeclNode->data->idVal;

  char *copyStr = (char*)Calloc(1, (strlen(varName) + 1));
  strcpy(copyStr, varName);
  stEntry->varName = copyStr;
  stEntry->varNameLineNumber = varDeclNode->data->lineNumber;
}

void setClassName(ASTree *classDecl, ClassDecl *stEntry) {
  char *className = classDecl->idVal;
  char *copyStr = (char*)Calloc(1, (strlen(className) + 1));
  strcpy(copyStr, className);
  stEntry->className = copyStr;
  stEntry->classNameLineNumber = classDecl->lineNumber;
}

void setSuperClass(ASTree *superClassDecl, ClassDecl *stEntry) {
  stEntry->superclass = classNameToNumber(superClassDecl->idVal);
  stEntry->superclassLineNumber = superClassDecl->lineNumber;
}

void setVarDeclList(ASTree *varDeclList, VarDecl *st) {
  ASTList *currentNode = varDeclList->children;
  ASTList *varDeclNode;
  int pos = 0;
  while(currentNode != NULL && currentNode->data != NULL) {
    VarDecl *stEntry = (VarDecl*)Calloc(1, sizeof(VarDecl));

    varDeclNode = currentNode->data->children;
    setVarDecl(varDeclNode, stEntry);

    st[pos] = *stEntry;
    pos += 1;
    currentNode = currentNode->next;
  }
}

void setMethodDecl(ASTList *methodDeclNode, MethodDecl *stEntry) {
  stEntry->returnType = getTypeNumber(methodDeclNode->data);
  stEntry->returnTypeLineNumber = methodDeclNode->data->lineNumber;

  methodDeclNode = methodDeclNode->next;

  char *methodName = methodDeclNode->data->idVal;
  char *copyStr = (char*)Calloc(1, (strlen(methodName) + 1));
  strcpy(copyStr, methodName);
  stEntry->methodName = copyStr;
  stEntry->methodNameLineNumber = methodDeclNode->data->lineNumber;

  methodDeclNode = methodDeclNode->next;

  stEntry->paramType = getTypeNumber(methodDeclNode->data);
  stEntry->paramTypeLineNumber = methodDeclNode->data->lineNumber;

  methodDeclNode = methodDeclNode->next;

  char *paramName = methodDeclNode->data->idVal;
  copyStr = (char*)Calloc(1, (strlen(paramName) + 1));
  strcpy(copyStr, paramName);
  stEntry->paramName = copyStr;
  stEntry->paramNameLineNumber = methodDeclNode->data->lineNumber;

  methodDeclNode = methodDeclNode->next;

  stEntry->numLocals = getLengthOfList(methodDeclNode->data->children);
  VarDecl *localST = (VarDecl*)Calloc(stEntry->numLocals, sizeof(VarDecl));
  setVarDeclList(methodDeclNode->data, localST);
  stEntry->localST = localST;

  methodDeclNode = methodDeclNode->next;
  stEntry->bodyExprs = methodDeclNode->data;

}

void setMethodDeclList(ASTree *methodDeclList, MethodDecl *st) {
  ASTList *currentNode = methodDeclList->children;
  ASTList *methodDeclNode;
  int pos = 0;
  while(currentNode != NULL && currentNode->data != NULL) {
    MethodDecl *stEntry = (MethodDecl*)Calloc(1, sizeof(MethodDecl));

    methodDeclNode = currentNode->data->children;
    setMethodDecl(methodDeclNode, stEntry);

    st[pos] = *stEntry;
    pos += 1;
    currentNode = currentNode->next;
  }
}

void setupMainBlockST(ASTree *mainVarDeclList) {
  /*Allocate memory for mainBlockST */
  mainBlockST = (VarDecl*)Calloc(numMainBlockLocals, sizeof(VarDecl));
  setVarDeclList(mainVarDeclList, mainBlockST);
}

void setupClassesST(ASTree *classDeclList) {
  /*Allocate memory for classST */
  classesST = (ClassDecl*)Calloc(numClasses+1, sizeof(ClassDecl));

  ASTList *currentNode = classDeclList->children;
//  ASTree *classDeclNode;
  ASTList *classDeclNode;

  int classNum = 1; //class 0 = Object
  while(currentNode != NULL && currentNode->data != NULL) {
    ClassDecl *stEntry = (ClassDecl*)Calloc(1, sizeof(ClassDecl));

    classDeclNode = currentNode->data->children;
    setClassName(classDeclNode->data, stEntry);

    classDeclNode = classDeclNode->next;

    stEntry->superclass = getTypeNumber(classDeclNode->data);
    stEntry->superclassLineNumber = classDeclNode->data->lineNumber;

    classDeclNode = classDeclNode->next;
    stEntry->numVars = getLengthOfList(classDeclNode->data->children);
    VarDecl *varList = (VarDecl*)Calloc(stEntry->numVars, sizeof(VarDecl));
    setVarDeclList(classDeclNode->data, varList);
    stEntry->varList = varList;

    classDeclNode = classDeclNode->next;
    stEntry->numMethods = getLengthOfList(classDeclNode->data->children);

    MethodDecl *methodList = (MethodDecl*)Calloc(stEntry->numMethods, sizeof(MethodDecl));
    setMethodDeclList(classDeclNode->data, methodList);
    stEntry->methodList = methodList;

    classesST[classNum] = *stEntry;
    classNum += 1;
    currentNode = currentNode->next;
  }
}


void printVarList(VarDecl *st, int size) {
  int i;
  for(i = 0; i < size; i += 1) {
    printf("    VarType: %d (at line number: %d)\n", st[i].type, st[i].typeLineNumber);
    printf("    VarName: %s (at line number: %d)\n", st[i].varName, st[i].varNameLineNumber);
  }
}

void printMethodList(MethodDecl *st, int size) {
  int i;
  for(i = 0; i < size; i += 1) {
    printf("  returnType => %d (at line number: %d)\n", st[i].returnType, st[i].returnTypeLineNumber);
    printf("  MethodName => %s (at line number: %d)\n", st[i].methodName, st[i].methodNameLineNumber);
    printf("  ParamType => %d (at line number: %d)\n", st[i].paramType, st[i].paramTypeLineNumber);
    printf("  ParamName => %s (at line number: %d)\n", st[i].paramName, st[i].paramNameLineNumber);
    printf("  Num Locals => %d\n", st[i].numLocals);
    printVarList(st[i].localST, st[i].numLocals);
  }
}

void printClassesToNumberMap() {
  int i;
  for(i = 0; i < numClasses; i += 1) {
    printf("ClassName: %s || ClassNumber = %d", classNameToNumberMap[i].name, i);
    printf("\n");
  }
}

void printClassesST() {
  int i;
  for(i = 1; i < numClasses; i += 1) {
    printf("ClassName => %s (line number: %d)\n", classesST[i].className, classesST[i].classNameLineNumber );
    printf("SuperClass => %d (line number: %d)\n", classesST[i].superclass, classesST[i].superclassLineNumber );
    printf("Num Vars => %d \n", classesST[i].numVars );
    printVarList(classesST[i].varList, classesST[i].numVars);
    printf("Num Methods => %d \n", classesST[i].numMethods );
    printMethodList(classesST[i].methodList, classesST[i].numMethods);
    printf("\n\n");
  }
}

void setupSymbolTables(ASTree *fullProgramAST) {
  if(fullProgramAST == NULL) throwError("Program AST is NULL");
  if(fullProgramAST->children == NULL) throwError("Program AST has no children");
  if(fullProgramAST->children->data == NULL) throwError("Program AST has no classes");
  if(fullProgramAST->children->next == NULL) throwError("Program AST has no local vars in main");

  /* Set global variable entire wholeProgram */
  wholeProgram = fullProgramAST;

  /* Set global varialbe mainExprs */
  mainExprs = fullProgramAST->children->next->next->data;

  ASTree *classDeclList = fullProgramAST->children->data;
  setNumClasses(classDeclList->children);
  setupClassesToNumberMap(classDeclList->children);

  bool DEBUG = false;

  if(DEBUG) printClassesToNumberMap();
  setupClassesST(classDeclList);

  ASTree *mainVarDeclList = fullProgramAST->children->next->data;
  setNumMainBlockLocals(mainVarDeclList->children);
  setupMainBlockST(mainVarDeclList);

  if(DEBUG) printClassesST();
  if(DEBUG) printVarList(mainBlockST, numMainBlockLocals);

}



