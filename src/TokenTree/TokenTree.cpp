#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include "TokenTree.h"

extern int globalOffset;
extern int localOffset;
extern bool printMem;

TokenTree::TokenTree() {
    // Initializer for class... doesn't do anything now
}

void TokenTree::setTokenClass(int tc) {
    this->tokenClass = tc;
}

int TokenTree::getTokenClass() {
    return this->tokenClass;
}

void TokenTree::setLineNum(int line) {
    this->lineNum = line;
}

int TokenTree::getLineNum() {
    return this->lineNum;
}

void TokenTree::setTokenString(char *str) {
    this->tokenStr = strdup(str);
}

char *TokenTree::getTokenString() {
    return this->tokenStr;
}

void TokenTree::setCharValue(char c) {
    this->cvalue = c;
}

char TokenTree::getCharValue() {
    return this->cvalue;
}

void TokenTree::setNumValue(int n) {
    this->nvalue = n;
}

int TokenTree::getNumValue() {
    return this->nvalue;
}

void TokenTree::setStringValue(char *str) {
    setStringValue(str, true);
}

void TokenTree::setStringValue(char *str, bool duplicate) {
    if (duplicate) {
        this->svalue = strdup(str);
    } else {
        this->svalue = str;
    }
}

char *TokenTree::getStringValue() {
    return this->svalue;
}

void TokenTree::_setParent() {
    for (int i = 0; i < MAX_CHILDREN; i++) {
        TokenTree *child = children[i];
        if (child != NULL) {
            child->parent = this;
            child->_setParent();
        }
    }

    if (this->sibling != NULL) {
        sibling->parent = parent;
        sibling->_setParent();
    }
}

void TokenTree::_setFunction() {
    TokenTree *topParent = getTopParent();
    if (topParent->getDeclKind() == DeclKind::FUNCTION) { // All top parents must be declarations!
        function = topParent;
    }

    for (int i = 0; i < MAX_CHILDREN; i++) {
        TokenTree *child = children[i];
        if (child != NULL) child->_setFunction();
    }

    if (sibling != NULL) {
        sibling->_setFunction();
    }
}

void TokenTree::setParentAndFunction() {
    _setParent();
    _setFunction();
}

TokenTree *TokenTree::getTopParent() {
    TokenTree *visitor = this;
    while (visitor->parent != NULL) {
        visitor = visitor->parent;
    }

    return visitor;
}

int TokenTree::getNumSiblings(bool includeSelf) {
    int count = includeSelf ? 1 : 0;
    TokenTree *visitor = this;
    while (visitor->sibling != NULL) {
        visitor = visitor->sibling;
        count++;
    }
    return count;
}

int TokenTree::getNumChildren() {
    int counter = 0;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (children[i] != NULL) counter++;
    }
    return counter;
}

bool TokenTree::hasParent(TokenTree *possibleParent, bool checkAllParents) {
    if (possibleParent == NULL) return false;
    TokenTree *visitor = this->parent;
    while (visitor != NULL) {
        if (visitor == possibleParent) return true;
        visitor = visitor->parent;
    }
    return false;
}

void TokenTree::setNodeKind(NodeKind nk) {
    this->nodeKind = nk;
}

NodeKind TokenTree::getNodeKind() {
    return this->nodeKind;
}

void TokenTree::setDeclKind(DeclKind dk) {
    this->nodeKind = NodeKind::DECLARATION;
    this->subKind.declKind = dk;
}

DeclKind TokenTree::getDeclKind() {
    if (this->nodeKind != NodeKind::DECLARATION) {
        throw std::runtime_error("subKind is not declaration!");
    }
    return subKind.declKind;
}

void TokenTree::setExprKind(ExprKind ek) {
    this->nodeKind = NodeKind::EXPRESSION;
    this->subKind.exprKind = ek;
}

ExprKind TokenTree::getExprKind() {
    if (this->nodeKind != NodeKind::EXPRESSION) {
        throw std::runtime_error("subKind is not expression!");
    }
    return subKind.exprKind;
}

void TokenTree::setStmtKind(StmtKind sk) {
    this->nodeKind = NodeKind::STATEMENT;
    this->subKind.stmtKind = sk;
}

StmtKind TokenTree::getStmtKind() {
    if (this->nodeKind != NodeKind::STATEMENT) {
        throw std::runtime_error("subKind is not statement!");
    }
    return subKind.stmtKind;
}

void TokenTree::setExprType(ExprType et) {
    this->exprType = et;
}

ExprType TokenTree::getExprType() {
    return this->exprType;
}

const char *TokenTree::getTypeString() {
    switch (getExprType()) {
        case ExprType::BOOL:
            return "type bool";
        case ExprType::CHAR:
            return "type char";
        case ExprType::INT:
            return "type int";
        case ExprType::VOID:
            return "type void";
        case ExprType::UNDEFINED:
            return "undefined type";
    }
    return "error";
}

bool TokenTree::isExprTypeUndefined() {
    return exprType == ExprType::UNDEFINED;
}

bool TokenTree::checkCascade() {
    bool b = true;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        TokenTree *child = children[i];
        if (child != NULL && child->isExprTypeUndefined()) {
            b = false;
        }
    }
    return b;
}

void TokenTree::setExprName(char *name) {
    this->exprName = strdup(name);
}

char *TokenTree::getExprName() {
    return this->exprName;
}

void TokenTree::setIsArray(bool b) {
    this->_isArray = b;
}

bool TokenTree::isArray() {
    return this->_isArray;
}

void TokenTree::setIsStatic(bool b) {
    this->_isStatic = b;
}

bool TokenTree::isStatic() {
    return this->_isStatic;
}

void TokenTree::cancelCheckInit(bool applyToChildren) {
    this->checkInitialized = false;
    if (applyToChildren) {
        for (int i = 0; i < MAX_CHILDREN; i++) {
            TokenTree *child = children[i];
            if (child != NULL) child->cancelCheckInit(applyToChildren);
        }
    }
}

bool TokenTree::shouldCheckInit() {
    return !_isStatic && this->checkInitialized;
}

void TokenTree::setIsUsed(bool b) {
    if (this->getNodeKind() != NodeKind::DECLARATION) {
        throw std::runtime_error("Cannot set isUsed on node that is not a declaration.");
    }
    _isUsed = b;
}

bool TokenTree::isUsed() {
    return _isUsed;
}

void TokenTree::setIsInitialized(bool b) {
    if (this->getNodeKind() != NodeKind::DECLARATION) {
        throw std::runtime_error("Cannot set isInitialized on node that is not declaration.");
    }
    if (this->getDeclKind() == DeclKind::FUNCTION) {
        throw std::runtime_error("Cannot set isInitialized on node that is a function.");
    }
    _isInitialized = b;
}

bool TokenTree::isInitialized() {
    return _isInitialized;
}

void TokenTree::setHasReturn(bool b) {
    if (this->getNodeKind() != NodeKind::DECLARATION || this->getDeclKind() != DeclKind::FUNCTION) {
        throw std::runtime_error("Can only set 'hasReturn' on function declaration.");
    }
    _hasReturn = b;
}

bool TokenTree::hasReturn() {
    
    if (this->getNodeKind() != NodeKind::DECLARATION || this->getDeclKind() != DeclKind::FUNCTION) {
        throw std::runtime_error("Value of 'hasReturn' is only valid on function declaration.");
    }
    return _hasReturn;
}

bool TokenTree::isConstantExpression() {
    if (this->getNodeKind() != NodeKind::EXPRESSION) {
        throw std::runtime_error("Cannot only call 'isConstantExpression' on an expression.");
    }
    if (this->getExprKind() == ExprKind::CONSTANT) {
        return true;
    } else if (this->getExprKind() == ExprKind::ID || this->getExprKind() == ExprKind::CALL || this->getExprKind() == ExprKind::ASSIGN) {
        return false;
    } else { // ExprKind::OP
        bool allChildrenAreConst = true;
        for (int i = 0; i < MAX_CHILDREN; i++) {
            TokenTree *child = this->children[i];
            if (child != NULL && !child->isConstantExpression()) {
                allChildrenAreConst = false;
                break;
            }
        }
        return allChildrenAreConst;
    }
    
}

void TokenTree::addSibling(TokenTree *sibl) {
    if (sibl == NULL) return;
    TokenTree *visitor = this;
    while (visitor->sibling != NULL) {
        visitor = visitor->sibling;
    }
    visitor->sibling = sibl;
}

void TokenTree::typeSiblings(ExprType type) {
    TokenTree *node = this;
    while (node != NULL) {
        node->setExprType(type);
        node = node->sibling;
    }
}

void TokenTree::staticSiblings() {
    TokenTree *node = this;
    while (node != NULL) {
        node->setIsStatic(true);
        node = node->sibling;
    }
}

void TokenTree::_printTree(int level, bool isChild, bool isSibling, int num) {
    // Print self
    for (int i = 0; i < level; i++) {
        printf(".   ");
    }
    if (isChild || isSibling) {
        if (isChild) {
            printf("Child: ");
        } else {
            printf("Sibling: ");
        }
        printf("%d  ", num);
    }
    printNode();

    // Print children
    for (int i = 0; i < MAX_CHILDREN; i++) {
        TokenTree *child = children[i];
        if (child != NULL) {
            child->_printTree(level + 1, true, false, i);
        }
    }

    // Print sibling
    if (sibling != NULL) {
        if (isChild) {
            sibling->_printTree(level, false, true, 1);
        } else {
            sibling->_printTree(level, false, true, num + 1);
        }
    }
}

void TokenTree::printTree() {
    this->_printTree(0, false, false, 0);
}

void TokenTree::printNode() {
    // Welcome to switch city...
    switch (nodeKind) {
        case NodeKind::DECLARATION:
            switch (getDeclKind()) {
                case DeclKind::VARIABLE:
                    printf("Var %s: ", getStringValue());
                    if (isStatic()) printf("static ");
                    if (isArray()) printf("array of ");
                    printf("%s ", getTypeString());
                    break;
                case DeclKind::FUNCTION:
                    printf("Func %s: returns %s ", getStringValue(), getTypeString());
                    break;
                case DeclKind::PARAM:
                    printf("Param %s: ", getStringValue());
                    if (isArray()) printf("array of ");
                    printf("%s ", getTypeString());
                default:
                    break;
            }
            break;
        case NodeKind::EXPRESSION:
            switch (getExprKind()) {
                case ExprKind::ASSIGN: {
                    char *arrayStr = (char *) "";
                    if (isArray()) arrayStr = (char *) "array of ";
                    printf("Assign %s : %s%s ", getTokenString(), arrayStr, getTypeString());
                    break;
                }
                case ExprKind::CALL:
                    printf("Call %s: %s ", getExprName(), getTypeString());
                    break;
                case ExprKind::CONSTANT: {
                    printf("Const");
                    if (getExprType() == ExprType::CHAR) {
                        if (isArray()) {
                            printf(" \"");
                            fwrite(getStringValue(), sizeof(char), getNumValue(), stdout);
                            printf("\"");
                        } else {
                            printf(": \'%c\'", getCharValue());
                        }
                        printf(" : ");
                    } else {
                        printf(" %s : ", getStringValue());
                    }
                    char *arrayStr = (char *) "";
                    if (isArray()) arrayStr = (char *) "array of ";
                    printf("%s%s ", arrayStr, getTypeString());
                    break;
                }
                case ExprKind::ID: {
                    char *staticStr = (char *) "";
                    char *arrayStr = (char *) "";
                    if (isStatic()) staticStr = (char *) "static ";
                    if (isArray()) arrayStr = (char *) "array of ";
                    printf("Id %s: %s%s%s ", getStringValue(), staticStr, arrayStr, getTypeString());
                    break;
                }
                case ExprKind::OP:
                    printf("Op %s : %s ", getStringValue(), getTypeString());
                    break;
                default:
                    break;
            }
            break;
        case NodeKind::STATEMENT:
            switch (getStmtKind()) {
                case StmtKind::BREAK:
                    printf("Break ");
                    break;
                case StmtKind::COMPOUND:
                    printf("Compound ");
                    break;
                case StmtKind::FOR:
                    printf("For ");
                    break;
                case StmtKind::WHILE:
                    printf("While ");
                    break;
                case StmtKind::RETURN:
                    printf("Return ");
                    break;
                case StmtKind::SELECTION:
                    printf("If ");
                    break;
            }
            break;
        default:
            break;
    }
    if (printMem)
        printMemory();
    printLine();
    printf("\n");
}

void TokenTree::printLine() {
    printf("[line: %d]", getLineNum());
}

void TokenTree::printMemory() {
    if (this->getMemoryType() == MemoryType::UNDEFINED) return;
    printf("[mem: %s  ", getMemoryTypeString());
    if (!(this->getNodeKind() == NodeKind::DECLARATION && this->getDeclKind() == DeclKind::FUNCTION)) {
        printf("size: %d  ", getMemorySize());
    }
    printf("loc: %d] ", getMemoryOffset());
}

void TokenTree::setMemorySize(unsigned int i) {
    this->memorySize = i;
}

unsigned int TokenTree::getMemorySize() {
    return this->memorySize;
}

void TokenTree::setMemoryType(MemoryType mt) {
    this->memoryType = mt;
}

MemoryType TokenTree::getMemoryType() {
    return this->memoryType;
}

char *TokenTree::getMemoryTypeString() {
    switch (this->getMemoryType()) {
        case MemoryType::GLOBAL:
            return (char *) "Global";
        case MemoryType::LOCAL:
            return (char *) "Local";
        case MemoryType::LOCAL_STATIC:
            return (char *) "Static";
        case MemoryType::PARAM:
            return (char *) "Param";
        case MemoryType::UNDEFINED:
            return (char *) "Undefined";
        default:
            return (char *) "Invalid/Err";
    }
}

bool TokenTree::isInGlobalMemory() {
    if (this->getMemoryType() == MemoryType::UNDEFINED) {
        throw std::runtime_error("isInGlobalMemory called on node with undefined memory type!");
    }
    if (this->getMemoryType() == MemoryType::GLOBAL || this->getMemoryType() == MemoryType::LOCAL_STATIC) {
        return true;
    }
    return false;
}

void TokenTree::setMemoryOffset(int i) {
    this->memoryOffset = i;
}

int TokenTree::getMemoryOffset() {
    return this->memoryOffset;
}

void TokenTree::calculateMemoryOffset() {
    int *offset = this->isInGlobalMemory() ? &globalOffset : &localOffset; // Pick between local and global offset
    int location = *offset; // Copy offset
    if (this->isArray() && this->getMemoryType() != MemoryType::PARAM) location --; // Decrement offset if using an array
    this->setMemoryOffset(location); // Set memory offset to location
    (*offset) -= this->getMemorySize(); // Reduce offset by size of array
}

void TokenTree::copyMemoryInfo(TokenTree *tree) {
    if (this == tree) {
        throw std::runtime_error("Invalid pointer. Copy memory info called on self.");
    }
    this->setMemoryType(tree->getMemoryType());
    this->setMemorySize(tree->getMemorySize());
    this->setMemoryOffset(tree->getMemoryOffset());
}

int TokenTree::_calculateMemoryOfChildren() {
    int sum = 0;
    if (this->getNodeKind() == NodeKind::DECLARATION && this->getDeclKind() != DeclKind::FUNCTION && !this->isInGlobalMemory()) {
        sum += getMemorySize();
    }

    for (int i = 0; i < MAX_CHILDREN; i++) {
        TokenTree *child = this->children[i];
        if (child != NULL) {
            sum += child->_calculateMemoryOfChildren();
        }
    }

    if (this->sibling != NULL) {
        sum += sibling->_calculateMemoryOfChildren();
    }

    return sum;
}

void TokenTree::calculateMemoryOfChildren() {
    int sum = 0;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        TokenTree *child = this->children[i];
        if (child != NULL) {
            sum += child->_calculateMemoryOfChildren();
        }
    }

    this->setMemorySize(2 + sum);
}

bool TokenTree::wasGenerated() {
    return _wasGenerated;
}

void TokenTree::setGenerated() {
    this->setGenerated(true, false);
}

void TokenTree::setGenerated(bool b) {
    setGenerated(true, false);
}

void TokenTree::setGenerated(bool b, bool applyToChildren) {
    this->_wasGenerated = b;

    if (!applyToChildren) {
        return;
    }
    for (int i = 0; i < MAX_CHILDREN; i++) {
        TokenTree *child = this->children[i];
        if (child != NULL) {
            child->setGenerated(b, applyToChildren);
        }
    }
}

bool TokenTree::hasLastLine() {
    return _hasLastLine;
}

void TokenTree::setHasLastLine(bool b) {
    this->_hasLastLine = b;
}

int TokenTree::getLastLine() {
    return this->lastLine;
}

void TokenTree::setLastLine(int line) {
    this->lastLine = line;
}