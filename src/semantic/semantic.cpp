#include "string.h"

#include "symbolTable.h"

#include "semantic.h"
#include "TokenTree.h"

#define NUM_OPS 18

extern int numErrors;
extern int numWarnings;
extern int localOffset;
extern int globalOffset;
extern TokenTree *syntaxTree;
extern SymbolTable *symbolTable;

void err(TokenTree *node) {
    printf("ERROR(%d): ", node->getLineNum());
    numErrors++;
}

void warn(TokenTree *node) {
    printf("WARNING(%d): ", node->getLineNum());
    numWarnings++;
}

bool sameType(TokenTree *lhs, TokenTree *rhs) {
    return lhs->getExprType() == rhs->getExprType();
}

void handleBooleanComparison(TokenTree *tree) {
    tree->setExprType(ExprType::BOOL);
    TokenTree *lhs = tree->children[0];
    TokenTree *rhs = tree->children[1];
    if (!lhs->isExprTypeUndefined() && lhs->getExprType() != ExprType::BOOL) {
        err(tree);
        printf("'%s' requires operands of %s but lhs is of %s.\n", tree->getTokenString(), tree->getTypeString(), lhs->getTypeString());
    }
    if (!rhs->isExprTypeUndefined() && rhs->getExprType() != ExprType::BOOL) {
        err(tree);
        printf("'%s' requires operands of %s but rhs is of %s.\n", tree->getTokenString(), tree->getTypeString(), rhs->getTypeString());
    }
    
    if (lhs->isArray() || rhs->isArray()) {
        err(tree);
        printf("The operation '%s' does not work with arrays.\n", tree->getTokenString());
    }
}

void handleNot(TokenTree *tree) {
    tree->setExprType(ExprType::BOOL);
    TokenTree *lhs = tree->children[0];
    if (tree->checkCascade() && lhs->getExprType() != ExprType::BOOL) {
        err(tree);
        printf("Unary '%s' requires an operand of %s but was given %s.\n", tree->getTokenString(), tree->getTypeString(), lhs->getTypeString());
    }
    if (lhs->isArray()) {
            err(tree);
            printf("The operation '%s' does not work with arrays.\n", tree->getTokenString());
        }
}

void handleMath(TokenTree *tree) {
    tree->setExprType(ExprType::INT);
    TokenTree *lhs = tree->children[0];
    TokenTree *rhs = tree->children[1];
    if (!lhs->isExprTypeUndefined() && lhs->getExprType() != ExprType::INT) {
        err(tree);
        printf("'%s' requires operands of %s but lhs is of %s.\n", tree->getTokenString(), tree->getTypeString(), lhs->getTypeString());
    }
    if (!rhs->isExprTypeUndefined() && rhs->getExprType() != ExprType::INT) {
        err(tree);
        printf("'%s' requires operands of %s but rhs is of %s.\n", tree->getTokenString(), tree->getTypeString(), rhs->getTypeString());
    }
    if (lhs->isArray() || rhs->isArray()) {
        err(tree);
        printf("The operation '%s' does not work with arrays.\n", tree->getTokenString());
    }
}

void handleChSignOrMath(TokenTree *tree) {
    if (tree->children[1] != NULL) {
        handleMath(tree);
    } else {
        tree->setExprType(ExprType::INT);
        TokenTree *lhs = tree->children[0];
        if (tree->checkCascade() && lhs->getExprType() != ExprType::INT) {
            err(tree);
            printf("Unary '%s' requires an operand of %s but was given %s.\n", tree->getTokenString(), tree->getTypeString(), lhs->getTypeString());
        }
        if (lhs->isArray()) {
            err(tree);
            printf("The operation '%s' does not work with arrays.\n", tree->getTokenString());
        }
    }
}

void handleSizeOfOrMath(TokenTree *tree) {
    if (tree->children[1] != NULL) {
        handleMath(tree);
    } else { // Unary Deref
        TokenTree *lhs = tree->children[0];
        tree->setExprType(ExprType::INT);
        if (tree->checkCascade()) {
            if (!lhs->isArray()) {
                err(tree);
                printf("The operation '%s' only works with arrays.\n", tree->getTokenString());
            }
        }
    }
}

void handleUnaryInt(TokenTree *tree) {
    tree->setExprType(ExprType::INT);
    TokenTree *lhs = tree->children[0];
    if (lhs->getExprType() != ExprType::INT) {
        err(tree);
        printf("Unary '%s' requires an operand of %s but was given %s.\n", tree->getTokenString(), tree->getTypeString(), lhs->getTypeString());
    }
    if (lhs->isArray()) {
        err(tree);
        printf("The operation '%s' does not work with arrays.\n", tree->getTokenString());
    }
}

void handleRandom(TokenTree *tree) {
    TokenTree *lhs = tree->children[0];
    tree->setExprType(lhs->getExprType());
    if (lhs->getExprType() != ExprType::INT) {
        err(tree);
        printf("Unary '%s' requires an operand of %s but was given %s.\n", tree->getTokenString(), "type int", lhs->getTypeString());
    }
    if (lhs->isArray()) {
        err(tree);
        printf("The operation '%s' does not work with arrays.\n", tree->getTokenString());
    }
}

void handleComparison(TokenTree *tree) {
    tree->setExprType(ExprType::BOOL);
    TokenTree *lhs = tree->children[0];
    TokenTree *rhs = tree->children[1];
    if (tree->checkCascade()) {
        if (!sameType(lhs, rhs)) {
            err(tree);
            printf("'%s' requires operands of the same type but lhs is %s and rhs is %s.\n", tree->getTokenString(), lhs->getTypeString(), rhs->getTypeString());
        }
    }
    if (lhs->isArray() ^ rhs->isArray()) {
        char *lhsStr = (char*) "";
        char *rhsStr = (char*) "";
        if (!lhs->isArray()) lhsStr = (char*) " not";
        if (!rhs->isArray()) rhsStr = (char*) " not";
        err(tree);
        printf("'%s' requires both operands be arrays or not but lhs is%s an array and rhs is%s an array.\n", tree->getTokenString(), lhsStr, rhsStr);
    }
}

void handleArrayAccess(TokenTree *tree) {
    TokenTree *array = tree->children[0];
    tree->setExprType(array->getExprType());
    TokenTree *index = tree->children[1];
    if (array == NULL) {
        err(tree);
        printf("Cannot index nonarray.\n");
        return;
    }
    if (!array->isArray()) {
        err(tree);
        printf("Cannot index nonarray '%s'.\n", array->getStringValue());
    }
    if (!index->isExprTypeUndefined() && index->getExprType() != ExprType::INT) {
        err(tree);
        printf("Array '%s' should be indexed by type int but got %s.\n", array->getStringValue(), index->getTypeString());
    }
    if (index->isArray()) {
        err(tree);
        printf("Array index is the unindexed array '%s'.\n", index->getStringValue());
    }
}

const char *operations[NUM_OPS] = {
    "<=",
    "<",
    ">=",
    ">",
    "&",
    "|",
    "!",
    "+",
    "-",
    "*",
    "/",
    "%",
    "++",
    "--",
    "?",
    "==",
    "!=",
    "["
};
void (*functionPointers[NUM_OPS])(TokenTree *) = {
    handleComparison, // LEQ
    handleComparison, // LESS THAN
    handleComparison, // GEQ
    handleComparison, // GREATHER THAN
    handleBooleanComparison, // AND
    handleBooleanComparison, // OR
    handleNot, // NOT
    handleMath, // PLUS
    handleChSignOrMath, // CHANGE SIGN OR MINUS
    handleSizeOfOrMath, // SIZEOF OR TIMES
    handleMath, // DIVISION
    handleMath, // MOD
    handleUnaryInt, // INC
    handleUnaryInt, // DEC
    handleRandom, // RANDOM ?
    handleComparison, // EQUALS
    handleComparison, // NOT EQUALS
    handleArrayAccess, // ARRAY ACCESSOR []
};

int indexOfOperation(TokenTree *tree) {
    for (int i = 0; i < NUM_OPS; i++) {
        if (strcmp(tree->getTokenString(), operations[i]) == 0) {
            return i;
        }
    }
    return -1;
}

bool compoundShouldEnterScope(TokenTree *parent) {
    if (parent == NULL) {
        return true;
    }
    if (parent->getNodeKind() == NodeKind::DECLARATION && parent->getDeclKind() == DeclKind::FUNCTION) {
        return false;
    }
    if (parent->getNodeKind() == NodeKind::STATEMENT && parent->getStmtKind() == StmtKind::FOR) {
        return false;
    }
    return true;
}

/**
 * Handles typing and scoping of all nodes.
 * Also handles some error checking for symbols.
 */
void beforeChildren(TokenTree *tree, bool *enteredScope, int &previousLocalOffset) {
    if (tree->parent != NULL && tree->parent->getNodeKind() == NodeKind::EXPRESSION && tree->parent->getExprKind() == ExprKind::CALL) {
        TokenTree *res = (TokenTree *) symbolTable->lookup(tree->parent->getStringValue());
        if (res != NULL) {
            int counter = 1;
            TokenTree *param = res->children[0];
            TokenTree *input = tree->parent->children[0]; // Start back over in tree so we can tell the position
            while (input != tree && param != NULL) { // Travel accross siblings until we reach our desired input
                param = param->sibling; // Param is moved along with input to ensure matching
                input = input->sibling;
                counter++;
            }
            if (param == NULL && input == tree) { // param == null means we had more inputs than allowed. input == tree prevents duplicate errors
                err(tree);
                printf("Too many parameters passed for function '%s' declared on line %d.\n", res->getStringValue(), res->getLineNum());
            }
        }
    }
    switch (tree->getNodeKind()) {
        case NodeKind::DECLARATION: {
            if (tree->getDeclKind() != DeclKind::VARIABLE) {
                bool defined = !symbolTable->insert(tree->getStringValue(), tree);
                if (tree->getDeclKind() == DeclKind::FUNCTION) {
                    symbolTable->enter("Function: " + std::string(tree->getStringValue()));
                    *enteredScope = true;
                    localOffset = -2;
                } else { // Must be param
                    tree->setMemoryType(MemoryType::PARAM);
                    tree->calculateMemoryOffset();
                    tree->setIsInitialized(true);
                }
                if (defined) {
                    TokenTree *res = (TokenTree *) symbolTable->lookup(tree->getStringValue());
                    err(tree);
                    printf("Symbol '%s' is already declared at line %d.\n", res->getStringValue(), res->getLineNum());
                }
            } else {
                if (tree->parent == NULL) {
                    tree->setMemoryType(MemoryType::GLOBAL);
                } else if (tree->isStatic()) {
                    tree->setMemoryType(MemoryType::LOCAL_STATIC);
                } else {
                    tree->setMemoryType(MemoryType::LOCAL);
                }
                if (tree->children[0] != NULL) {
                    tree->setIsInitialized(true);
                }
            }            
            break;
        }
        case NodeKind::EXPRESSION: {
            switch (tree->getExprKind()) {
                case ExprKind::ASSIGN: {

                    break;
                }
                case ExprKind::CALL: {
                    TokenTree *res = (TokenTree *) symbolTable->lookup(tree->getStringValue());
                    if (res == NULL) {
                        err(tree);
                        printf("Function '%s' is not declared.\n", tree->getStringValue());
                    } else {
                        tree->setExprType(res->getExprType());
                        if (res->getDeclKind() != DeclKind::FUNCTION) {
                            err(tree);
                            printf("'%s' is a simple variable and cannot be called.\n", tree->getStringValue());
                            tree->setExprType(ExprType::UNDEFINED);
                        }
                    }
                    break;
                }
                case ExprKind::CONSTANT: {
                    if (tree->isArray()) {
                        tree->setMemoryType(MemoryType::GLOBAL);
                        tree->calculateMemoryOffset();
                    }
                    break;
                }
                case ExprKind::ID: {
                    TokenTree *res = (TokenTree *) symbolTable->lookup(tree->getStringValue());
                    if (res == NULL || (res->getDeclKind() == DeclKind::VARIABLE && tree->hasParent(res, true))) {
                        err(tree);
                        printf("Variable '%s' is not declared.\n", tree->getStringValue());
                    } else if (res->getDeclKind() != DeclKind::FUNCTION) {
                        res->setIsUsed(true);
                        tree->copyMemoryInfo(res);
                        tree->setExprType(res->getExprType());
                        tree->setIsArray(res->isArray());
                        tree->setIsStatic(res->isStatic());
                        tree->setMemoryType(res->getMemoryType());
                        if (tree->shouldCheckInit() && res->shouldCheckInit() && !res->isInitialized() && res->parent != NULL) {
                            warn(tree);
                            printf("Variable %s may be uninitialized when used here.\n", tree->getStringValue());
                            res->cancelCheckInit(false);
                        }
                    } else {
                        err(tree);
                        printf("Cannot use function '%s' as a variable.\n", tree->getStringValue());
                    }
                    break;
                }
            }
            break;
        }
        case NodeKind::STATEMENT: {
            switch (tree->getStmtKind()) {
                case StmtKind::COMPOUND: {
                    if (compoundShouldEnterScope(tree->parent)) {
                        *enteredScope = true;
                        symbolTable->enter("Compound Statement");
                        previousLocalOffset = localOffset;
                        break;
                    }
                    break;
                }
                case StmtKind::FOR: {
                    *enteredScope = true;
                    symbolTable->enter("For Statement");
                    previousLocalOffset = localOffset;
                    TokenTree *child = tree->children[0];
                    TokenTree *array = tree->children[1];
                    TokenTree *res = (TokenTree *) symbolTable->lookup(array->getStringValue());
                    if (res != NULL) {
                        child->setExprType(res->getExprType());
                    }
                    child->setIsInitialized(true);
                    break;
                }
                case StmtKind::BREAK: {
                    TokenTree *visitor = tree;
                    bool foundLoop = false;
                    while (visitor->parent != NULL) {
                        TokenTree *parent = visitor->parent;
                        if (parent->getNodeKind() == NodeKind::STATEMENT && (parent->getStmtKind() == StmtKind::WHILE || parent->getStmtKind() == StmtKind::FOR)) {
                            foundLoop = true;
                            break;
                        }
                        visitor = parent;
                    }

                    if (!foundLoop) {
                        err(tree);
                        printf("Cannot have a break statement outside of loop.\n");
                    }
                    break;
                }
            }
            break;
        }
    }
}

void afterChild(TokenTree *tree, int childNo) {
    switch (tree->getNodeKind()) {
        case NodeKind::STATEMENT: {
            switch (tree->getStmtKind()) {
                case StmtKind::FOR: {
                    if (childNo == 1) {
                        TokenTree *array = tree->children[1];
                        TokenTree *res = (TokenTree *) symbolTable->lookup(array->getStringValue());
                        if (res != NULL) {
                            res->setIsInitialized(true);
                        }
                        if (res == NULL || !res->isArray()) {
                            err(tree);
                            printf("For statement requires that symbol '%s' be an array to loop through.\n", array->getStringValue());
                        }
                    }
                    break;
                }
                case StmtKind::SELECTION: {
                    if (childNo == 0) {
                        TokenTree *condition = tree->children[0];
                        if (!condition->isExprTypeUndefined()) {
                            if (condition->getExprType() != ExprType::BOOL) {
                                err(tree);
                                printf("Expecting Boolean test condition in %s statement but got %s.\n", tree->getTokenString(), condition->getTypeString());
                            }
                            if (condition->isArray()) {
                                err(tree);
                                printf("Cannot use array as test condition in %s statement.\n", tree->getTokenString());
                            }
                        }
                    }
                    break;
                }
                case StmtKind::WHILE: {
                    if (childNo == 0) {
                        TokenTree *condition = tree->children[0];
                        if (!condition->isExprTypeUndefined()) {
                            if (condition->getExprType() != ExprType::BOOL) {
                                err(tree);
                                printf("Expecting Boolean test condition in %s statement but got %s.\n", tree->getTokenString(), condition->getTypeString());
                            }
                            if (condition->isArray()) {
                                err(tree);
                                printf("Cannot use array as test condition in %s statement.\n", tree->getTokenString());
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
}

void afterChildren(TokenTree *tree) {
    switch (tree->getNodeKind()) {
        case NodeKind::DECLARATION: {
            switch (tree->getDeclKind()) {
                case DeclKind::FUNCTION: {
                    if (tree->getExprType() != ExprType::VOID && !tree->hasReturn()) {
                        warn(tree);
                        printf("Expecting to return %s but function '%s' has no return statement.\n", tree->getTypeString(), tree->getStringValue());
                    }
                    tree->calculateMemoryOfChildren();
                    break;
                }
                case DeclKind::VARIABLE: {
                    bool defined = !symbolTable->insert(tree->getStringValue(), tree);
                    tree->calculateMemoryOffset();
                    TokenTree *child = tree->children[0];
                    if (tree->children[0] != NULL && !tree->children[0]->isConstantExpression()) {
                        err(tree);
                        printf("Initializer for variable '%s' is not a constant expression.\n", tree->getStringValue());
                    }
                    if (defined) {
                        TokenTree *res = (TokenTree *) symbolTable->lookup(tree->getStringValue());
                        err(tree);
                        printf("Symbol '%s' is already declared at line %d.\n", res->getStringValue(), res->getLineNum());
                    }
                    if (child != NULL && !child->isExprTypeUndefined() && !sameType(tree, child)) {
                        err(tree);
                        printf("Variable '%s' is of %s but is being initialized with an expression of %s.\n", tree->getStringValue(), tree->getTypeString(), child->getTypeString());
                    }
                    break;
                }
            }
            break;
        }
        case NodeKind::EXPRESSION: {
            switch (tree->getExprKind()) {
                case ExprKind::ASSIGN: {
                    TokenTree *lhs = tree->children[0];
                    TokenTree *rhs = tree->children[1];
                    if (rhs == NULL) { // INC/DEC
                        tree->setExprType(ExprType::INT);
                        if (tree->checkCascade()) {
                            if (lhs->getExprType() != ExprType::INT) {
                                err(tree);
                                printf("Unary '%s' requires an operand of %s but was given %s.\n", tree->getTokenString(), tree->getTypeString(), lhs->getTypeString());
                            }
                        }
                        
                        if (lhs->isArray()) {
                            err(tree);
                            printf("The operation '%s' does not work with arrays.\n", tree->getTokenString());
                        }
                    } else { // ASSIGN,ADDASS,SUBASS,MULASS,DIVASS
                        bool isAssign = (strcmp(tree->getStringValue(), "=") == 0);
                        if (isAssign) {
                            tree->setExprType(lhs->getExprType());
                            tree->setIsArray(lhs->isArray());
                            if (tree->checkCascade()) { // Prevent cascading errors
                                if (!sameType(lhs, rhs)) {
                                    err(tree);
                                    printf("'%s' requires operands of the same type but lhs is %s and rhs is %s.\n", tree->getTokenString(), lhs->getTypeString(), rhs->getTypeString());
                                }
                            }
                            if (lhs->isArray() ^ rhs->isArray()) {
                                char *lhsStr = (char*) "";
                                char *rhsStr = (char*) "";
                                if (!lhs->isArray()) lhsStr = (char*) " not";
                                if (!rhs->isArray()) rhsStr = (char*) " not";
                                err(tree);
                                printf("'%s' requires both operands be arrays or not but lhs is%s an array and rhs is%s an array.\n", tree->getTokenString(), lhsStr, rhsStr);
                            }
                        } else {
                            tree->setExprType(ExprType::INT);
                            if (!lhs->isExprTypeUndefined() && lhs->getExprType() != ExprType::INT) {
                                err(tree);
                                printf("'%s' requires operands of %s but lhs is of %s.\n", tree->getTokenString(), tree->getTypeString(), lhs->getTypeString());
                            }
                            if (!rhs->isExprTypeUndefined() && rhs->getExprType() != ExprType::INT) {
                                err(tree);
                                printf("'%s' requires operands of %s but rhs is of %s.\n", tree->getTokenString(), tree->getTypeString(), rhs->getTypeString());
                            }
                            if (lhs->isArray() || rhs->isArray()) {
                                err(tree);
                                printf("The operation '%s' does not work with arrays.\n", tree->getTokenString());
                            }
                        }
                        TokenTree *res;
                        if (lhs->getExprKind() == ExprKind::ID) {
                            res = (TokenTree *) symbolTable->lookup(lhs->getStringValue());
                        } else {
                            res = (TokenTree *) symbolTable->lookup(lhs->children[0]->getStringValue());
                        }
                        if (res == NULL || res->getDeclKind() == DeclKind::FUNCTION) break;
                        res->setIsInitialized(true);
                    }
                    break;
                }
                case ExprKind::CALL: {
                    TokenTree *res = (TokenTree *) symbolTable->lookup(tree->getStringValue());
                    if (res != NULL) {
                        TokenTree *param = res->children[0];
                        TokenTree *input = tree->children[0];
                        int numParams, numInputs;
                        if (param == NULL) numParams = 0; else numParams = param->getNumSiblings(true);
                        if (input == NULL) numInputs = 0; else numInputs = input->getNumSiblings(true);

                        // Param inputs handled at bottom of this function.
                        // Reasoning is described there
                        // We only check for too few parameters here
                        if (numParams > numInputs) {
                            err(tree);
                            printf("Too few parameters passed for function '%s' declared on line %d.\n", res->getStringValue(), res->getLineNum());
                        }
                        
                    }
                    break;
                }
                case ExprKind::OP: {
                    void (*fp)(TokenTree *) = functionPointers[indexOfOperation(tree)];
                    fp(tree);
                    break;
                }
                case ExprKind::ID: {                    
                    break;
                }
            }
            break;
        }
        case NodeKind::STATEMENT: {
            switch (tree->getStmtKind()) {
                case StmtKind::RETURN: {
                    tree->function->setHasReturn(true);
                    TokenTree *returnValue = tree->children[0];
                    bool returnNodeExists = returnValue != NULL;
                    bool expectingReturn = tree->function->getExprType() != ExprType::VOID;
                    if (returnNodeExists) {
                        if (expectingReturn) {
                            if (!returnValue->isExprTypeUndefined() && returnValue->getExprType() != tree->function->getExprType()) {
                                err(tree);
                                printf("Function '%s' at line %d is expecting to return %s but got %s.\n", tree->function->getStringValue(), tree->function->getLineNum(), tree->function->getTypeString(), returnValue->getTypeString());
                            }
                        } else { // Function does not expect return
                            err(tree);
                            printf("Function '%s' at line %d is expecting no return value, but return has return value.\n", tree->function->getStringValue(), tree->function->getLineNum());
                        }
                        if (returnValue->isArray()) {
                            err(tree);
                            printf("Cannot return an array.\n");
                        }
                    } else { // No return node exists
                        if (expectingReturn) {
                            err(tree);
                            printf("Function '%s' at line %d is expecting to return %s but return has no return value.\n", tree->function->getStringValue(), tree->function->getLineNum(), tree->function->getTypeString());
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    /**
     * It appears that type checking calls occurs in a counterintuitive way.
     * Rather than checking all inputs to the call after they have all been
     * processed, the test system seems to check inputs as soon as they have
     * been typed.
     * This requires us to catch each input to a call before moving on to
     * siblings if we want to output errors in the correct order.
     */
    if (tree->parent != NULL && tree->parent->getNodeKind() == NodeKind::EXPRESSION && tree->parent->getExprKind() == ExprKind::CALL) {
        TokenTree *res = (TokenTree *) symbolTable->lookup(tree->parent->getStringValue());
        if (res != NULL) {
            int counter = 1;
            TokenTree *param = res->children[0];
            TokenTree *input = tree->parent->children[0]; // Start back over in tree so we can tell the position
            while (input != tree && param != NULL) { // Travel accross siblings until we reach our desired input
                param = param->sibling; // Param is moved along with input to ensure matching
                input = input->sibling;
                counter++;
            }
            if (param != NULL) { // If param is null, then we had more inputs than function allowed
                if (!input->isExprTypeUndefined() && param->getExprType() != input->getExprType()) {
                    err(tree);
                    printf("Expecting %s in parameter %i of call to '%s' declared on line %d but got %s.\n", param->getTypeString(), counter, res->getStringValue(), res->getLineNum(), input->getTypeString());
                }
                if (param->isArray() && !input->isArray()) {
                    err(tree);
                    printf("Expecting array in parameter %i of call to '%s' declared on line %d.\n", counter, res->getStringValue(), res->getLineNum());
                } else if (!param->isArray() && input->isArray()) {
                    err(tree);
                    printf("Not expecting array in parameter %i of call to '%s' declared on line %d.\n", counter, res->getStringValue(), res->getLineNum());
                }
            }
        }
    }
}

void checkUsage(std::string, void *node) {
    TokenTree *tree = (TokenTree *) node;
    NodeKind nk = tree->getNodeKind();
    if (nk == NodeKind::DECLARATION && tree->getDeclKind() != DeclKind::FUNCTION) {
        if (!tree->isUsed()) {
            warn(tree);
            printf("The variable %s seems not to be used.\n", tree->getStringValue());
        }
    }
}

/**
 * 
 */
void buildSymbolTable(TokenTree *tree) {
    bool enteredScope = false;
    int previousLocalOffset = -2;

    /**
     * Recursing into tree
     * Building symbol table
     * Typing and scoping variables
     */
    beforeChildren(tree, &enteredScope, previousLocalOffset);
    

    for (int i = 0; i < MAX_CHILDREN; i++) {
        TokenTree *child = tree->children[i];
        if (child != NULL) {
            buildSymbolTable(child);
        }
        afterChild(tree, i);
    }

    /**
     * Recursing out of tree.
     * Children and siblings are all typed and scoped
     * Perform semantic analysis
     */
    afterChildren(tree);

    if (enteredScope) {
        symbolTable->applyToAll(checkUsage);
        symbolTable->leave();
        localOffset = previousLocalOffset;
    }

    if (tree->sibling != NULL) {
        buildSymbolTable(tree->sibling);
    }
    
}

void buildIORoutines() {
    TokenTree *outnl = new TokenTree();
    outnl->setDeclKind(DeclKind::FUNCTION);
    outnl->setLineNum(-1);
    outnl->setTokenString((char *) "outnl");
    outnl->setStringValue((char *) "outnl");
    outnl->setExprType(ExprType::VOID);

    TokenTree *inputc = new TokenTree();
    inputc->setDeclKind(DeclKind::FUNCTION);
    inputc->setLineNum(-1);
    inputc->setTokenString((char *) "inputc");
    inputc->setStringValue((char *) "inputc");
    inputc->setExprType(ExprType::CHAR);
    inputc->setHasReturn(true);
    inputc->addSibling(outnl);

    TokenTree *inputb = new TokenTree();
    inputb->setDeclKind(DeclKind::FUNCTION);
    inputb->setLineNum(-1);
    inputb->setTokenString((char *) "inputb");
    inputb->setStringValue((char *) "inputb");
    inputb->setExprType(ExprType::BOOL);
    inputb->setHasReturn(true);
    inputb->addSibling(inputc);

    TokenTree *input = new TokenTree();
    input->setDeclKind(DeclKind::FUNCTION);
    input->setLineNum(-1);
    input->setTokenString((char *) "input");
    input->setStringValue((char *) "input");
    input->setExprType(ExprType::INT);
    input->setHasReturn(true);
    input->addSibling(inputb);

    TokenTree *charDummy = new TokenTree();
    charDummy->setDeclKind(DeclKind::PARAM);
    charDummy->setLineNum(-1);
    charDummy->setTokenString((char *) "*dummy*");
    charDummy->setStringValue((char *) "*dummy*");
    charDummy->setExprType(ExprType::CHAR);
    charDummy->cancelCheckInit(false);
    charDummy->setIsUsed(true);

    TokenTree *outputc = new TokenTree();
    outputc->setDeclKind(DeclKind::FUNCTION);
    outputc->setLineNum(-1);
    outputc->setTokenString((char *) "outputc");
    outputc->setStringValue((char *) "outputc");
    outputc->setExprType(ExprType::VOID);
    outputc->children[0] = charDummy;
    outputc->addSibling(input);

    TokenTree *boolDummy = new TokenTree();
    boolDummy->setDeclKind(DeclKind::PARAM);
    boolDummy->setLineNum(-1);
    boolDummy->setTokenString((char *) "*dummy*");
    boolDummy->setStringValue((char *) "*dummy*");
    boolDummy->setExprType(ExprType::BOOL);
    boolDummy->cancelCheckInit(false);
    boolDummy->setIsUsed(true);

    TokenTree *outputb = new TokenTree();
    outputb->setDeclKind(DeclKind::FUNCTION);
    outputb->setLineNum(-1);
    outputb->setTokenString((char *) "outputb");
    outputb->setStringValue((char *) "outputb");
    outputb->setExprType(ExprType::VOID);
    outputb->children[0] = boolDummy;
    outputb->addSibling(outputc);

    TokenTree *intDummy = new TokenTree();
    intDummy->setDeclKind(DeclKind::PARAM);
    intDummy->setLineNum(-1);
    intDummy->setTokenString((char *) "*dummy*");
    intDummy->setStringValue((char *) "*dummy*");
    intDummy->setExprType(ExprType::INT);
    intDummy->cancelCheckInit(false);
    intDummy->setIsUsed(true);

    TokenTree *output = new TokenTree();
    output->setDeclKind(DeclKind::FUNCTION);
    output->setLineNum(-1);
    output->setTokenString((char *) "output");
    output->setStringValue((char *) "output");
    output->setExprType(ExprType::VOID);
    output->children[0] = intDummy;
    output->addSibling(outputb);

    buildSymbolTable(output);
}

void buildSymbolTable() {
    buildIORoutines();
    buildSymbolTable(syntaxTree);
    TokenTree *main = (TokenTree *) symbolTable->lookupGlobal("main");
    if (main == NULL || main->getDeclKind() != DeclKind::FUNCTION) {
        printf("ERROR(LINKER): Procedure main is not declared.\n");
        numErrors++;
    }
}