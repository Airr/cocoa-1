//
//  main.c
//  alder-newick
//
//  Created by Sang Chul Choi on 9/9/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main0(void);

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    main0();
    return 0;
}

// The token buffer. We never check for overflow! Do so in production code.
char buf[1024];
int n = 0;

// The current character.
int ch;

// The look-ahead token.  This is the 1 in LL(1).
enum { LEFT_PAREN, RIGHT_PAREN, NUMBER, END_INPUT,
    LABEL, COMMA_OP, COLON_OP, SEMICOLON_OP} look_ahead;

// Forward declarations.
void init(void);
void advance(void);
double expr(void);
void error(char *msg);
double subtree();
double label();
double tree();

// Parse expressions, one per line.
int main0(void)
{
    init();
    while (1) {
        double val = tree();
        printf("val: %f\n", val);
        if (look_ahead != SEMICOLON_OP) error("junk after expression");
        advance();  // past end of input mark
        if (look_ahead != END_INPUT) error("junk after expression");
        advance();  // past end of input mark
    }
    return 0;
}

// Just die on any error.
void error(char *msg)
{
    fprintf(stderr, "Error: %s. I quit.\n", msg);
    exit(1);
}

// Buffer the current character and read0 a new one.
void read0()
{
    buf[n++] = ch;
    buf[n] = '\0';  // Terminate the string.
    ch = getchar();
}

// Ignore the current character.
void ignore()
{
    ch = getchar();
}

// Reset the token buffer.
void reset()
{
    n = 0;
    buf[0] = '\0';
}

// The scanner.  A tiny deterministic finite automaton.
int scan()
{
    reset();
START:
    switch (ch) {
        case ' ': case '\t': case '\r':
            ignore();
            goto START;
            
        case ',':
            read0();
            return COMMA_OP;
            
        case '(':
            read0();
            return LEFT_PAREN;
            
        case ')':
            read0();
            return RIGHT_PAREN;
            
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            read0();
            goto IN_LEADING_DIGITS;
            
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            read0();
            goto IN_LEADING_ALPHABET;
            
        case '\n':
            ch = ' ';    // delayed ignore()
            return END_INPUT;
            
        case ';':
            ch = ' ';
            return SEMICOLON_OP;
            
        case ':':
            ch = ' ';
            return COLON_OP;
            
        default:
            error("bad character");
    }
    
IN_LEADING_DIGITS:
    switch (ch) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            read0();
            goto IN_LEADING_DIGITS;
            
        case '.':
            read0();
            goto IN_TRAILING_DIGITS;
            
        default:
            return NUMBER;
    }
    
IN_TRAILING_DIGITS:
    switch (ch) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            read0();
            goto IN_TRAILING_DIGITS;
            
        default:
            return NUMBER;
    }
    
IN_LEADING_ALPHABET:
    while (isalpha(ch)) {
        read0();
    }
    return LABEL;
}

// To advance is just to replace the look-ahead.
void advance()
{
    look_ahead = scan();
}

// Clear the token buffer and read0 the first look-ahead.
void init()
{
    reset();
    ignore(); // junk current character
    advance();
}

// expr = subtree { "," subtree }
double expr()
{
    double rtn = subtree();
    while (look_ahead == COMMA_OP) {
        advance();
        rtn += subtree();
    }
    return rtn;
}

double branchLength()
{
    double rtn = 0;
    sscanf(buf, "%lf", &rtn);
    return rtn;
}

double subtree()
{
    double rtn = 0;
    switch (look_ahead) {
            
        case LEFT_PAREN:
            advance();
            rtn = expr();
            if (look_ahead != RIGHT_PAREN) error("missing ')'");
            advance();
            if (look_ahead == LABEL) {
                printf("LABEL: %s\n", buf);
            }
            break;
            
        case LABEL:
            printf("LABEL: %s\n", buf);
            //            sscanf(buf, "%lf", &rtn);
            advance();
            break;
        default:
            error("unexpected token");
    }
    
    if (look_ahead == COLON_OP) {
        advance();
        rtn += branchLength();
        advance();
    }
    
    return rtn;
}

double tree()
{
    return subtree();
}

double label()
{
    return 0;
}