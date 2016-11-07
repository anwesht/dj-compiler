/**
 * Created by atuladhar on 11/6/16.
 * Pledge: I pledge my Honor that I have not cheated, and will not cheat, on this assignment
 * Name: Anwesh Tuladhar
 */
#include <stdio.h>
#include <string.h>
#include "symtbl.h"

typedef struct Map {
  char *name;
//  int num;
} Map;

static Map *classNameToNumberMap;
void printClassesToNumberMap();

//Available global variables from symtbl.h
//ASTree *mainExprs;
//
//// Array (symbol table) of locals in the main block
//int numMainBlockLocals;  //size of the array
//VarDecl *mainBlockST;  //the array itself
//int numClasses;  //size of the array
//ClassDecl *classesST;  //the array itself

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
  int returnNum = -3;
  for(int i = 0; i <= numClasses; i += 1) {
    printf("To Find: %s\n", className);
    printf("Current ClassName: %s || ClassNumber = %d\n", classNameToNumberMap[i].name, i);
    printf("\n");

    if(strcmp(className, classNameToNumberMap[i].name) == 0) {
      returnNum = i;
      printf("returning : %d\n", returnNum);
      break;
    }
  }
  return returnNum;
}

int getLengthOfList(ASTList *currentNode){
  int length = 0;
  printf("Fields are:  %p" ,currentNode);

  while (currentNode != NULL && currentNode->data != NULL) {
    length += 1;
    currentNode = currentNode->next;
  }
  printf("Length of current list is: %d\n" , length);
  return length;
}

void setNumMainBlockLocals(ASTList *mainVarDeclList) {
  numMainBlockLocals = getLengthOfList(mainVarDeclList);
  printf("length of local vars is: %d\n", numMainBlockLocals);
}

void setNumClasses(ASTList *classDeclList) {
  numClasses = getLengthOfList(classDeclList);
  printf("Number of classes is: %d\n", numClasses);
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

  printf("Var Type = %d\n",  stEntry->type);
  printf("Var Type line number = %d\n", stEntry->typeLineNumber);

  varDeclNode = varDeclNode->next;

  char *varName = varDeclNode->data->idVal;
  printf("setting var decl for : %s\n", varName);
  char *copyStr = (char*)Calloc(1, (strlen(varName) + 1));
  strcpy(copyStr, varName);
  stEntry->varName = copyStr;
  stEntry->varNameLineNumber = varDeclNode->data->lineNumber;

  printf("Var Name = %s\n",  stEntry->varName);
  printf("Var Name line number = %d\n", stEntry->varNameLineNumber);
}

void setClassName(ASTree *classDecl, ClassDecl *stEntry) {
  char *className = classDecl->idVal;
  char *copyStr = (char*)Calloc(1, (strlen(className) + 1));
  strcpy(copyStr, className);
  stEntry->className = copyStr;
  stEntry->classNameLineNumber = classDecl->lineNumber;
  printf("Class Name = %s\n",  stEntry->className);
  printf("Class Name line number = %d\n", stEntry->classNameLineNumber);
}

void setSuperClass(ASTree *superClassDecl, ClassDecl *stEntry) {
  printf("SuperClass is : %s\n", superClassDecl->idVal);
  stEntry->superclass = classNameToNumber(superClassDecl->idVal);
  printf("after setting super class num;\n");
  stEntry->superclassLineNumber = superClassDecl->lineNumber;
    printf("after setting super class line number;\n");

  printf("Super Class Number = %d\n",  stEntry->superclass);
  printf("Super Class line number = %d\n", stEntry->superclassLineNumber);
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

  printf("Return Type = %d\n",  stEntry->returnType);
  printf("Return Type line number = %d\n", stEntry->returnTypeLineNumber);

  methodDeclNode = methodDeclNode->next;

  char *methodName = methodDeclNode->data->idVal;
  char *copyStr = (char*)Calloc(1, (strlen(methodName) + 1));
  strcpy(copyStr, methodName);
  stEntry->methodName = copyStr;
  stEntry->methodNameLineNumber = methodDeclNode->data->lineNumber;

  printf("Method Name = %s\n",  stEntry->methodName);
  printf("Method Name line number = %d\n", stEntry->methodNameLineNumber);

  methodDeclNode = methodDeclNode->next;

  stEntry->paramType = getTypeNumber(methodDeclNode->data);
  stEntry->paramTypeLineNumber = methodDeclNode->data->lineNumber;

  printf("param Type = %d\n",  stEntry->paramType);
  printf("param Type line number = %d\n", stEntry->paramTypeLineNumber);

  methodDeclNode = methodDeclNode->next;

  char *paramName = methodDeclNode->data->idVal;
  copyStr = (char*)Calloc(1, (strlen(paramName) + 1));
  strcpy(copyStr, paramName);
  stEntry->paramName = copyStr;
  stEntry->paramNameLineNumber = methodDeclNode->data->lineNumber;

  printf("Param Name = %s\n",  stEntry->paramName);
  printf("Param Name line number = %d\n", stEntry->paramNameLineNumber);

  methodDeclNode = methodDeclNode->next;

  stEntry->numLocals = getLengthOfList(methodDeclNode->data->children);
  printf("Number of Locals in Method is : %d\n " , stEntry->numLocals);
  VarDecl *localST = (VarDecl*)Calloc(stEntry->numLocals, sizeof(VarDecl));
  setVarDeclList(methodDeclNode->data, localST);

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
  printf("Setting up main block st\n");
  /*Allocate memory for mainBlockST */
  mainBlockST = (VarDecl*)Calloc(numMainBlockLocals, sizeof(VarDecl));
  setVarDeclList(mainVarDeclList, mainBlockST);
}

void setupClassesST(ASTree *classDeclList) {
  printf("Setting up Classes ST\n");
  /*Allocate memory for classST */
  classesST = (ClassDecl*)Calloc(numClasses+1, sizeof(ClassDecl));

  ASTList *currentNode = classDeclList->children;
//  ASTree *classDeclNode;
  ASTList *classDeclNode;

  int classNum = 1; //class 0 = Object
  while(currentNode != NULL && currentNode->data != NULL) {
    printf("Setting up Class Num: %d\n", classNum);
    ClassDecl *stEntry = (ClassDecl*)Calloc(1, sizeof(ClassDecl));

    printf("after calloc class;\n");
    classDeclNode = currentNode->data->children;
    printf("class decl NOde.\n");
    setClassName(classDeclNode->data, stEntry);
    printf("after setting class name.\n");

    classDeclNode = classDeclNode->next;
    printf("super class. \n");

    printAST(classDeclNode->data);
//    setSuperClass(classDeclNode->data, stEntry);
    stEntry->superclass = getTypeNumber(classDeclNode->data);
    stEntry->superclassLineNumber = classDeclNode->data->lineNumber;


    classDeclNode = classDeclNode->next;
    stEntry->numVars = getLengthOfList(classDeclNode->data->children);
    printf("Number of fields is : %d\n " , stEntry->numVars);
    VarDecl *varList = (VarDecl*)Calloc(stEntry->numVars, sizeof(VarDecl));
    setVarDeclList(classDeclNode->data, varList);

    classDeclNode = classDeclNode->next;
    stEntry->numMethods = getLengthOfList(classDeclNode->data->children);
    printf("Number of Methods is : %d\n", stEntry->numMethods);

    MethodDecl *methodList = (MethodDecl*)Calloc(stEntry->numMethods, sizeof(MethodDecl));
    setMethodDeclList(classDeclNode->data, methodList);

    classesST[classNum] = *stEntry;
    classNum += 1;
    currentNode = currentNode->next;
  }
}


void printMainBlockST() {
  for(int i = 0; i < numMainBlockLocals; i += 1) {
    printf("VarName: %s (at line number: %d)", mainBlockST[i].varName, mainBlockST[i].varNameLineNumber);
    printf("\n");
  }
}

void printClassesToNumberMap() {
  for(int i = 0; i <= numClasses; i += 1) {
    printf("ClassName: %s || ClassNumber = %d", classNameToNumberMap[i].name, i);
    printf("\n");
  }
}

void setupSymbolTables(ASTree *fullProgramAST) {
  if(fullProgramAST == NULL) throwError("Program AST is NULL");
  if(fullProgramAST->children == NULL) throwError("Program AST has no children");
  if(fullProgramAST->children->data == NULL) throwError("Program AST has no classes");
  if(fullProgramAST->children->next == NULL) throwError("Program AST has no local vars in main");

  ASTree *classDeclList = fullProgramAST->children->data;
  printf("calling set numClasses\n");
  setNumClasses(classDeclList->children);
  printf("calling Setup Classes to NUmber map\n");
  setupClassesToNumberMap(classDeclList->children);

  printf("Calling print classes to number map\n");
  printClassesToNumberMap();
  printf("calling Setup Classes ST\n");
  setupClassesST(classDeclList);

  ASTree *mainVarDeclList = fullProgramAST->children->next->data;
  printf("calling set num main block st\n");
  setNumMainBlockLocals(mainVarDeclList->children);
  printf("calling setup main block st\n");
  setupMainBlockST(mainVarDeclList);


  printMainBlockST();

}



