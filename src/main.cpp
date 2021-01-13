/**
 * Main driver program for c- compiler.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "TokenTree.h"
#include "ourgetopt.h"
#include "symbolTable.h"
#include "yyerror.h"
#include "semantic.h"
#include "codegen/codegen.h"
#include "utils/utils.h"


// Globals
int numErrors = 0;
int numWarnings = 0;
int localOffset = -2;
int globalOffset = 0;
bool symtabDebug = false;
bool printMem = false;
TokenTree *syntaxTree;
SymbolTable *symbolTable;
FILE *code;

// External stuff
extern int yyparse();
extern int yydebug;
extern FILE *yyin;

// Prototypes

int main(int argc, char **argv) {
    extern int optind;
    bool printAST = false;
    char *fileName = NULL;
    char *outputFileName = NULL;
    int c;

    initErrorProcessing();

    while ((c = ourGetopt(argc, argv, (char *) "dhPMS")) != EOF) {
        switch (c) {
            case 'd':
                yydebug = true;
                break;
            case 'h':
                printf("Usage: c- [options] [sourceFile]\n");
                printf("  -d  turn on Bison debugging\n");
                printf("  -h  this usage message\n");
                printf("  -P  print abstract syntax tree + types\n");
                printf("  -M  print abstract syntax tree + types + memory info\n");
                printf("  -S  turn on symbol table debugging\n");
                return 0;
            case 'P':
                printAST = true;
                break;
            case 'M':
                printAST = true;
                printMem = true;
                break;
            case 'S':
                symtabDebug = true;
                break;
        }
    }
    
    if (optind < argc) {
        fileName = argv[optind];
        yyin = fopen(fileName, "r");
    }

    yyparse();


    if (numErrors == 0) {

        syntaxTree->setParentAndFunction();

        symbolTable = new SymbolTable();
        symbolTable->debug(symtabDebug);
        buildSymbolTable(); // Also performs semantic analysis

        if (printAST) {
            syntaxTree->printTree();
        }

        if (numErrors == 0) {
            if (fileName == NULL) {
                outputFileName = (char *) "out.tm";
            } else {
                int outLength = strlen(fileName) + 1;
                outputFileName = (char *) malloc(sizeof(char) * outLength);
                bstrcpy(outputFileName, outLength, fileName);
                outputFileName[outLength - 3] = 't';
                outputFileName[outLength - 2] = 'm';
            }
            code = fopen(outputFileName, "w");
            generateCode();
        }
    }
    
    printf("Number of warnings: %d\n", numWarnings);
    printf("Number of errors: %d\n", numErrors);
}