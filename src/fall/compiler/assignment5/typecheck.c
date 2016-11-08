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
void checkUniqueClassNames();

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

  checkUniqueClassNames();
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
void checkUniqueClassNames() {
  int i, j;
  for(i = 1; i <= numClasses; i += 1) {
    char *currentClass = classesST[i].className;
    for(j = i + 1; j <= numClasses; j += 1) {
      if(strcmp(currentClass, classesST[j].className) == 0){
        printf("Class %s is already defined in line: %d.", classesST[i].className, classesST[i].classNameLineNumber);
        throwError("Class redefined.", classesST[j].classNameLineNumber);
      }
    }
  }
}

int typeExpr(ASTree *t, int classContainingExpr, int methodContainingExpr);

int typeExprs(ASTree *t, int classContainingExprs, int methodContainingExprs);

