//  TM Code emitting utilities
//  implementation for the TINY compiler
//  Compiler Construction: Principles and Practice
//  Kenneth C. Louden
//  Modified slightly by Jorge L. Williams
//  Modified Nov 13, 2020 Robert Heckendorn
//
//  The two comment string forms of the calls allow you to easily
//  compose a comment from text and a symbol name for example. 
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emitcode.h"

extern FILE *code;


//  TM location number for current instruction emission
static int emitLoc = 0;   // next empty slot in Imem growing to lower memory
static int litLoc = 0;    // next empty slot in Dmem growing to higher memory


//  Procedure emitComment prints a comment line 
// with a comment that is the concatenation of c and d
// 
void emitComment(char *c, char *cc)
{
    fprintf(code, "* %s %s\n", c, cc);
}

//  Procedure emitComment prints a comment line 
// with comment c in the code file
// 
void emitComment(char *c)
{
    fprintf(code, "* %s\n", c);
}



// emitRO emits a REGISTER-ONLY TM instruction
// op = the opcode
// r = target register
// s = 1st source register
// t = 2nd source register
// c = a comment
// 
void emitRO(char *op, long long int r, long long int s, long long int t, char *c, char *cc)
{
    fprintf(code, "%3d:  %5s  %lld,%lld,%lld\t%s %s\n", emitLoc, op, r, s, t, c, cc);
    fflush(code);
    emitLoc++;
}

void emitRO(char *op,long long int r,long long int s,long long int t, char *c)
{
    emitRO(op, r, s, t, c, (char *)"");
}



// emitRM emits a REGISTER-TO-MEMORY TM instruction
// op = the opcode
// r = target register
// d = the offset
// s = the base register
// c = a comment
// 
void emitRM(char *op, long long int r, long long int d, long long int s, char *c, char *cc)
{
    fprintf(code, "%3d:  %5s  %lld,%lld(%lld)\t%s %s\n", emitLoc, op, r, d, s, c, cc);
    fflush(code);
    emitLoc++;
}

void emitRM(char *op,long long int r,long long int d,long long int s, char *c)
{
    emitRM(op, r, d, s, c, (char *)"");
}


void emitGoto(int d,long long int s, char *c, char *cc)
{
    emitRM((char *)"JMP", (long long int)PC, d, s, c, cc);
}


void emitGoto(int d,long long int s, char *c)
{
    emitGoto(d,  s, c, (char *)"");
}



// emitRMAbs converts an absolute reference 
// to a pc-relative reference when emitting a
// register-to-memory TM instruction
// op = the opcode
// r = target register
// a = the absolute location in memory
// c = a comment
// 
void emitRMAbs(char *op, long long int r, long long int a, char *c, char *cc)
{
    fprintf(code, "%3d:  %5s  %lld,%lld(%lld)\t%s %s\n", emitLoc, op, r, a - (long long int)(emitLoc + 1),
	    (long long int)PC, c, cc);
    fflush(code);
    emitLoc++;
}


void emitRMAbs(char *op,long long int r,long long int a, char *c)
{
    emitRMAbs(op, r, a, c, (char *)"");
}


void emitGotoAbs(int a, char *c, char *cc)
{
    emitRMAbs((char *)"JMP", (long long int)PC, a, c, cc);
}


void emitGotoAbs(int a, char *c)
{
    emitGotoAbs(a, c, (char *)"");
}


// emit a literal instructions
// IMPORTANT: assumes litterals are stored in low addresses in data memory
// string is stored from higher order address to lower order address!!
// 0  blah
// 1  blah
// 2  blah
// 3  e  <-- starting litLoc
// 4  s 
// 5  r
// 6  o
// 7  h  <-- return this address
// 8  5
// 9  blah   <-- ending litLoc

int emitLit(char *s)
{
    int loc;
    
    litLoc += strlen(s)-1;
    loc = litLoc;
    fprintf(code, "%3d:  %5s  \"%s\"\n", litLoc, (char *)"LIT", s);
    emitRM((char *)"LDC", 3, loc, 6, (char *)"Load address of literal char array");
    litLoc+=2;  // next empty spot which is past length

    return loc;
}

// load the literal at the address given
void emitLitAbs(int a, char *s)
{
    fprintf(code, "%3d:  %5s  \"%s\"\n", a, (char *)"LIT", s);
    emitRM((char *)"LDC", 3, a+strlen(s), 6, (char *)"Load literal value");
}


// 
//  Backpatching Functions
// 

// emitSkip skips "howMany" code
// locations for later backpatch.
// It also returns the current code position.
// emitSkip(0) tells you where you are and reserves no space.
// 
int emitSkip(int howMany)
{
    int i = emitLoc;
    emitLoc += howMany;

    return i;
}


// emitBackup backs up to 
// loc = a previously skipped location
// 
void emitBackup(int loc)
{
    emitLoc = loc;
}


// this back patches a LDA at the instruction address addr that
// jumps to the current instruction location now that it is known.
// This is essentially a backpatched "goto"
void backPatchAJumpToHere(int addr, char *comment)
{
    int currloc;

    currloc = emitSkip(0);          // remember where we are
    emitBackup(addr);               // go to addr
    emitGotoAbs(currloc, comment);  // the LDA to here
    emitBackup(currloc);            // restore addr
}


// this back patches a JZR or JNZ at the instruction address addr that
// jumps to the current instruction location now that it is known.
void backPatchAJumpToHere(char *cmd, int reg, int addr, char *comment)
{
    int currloc;

    currloc = emitSkip(0);          // remember where we are
    emitBackup(addr);               // go to addr
    emitRMAbs(cmd, reg, currloc, comment);  // cmd = JZR, JNZ
    emitBackup(currloc);            // restore addr
}

