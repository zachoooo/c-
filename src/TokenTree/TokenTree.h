#ifndef TOKEN_TREE_H
#define TOKEN_TREE_H

#define MAX_CHILDREN 3
#include <stddef.h>

// A bunch of enums for tracking node info
enum class NodeKind { DECLARATION, EXPRESSION, STATEMENT };
enum class DeclKind { FUNCTION, VARIABLE, PARAM };
enum class ExprKind { CALL, CONSTANT, ID, OP, ASSIGN };
enum class StmtKind { COMPOUND, SELECTION, FOR, WHILE, RETURN, BREAK };
enum class ExprType { INT, BOOL, CHAR, VOID, UNDEFINED };
enum class MemoryType { LOCAL, LOCAL_STATIC, PARAM, GLOBAL, UNDEFINED };

/**
 * TokenTree is a single class utilized by both the scanner and the parser.
 * It starts off by just collecting token information during the initial
 * scanning phase. Once completed, the parser is able to determine more info
 * about the token and start building the AST in the tree information.
 * This whole thing was made to avoid the hassle of instantiating and copying
 * info from the scanner to the parser.
 */
class TokenTree {

    private:
        // Token Information
        int tokenClass;
        int lineNum;
        char *tokenStr;         // what string was actually read
        char cvalue;            // any character value
        int  nvalue;            // any numeric value or Boolean value
        char *svalue;           // any string value e.g. an id

        // Expression Information
        NodeKind nodeKind;
        union {
            DeclKind declKind;
            ExprKind exprKind;
            StmtKind stmtKind;
        } subKind;
        ExprType exprType = ExprType::UNDEFINED;
        char *exprName;
        bool _isArray = false;
        bool _isStatic = false;

        // Semantic information
        bool checkInitialized = true;
        bool _isUsed = false; // For varibles (maybe functions/params in future)
        bool _isInitialized = false; // For checking variable declarations
        bool _hasReturn = false; // For determining whether a function has a return value

        // Memory / Code Gen Information
        unsigned int memorySize = 1;
        MemoryType memoryType = MemoryType::UNDEFINED;
        int memoryOffset;
        bool _wasGenerated = false;
        bool _hasLastLine = false;
        int lastLine;

        void _printTree(int level, bool isChild, bool isSibling, int num);
        void _setParent();
        void _setFunction();
        int _calculateMemoryOfChildren();


    public:
        TokenTree();
        // Basic getters and setters of token info
        void setTokenClass(int tc);
        int getTokenClass();
        void setLineNum(int line);
        int getLineNum();
        void setTokenString(char *str);
        char *getTokenString();
        void setCharValue(char c);
        char getCharValue();
        void setNumValue(int n);
        int getNumValue();

        /**
         * Sets the string value for this node
         * 
         * This defaults to using strdup() to duplicate the string.
         * Use setStringValue(str, false) to avoid duplication.
         * 
         * @param str The string to use
         */
        void setStringValue(char *str);

        /**
         * Sets the string value for this node
         * 
         * @param str The string to use
         * @param duplicate Whether or not to duplicate the string or just set
         */
        void setStringValue(char *str, bool duplicate);
        char *getStringValue();
        
        // Tree Information
        TokenTree *children[3] = {NULL};
        TokenTree *parent = NULL;
        TokenTree *sibling = NULL;
        TokenTree *function = NULL;
        void setParentAndFunction();
        TokenTree *getTopParent();
        /**
         * Returns the number of siblings that a node has.
         * 
         * includeSelf determines whether or not the current node should count.
         */
        int getNumSiblings(bool includeSelf);
        int getNumChildren();
        /**
         * Checks if the given tree node is a parent to the node.
         * If checkAllParents is true, then it will continually check up the
         * tree to see if the parent is a grandparent, great grandparent, etc.
         */
        bool hasParent(TokenTree *possibleParent, bool checkAllParents);

        // Expression information
        void setNodeKind(NodeKind nk);
        NodeKind getNodeKind();
        void setDeclKind(DeclKind dk);
        DeclKind getDeclKind();
        void setExprKind(ExprKind ek);
        ExprKind getExprKind();
        void setStmtKind(StmtKind sk);
        StmtKind getStmtKind();
        void setExprType(ExprType et);
        ExprType getExprType();
        const char *getTypeString();
        bool isExprTypeUndefined();
        /**
         * Prevents cascading errors from occuring in type checking by
         * verifying that none of the children have undefined types.
         * 
         * Returns true if the cascade check passed.
         * Returns false if the cascade check failed.
         * A passed check means its ok to go ahead with further chcecks.
         * A failed check means you should not do more type checking.
         */
        bool checkCascade();
        void setExprName(char *name);
        char *getExprName();
        void setIsArray(bool b);
        bool isArray();
        void setIsStatic(bool b);
        bool isStatic();

        // Semantic information
        /**
         * Cancels initialization checks on this node.
         * This can be performed recursively on chidlren using the
         * applyToChildren param.
         */
        void cancelCheckInit(bool applyToChildren);
        /**
         * Whether or not we should check initialization on this node.
         * This defaults to true. Use cancelCheckInit to stop checking.
         */
        bool shouldCheckInit();
        void setIsUsed(bool b);
        bool isUsed();
        void setIsInitialized(bool b);
        bool isInitialized();
        void setHasReturn(bool b);
        bool hasReturn();
        bool isConstantExpression();

        /**
         * Adds the given Tree node as a sibling
         * 
         * @param sibl The Tree node to add as a sibling
         */
        void addSibling(TokenTree *sibl);
        
        /**
         * Specifies the expression type for all siblings including self
         * 
         * Useful when a type specifier applies to a whole list of variables
         * or parameters. 
         * 
         * @param type The desired expression type
         */
        void typeSiblings(ExprType type);

        /**
         * Sets all siblings to static including self
         * 
         * Useful when a static specifier applies to a whole list of
         * declarations.
         */
        void staticSiblings();

        /**
         * Entry point for printing the tree to stdout.
         * 
         * Fills out defaults for the private recursive _printTree function.
         */
        void printTree();

        /**
         * Print this specific node information. 
         */
        void printNode();
        void printLine();
        void printMemory();

        void setMemorySize(unsigned int i);
        unsigned int getMemorySize();
        void setMemoryType(MemoryType mt);
        MemoryType getMemoryType();
        char *getMemoryTypeString();
        bool isInGlobalMemory();
        void setMemoryOffset(int i);
        int getMemoryOffset();
        void calculateMemoryOffset();
        void copyMemoryInfo(TokenTree *tree);
        void calculateMemoryOfChildren();

        bool wasGenerated();
        void setGenerated();
        void setGenerated(bool b);
        void setGenerated(bool b, bool applyToChildren);
        bool hasLastLine();
        void setHasLastLine(bool b);
        int getLastLine();
        void setLastLine(int line);
};

#endif