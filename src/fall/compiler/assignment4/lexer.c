/** Created by atuladhar on 9/15/16.
  * Pledge: I pledge my Honor that I have not cheated, and will not cheat, on this assignment
  * Name: Anwesh Tuladhar
  * Code adapted from Tiny Scanner implementation in Compiler Construction: Principles and Practice by K. Louden
  */

#include "lexer.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

char *curLine;                      // current line buffer
int pos = 0, lineNo = 0;            // position and line number tracker
ssize_t lineLength = 0;             // length of current line
size_t len = 256;                   // size for buffer. Gets resized automatically.
char tokenString[MAXTOKENLEN + 1];  // current token buffer.
FILE *fp;                           // file to be tokenized.
int isEOF = FALSE;
int DEBUG = FALSE;

/** lookup table of reserved words */
static struct
{  char* str;
   Token tok;
} reservedWords[MAXRESERVED] = { {"class", CLASS}, {"extends", EXTENDS}, {"main", MAIN},
                                 {"nat", NATTYPE}, {"Object", OBJECT}, {"printNat", PRINTNAT},
                                 {"readNat", READNAT}, {"if", IF}, {"else", ELSE}, {"for", FOR},
                                 {"null", NUL}, {"new", NEW}, {"this", THIS}
                               };

/** Check if ID is a reserved word
  * @param s => ID to lookup
  * @return Token
  */
static Token reservedLookup (char * s)
{ int i;
  for (i = 0; i < MAXRESERVED; i++)
    if (!strcmp(s, reservedWords[i].str))
      return reservedWords[i].tok;
  return ID;
}

/** Read next line in the file. Frees the memory allocated for current line.
  * @return => TRUE when successfully reads a line, else FALSE and sets the isEOF flag.
  */
static int getNextLine() {
  if (curLine != NULL) {
    free(curLine);
    curLine = NULL;
  }
  if ((lineLength = getline( &curLine, &len, fp )) != -1 ) {
    pos = 0;
    lineNo++;
    return TRUE;
  } else {
    isEOF = TRUE;
    return FALSE;
  }
}

/** Retrieves the next char in the input stream and moves the cursor forward.
  * Reads the given file line by line and keeps track of the line number.
  * @return => The next char in the input stream or EOF.
  */
static int getNextChar(void)
{
  if (pos < (int)lineLength || getNextLine()) {
    return curLine[pos++];
  }
  else {
    return EOF;
  }
}

/** Backtrack the cursor by one position.
  * Used when current char is not to be consumed by the state machine.
  */
static void ungetNextChar(void) {
  pos--;
}

/** Output the given token to stdout
  * @param t => Token to output
  * @param tokenString => The string representing the token. The attribute of the Token incase of NATLITERAL and ID
  */
void printToken(TokenType tt) {
  Token t = tt.tok;
  char* tokenString = tt.str;
  switch(t) {
    case CLASS:         printf("CLASS "); break;
    case ID:            printf("ID(%s) ", tokenString); break;
    case EXTENDS:       printf("EXTENDS "); break;
    case MAIN:          printf("MAIN "); break;
    case NATTYPE:       printf("NATTYPE "); break;
    case OBJECT:        printf("OBJECT "); break;
    case NATLITERAL:    printf("NATLITERAL(%s) ", tokenString); break;
    case PRINTNAT:      printf("PRINTNAT "); break;
    case READNAT:       printf("READNAT "); break;
    case IF:            printf("IF "); break;
    case ELSE:          printf("ELSE "); break;
    case FOR:           printf("FOR "); break;
    case PLUS:          printf("PLUS "); break;
    case MINUS:         printf("MINUS "); break;
    case TIMES:         printf("TIMES "); break;
    case EQUALITY:      printf("EQUALITY "); break;
    case GREATER:       printf("GREATER "); break;
    case OR:            printf("OR "); break;
    case NOT:           printf("NOT "); break;
    case ASSIGN:        printf("ASSIGN "); break;
    case NUL:           printf("NUL "); break;
    case NEW:           printf("NEW "); break;
    case THIS:          printf("THIS "); break;
    case DOT:           printf("DOT "); break;
    case SEMICOLON:     printf("SEMICOLON "); break;
    case LBRACE:        printf("LBRACE "); break;
    case RBRACE:        printf("RBRACE "); break;
    case LPAREN:        printf("LPAREN "); break;
    case RPAREN:        printf("RPAREN "); break;
    case ENDOFFILE:     printf("ENDOFFILE\n"); break;
    case ERROR:         printf("\nLex error on line %d: Illegal character %s\n", lineNo, tokenString); break;
    default:            printf("ERROR: invalid token in scanned().\n");
  }
}

/** A state machine to identify the next token.
  * Skips over comment section.
  * Incase of error: Returns ERROR token. Next time getToken is called, continues tokenizing from the next character.
  * @return currentToken => The longest token identified.
  */
TokenType getToken(void)
{
  int currentTokenIndex = 0;        // tokenString index
  int save;                         // flag to indicate save current char to tokenString
  Token currentToken;               // current token
  State state = START;              // track current state of the DFA
  TokenType currentTokenType;

  while (state != DONE) {
    int c = getNextChar();
    save = TRUE;
    switch (state) {
      case START:
        if (isdigit(c)){            // library function
          state = INNUM;
        }
        else if (isalpha(c)) {      //library function
          state = INID;
        }
        else if (c == '=') {
          state = INASSIGN;
        }
        else if (c == '/') {
          state = INCOMMENT;
        }
        else if(c == '|') {
          state = INOR;
        }
        else if ((c == ' ') || (c == '\t') || (c == '\n')) {
          save = FALSE;             // ignore white spaces. still in start state.
        }
        else {
          state = DONE;
          switch (c) {              // identify single char tokens.
            case EOF:
              save = FALSE;
              currentToken = ENDOFFILE;
              break;
            case '+':
              currentToken = PLUS;
              break;
            case '-':
              currentToken = MINUS;
              break;
            case '*':
              currentToken = TIMES;
              break;
            case '>':
              currentToken = GREATER;
              break;
            case '!':
              currentToken = NOT;
              break;
            case '.':
              currentToken = DOT;
              break;
            case ';':
              currentToken = SEMICOLON;
              break;
            case '{':
              currentToken = LBRACE;
              break;
            case '}':
              currentToken = RBRACE;
              break;
            case '(':
              currentToken = LPAREN;
              break;
            case ')':
              currentToken = RPAREN;
              break;
            default:
              currentToken = ERROR;
              break;
          }
        }
        break;
        
      case INCOMMENT:
        if (c == '/') {
          save = FALSE;
          state = START;
          currentTokenIndex = 0;
          getNextLine();            // in comment. skip to next line.
        }
        else {
          ungetNextChar();          // do not consume input.
          save = FALSE;
          state = DONE;
          currentToken = ERROR;
        }
        break;

      case INASSIGN:
        state = DONE;
        if (c == '=')
          currentToken = EQUALITY;
        else
        {
          ungetNextChar();          // is assign. do not consume input.
          save = FALSE;
          currentToken = ASSIGN;
        }
        break;

      case INNUM:
        if (!isdigit(c))            // keep collecting digits to tokenString until a non-digit is encountered.
        {
          ungetNextChar();          // do not consume input.
          save = FALSE;
          state = DONE;
          currentToken = NATLITERAL;
        }
        break;

      case INID:
        /** keep collecting alpha-numeric chars to tokenString until a whitespace or non-alpha-numeric input. */
        if ((c == ' ') || (c == '\t') || (c == '\n') || (!isalpha(c) && !isdigit(c)))
        {
          ungetNextChar();          // do not consume input.
          save = FALSE;
          state = DONE;
          currentToken = ID;
        }
        break;

      case INOR:
        state = DONE;
        if (c == '|')
          currentToken = OR;
        else
        {
          ungetNextChar();          // do not consume input.
          save = FALSE;
          currentToken = ERROR;
        }
        break;

      case DONE: /** Do nothing */

      default: /** should never happen */
        printf("Unhandled state = %d\n", state);
        state = DONE;
        currentToken = ERROR;
        break;
    }

    /** concat current input to tokenString */
    if ((save) && (currentTokenIndex <= MAXTOKENLEN)) {
      tokenString[currentTokenIndex++] = (char) c;
    }

    if (state == DONE) {
      tokenString[currentTokenIndex] = '\0';              // Terminate the string
      if (currentToken == ID) {
        /** Check if current ID is a reserved word and update currentToken accordingly. */
        currentToken = reservedLookup(tokenString);
      }
    }
  }
  currentTokenType.tok = currentToken;
  /** need to Allocate memory and copy string. */
  currentTokenType.str = malloc(strlen(tokenString) + 1);
  strcpy(currentTokenType.str, tokenString);
  currentTokenType.lineNo = lineNo;

  if(DEBUG) {
    printToken(currentTokenType);
  }
  return currentTokenType;
}
