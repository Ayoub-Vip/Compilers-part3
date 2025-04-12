/*========================================================================= *
* @file AST.hpp
*
* @brief: This file is the interface of AST
*
* @authors: jamaa JAIR & Ayoub Assaoud
* @date  : 23-3-2025
* @projet: INFO0085 : Implementing a VSOP Compiler
*
* ========================================================================= */

/*========================================================================= *
* ========================= HERE WE START ================================= *
* ========================================================================= */

#ifndef AST_H
#define AST_H

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

/**
 * Type - Represents a data type
 */
class Type{
    private:
        std::string type_name; // Valid types include: "int32", "bool", "string", "unit"
        unsigned int column;
        unsigned int line;
    
    public:
        Type(const std::string &name);
        Type(const std::string &name, unsigned int column, unsigned int line);
        
        std::string getName() const;
        unsigned int getColumn() const { return column; };
        unsigned int getLine() const { return line; };        
        std::string toString() const;
        std::string toString2() const;
};

/**
 * ASTNode - Base class for all nodes in the AST
 */
class ASTNode {
    protected:
        unsigned int column;
        unsigned int line;

    public:
        ASTNode() : column(0), line(0) {};
        ASTNode(unsigned int column, unsigned int line) : column(column), line(line) {};
        unsigned int getColumn() const { return column; };
        unsigned int getLine() const { return line; };
        virtual std::string toString() const = 0; // Method to be overridden
        virtual std::string toString2() const = 0; // Method to be overridden
};

/**
 * Expr - Base class for all expressions in the language
 * All expressions inherit from this class 
 */
class Expr {
    protected:
        Type type; // "int32", "bool", "string", "unit" or class-name
        unsigned int column;
        unsigned int line;

    public:
        Expr(const Type& t) : type(t), column(0), line(0) {};
        Expr() : type(Type("Undefined_return_type")), column(0), line(0){};
        Expr(const Type& t, unsigned int column, unsigned int line) : type(t), column(column), line(line) {};
        Expr(unsigned int column, unsigned int line) : type(Type("Undefined_return_type")), column(column), line(line) {};
        virtual ~Expr() = default;
        virtual std::string toString() const = 0;
        virtual std::string toString2() const = 0;
        std::string getTypeName() const { return type.getName(); };
        void setTypeByName(std::string t) { type = Type(t); };
        void setType(const Type& t) { type = t; };
        unsigned int getColumn() const { return column; };
        unsigned int getLine() const { return line; };
        void setColumn(unsigned int col) { column = col; };
        void setLine(unsigned int yyline) { line = yyline; };
    
};

/*=============================  Integers ============================== */
/**
 * IntegerLiteral - Represents an integer
 */
class IntegerLiteral : public Expr {
    public:
        IntegerLiteral(int value);
        IntegerLiteral(int value, unsigned int column, unsigned int line) : Expr(Type("int32"),column, line), value(value) {};
        int getValue() const;
        std::string toString() const override;
        std::string toString2() const override;
    
    private:
        int value;
};
/*====================================================================== */

/*=============================  Strings ============================== */
/**
 * StringLiteral - Represents a string
 */
class StringLiteral : public Expr {
    public:
        StringLiteral(const std::string &value);
        StringLiteral(const std::string &value, unsigned int column, unsigned int line) : Expr(Type("string"),column, line), str(value) {};
        std::string getString() const;
        std::string toString() const override;
        std::string toString2() const override;
    private:
        std::string str;
};
/*====================================================================== */

/*=============================  Booleans ============================== */
/**
 * BooleanLiteral - Represents a boolean
 */
class BooleanLiteral : public Expr {
    public:
        BooleanLiteral(bool value);
        BooleanLiteral(bool value, unsigned int column, unsigned int line) : Expr(Type("bool"),column, line), value(value) {};
        bool getValue() const;
        std::string toString() const override;
        std::string toString2() const override;
    private:
        bool value;
};
/*====================================================================== */

/*=============================  Binary Operations ============================== */
/**
 * BinaryOperation - Represents an operation with two operands
 */
class BinaryOperation : public Expr {
    public:
        BinaryOperation(const std::string &op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);
        BinaryOperation(const std::string &op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, unsigned int column, unsigned int line);
        std::string getOperator() const;
        Expr* getLeft() const;
        Expr* getRight() const;
        std::string toString() const override;
        std::string toString2() const override;
    private:
        std::string op;
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;
};
/*====================================================================== */

/*=============================  Conditional ============================== */
/**
 * Conditional - Represents an if-then-else
 */
class Conditional : public Expr {
    public:
        Conditional(std::unique_ptr<Expr> cond_expr, std::unique_ptr<Expr> then_expr, std::unique_ptr<Expr> else_expr = nullptr);
        std::string toString() const override;
        std::string toString2() const override;

        Expr* getCond_expr() const;
        Expr* getThen_expr() const;
        Expr* getElse_expr() const;

    private:
        std::unique_ptr<Expr> cond_expr; /**< Pointer to the condition expression. */
        std::unique_ptr<Expr> then_expr; /**< Pointer to the 'then' expression. */
        std::unique_ptr<Expr> else_expr; /**< Pointer to the 'else' expression. */
};
/*====================================================================== */

/*=============================  While ============================== */
/**
 * WhileLoop - Represents a while loop construct
 */
class WhileLoop : public Expr {
    public:
        WhileLoop(std::unique_ptr<Expr> cond_expr, std::unique_ptr<Expr> body_expr);
        std::string toString() const override;
        std::string toString2() const override;

        Expr* getCond_expr() const;
        Expr* getBody_expr() const;

    private:
        std::unique_ptr<Expr> cond_expr; /**< Pointer to the condition expression. */
        std::unique_ptr<Expr> body_expr; /**< Pointer to the body expression. */
};
/*====================================================================== */

/*=============================  Block ============================== */
/**
 * Block - Represents a block of expressions
 */
class Block : public Expr {
    public:
        Block() = default;
        Block(std::vector<std::unique_ptr<Expr>> exprs);
        std::string toString() const override;
        std::string toString2() const override;
        void addExpr(std::unique_ptr<Expr> expr);
        std::vector<std::unique_ptr<Expr>>& getExprs();

    private:
        std::vector<std::unique_ptr<Expr>> exprs;
};
/*====================================================================== */

/*============================= Formal ===================================== */
/**
 * Formal - Represents a formal parameter in a method declaration
 */
class Formal : public Expr {
private:
    std::string name;
    Type type;
    
public:
    Formal(const std::string& n, const Type& t);
    
    std::string getName() const;
    Type getType() const;
    
    std::string toString() const override;
    std::string toString2() const override;
};
/*====================================================================== */

/*=============================  Let  ============================== */
/**
 * Let - Represents a let
 */
class Let : public Expr {
    public:
        Let(std::string n, Type t, std::unique_ptr<Expr> expr = nullptr, std::unique_ptr<Expr> scope = nullptr);
        Let(std::string n, Type t, unsigned int column, unsigned int line, std::unique_ptr<Expr> expr = nullptr, std::unique_ptr<Expr> scope = nullptr);
        std::string toString() const override;
        std::string toString2() const override;
        std::string getName() const;
        Type getType() const;
        Expr* getInitExpr() const;
        Expr* getScopeExpr() const;

    private:
        std::string name;
        Type type;
        std::unique_ptr<Expr> init_expr;
        std::unique_ptr<Expr> scope_expr;
};
/*====================================================================== */

/*==========================  ASSIGN ===================================*/
/**
 * Assign - Represents an assignment operation
 */
class Assign : public Expr{
    public:
        Assign(std::string n, std::unique_ptr<Expr> expr = nullptr);
        Assign(std::string n, unsigned int column, unsigned int line,std::unique_ptr<Expr> expr = nullptr);
        std::string getName();
        Expr* getExpr() const;
        std::string toString() const override;
        std::string toString2() const override;

    private:
        std::string name;
        std::unique_ptr<Expr> expr;
};
/* ======================================================================= */

/* ============================  UnOp =================================== */
/**
 * UnOp - Represents a unary operation
 */
class UnOp : public Expr {
    public:
        UnOp(std::string op, std::unique_ptr<Expr> expr);
        std::string toString() const override;
        std::string toString2() const override;
        std::string getOp();
        Expr* getExpr();

    private:
        std::string op;
        std::unique_ptr<Expr> expr; 
};
/* ====================================================================== */

/* ============================  Call =================================== */
/**
 * Call - Represents a method call
 */
class Call : public Expr {
    public:
        Call(std::string n, std::vector<std::unique_ptr<Expr>> args, std::unique_ptr<Expr> exprobject_ident);
        Call(std::string n, std::vector<std::unique_ptr<Expr>> args, std::unique_ptr<Expr> exprobject_ident, 
            unsigned int column, unsigned int line);
        std::string toString() const override;
        std::string toString2() const override ;
        std::string getMethodName() const;
        std::vector<std::unique_ptr<Expr>>& getArgs();
        std::string getClassName() const;
        Expr* getExprObjectIdentifier() const {return exprobject_ident.get(); };

    private:
        std::string method_name;
        std::vector<std::unique_ptr<Expr>> args;
        std::unique_ptr<Expr> exprobject_ident;
};
/* ====================================================================== */

/*=============================  FieldNode ============================== */
/**
 * FieldNode - Represents a field declaration
 */
class FieldNode : public ASTNode {
    private:
        std::string name;
        Type type;
        std::unique_ptr<Expr> init_expr;
    
    public:
        FieldNode(std::string n, Type t, std::unique_ptr<Expr> expr = nullptr);
        FieldNode(std::string n, Type t, unsigned int column, unsigned int line, std::unique_ptr<Expr> expr = nullptr);
        std::string toString() const override;
        std::string toString2() const override;
        std::string getName() const;
        std::string getTypeName() {return type.getName(); };
        std::unique_ptr<Expr>& getInitExpr() { return init_expr; };
        Type getType() const;
};
/*======================================================================= */


/* ========================== ObjectIdentifier ========================== */
/**
 * ObjectIdentifier - Represents the object idebtifier 
 */
class ObjectIdentifier : public Expr {
    public:
        ObjectIdentifier(std::string n);
        ObjectIdentifier(std::string n, unsigned int column, unsigned int line);
        std::string toString() const override;
        std::string toString2() const override ;
        std::string getName() const;

    private:
        std::string name;
};
/*====================================================================== */

/*================================  Self ============================== */
/**
 * Self - Represents the "self"
 */
class Self : public Expr {
    public:
        Self() = default;
        Self(std::string n="self");
        std::string toString() const override;
        std::string toString2() const override;
    private:
        std::string name_self;
};
/*====================================================================== */

/*=============================  New ==================================== */
/**
 * New - Represents object instantiation
 */
class New : public Expr {
    public:
        New(std::string n);
        std::string toString() const override;
        std::string toString2()const override;
        std::string getClassName() const;

    private:
        std::string name;
};
/*====================================================================== */

/*=============================  Parenthesis ============================== */
/**
 * Parenthesis - Represents an empty pair of parentheses
 */
class Parenthesis : public Expr {
    public:
        Parenthesis();
        std::string toString() const override;
        std::string toString2() const override;
};
/*====================================================================== */

/*=============================  MethodNode ============================== */
/**
 * MethodNode - Represents a method definition in a class
 */
class MethodNode : public ASTNode {
    private:
        std::string name;
        Type returnType;
        std::vector<std::unique_ptr<Formal>> formals;
        std::unique_ptr<Block> bloc;
    
    public:
        /**
         * Constructor with formal parameters list
         * @param n Method name
         * @param rt Return type
         * @param params List of formal parameters
         * @param b Method body block
         */
        MethodNode(std::string n, Type rt,
            std::vector<std::unique_ptr<Formal>> params,
            std::unique_ptr<Block> b) :
            name(std::move(n)), returnType(std::move(rt)), formals(std::move(params)), bloc(std::move(b)) {}

        MethodNode(std::string n, Type rt,
            std::vector<std::unique_ptr<Formal>> params,
            std::unique_ptr<Block> b,
            unsigned int column, unsigned int line) : ASTNode(column, line),
            name(std::move(n)), returnType(std::move(rt)), formals(std::move(params)), bloc(std::move(b)) {}

        /**
         * Constructor for methods without parameters
         * @param n Method name
         * @param rt Return type
         * @param b Method body block
         */
        MethodNode(std::string n, Type rt, std::unique_ptr<Block> b)
        : name(std::move(n)), returnType(std::move(rt)), bloc(std::move(b)) {}

        MethodNode(std::string n, Type rt, std::unique_ptr<Block> b,
            unsigned int column, unsigned int line)
            : ASTNode(column, line), name(std::move(n)), returnType(std::move(rt)), bloc(std::move(b)) {}


            
        std::string toString() const override;
        std::string toString2() const override;
        std::string getName() { return name; };
        Type getReturnType() { return returnType; };
        std::vector<std::unique_ptr<Formal>>& getFormals() { return formals; };
        Block* getBlock() { return bloc.get(); };
};
/*====================================================================== */

/**
 * ClassNode - Represents a class definition
 */
class ClassNode : public ASTNode{
    public:
        std::string name; // class name 
        std::string parent; // parent
        std::vector<std::unique_ptr<FieldNode>> fields; // class fields
        std::vector<std::unique_ptr<MethodNode>> methods; // class methods

        /**
         * Constructor for ClassNode
         * @param name The class name
         * @param parent The parent class name
         * @param f Pointer to a vector of field nodes
         * @param M Pointer to a vector of method nodes
         */
        ClassNode(std::string name, std::string parent, std::vector<std::unique_ptr<FieldNode>>* f, std::vector<std::unique_ptr<MethodNode>>* M);

        ClassNode(std::string name, std::string parent, std::vector<std::unique_ptr<FieldNode>>* f, std::vector<std::unique_ptr<MethodNode>>* M,
        unsigned int column, unsigned int line);
        
        /**
         * Adds a field to the class
         * @param field The field node to add
         */
        void addField(std::unique_ptr<FieldNode> field);
    
        /**
         * Adds a method to the class
         * @param method The method node to add
         */
        void addMethod(std::unique_ptr<MethodNode> method);

        /**
         * Returns the vector of fields
         */
        std::vector<std::unique_ptr<FieldNode>>& getFields();

        /**
         * Returns the vector of methods
         */
        std::vector<std::unique_ptr<MethodNode>>& getMethods();

        std::string toString() const override;
        std::string toString2() const override;
};

/* ============================ Program ================================ */
/**
 * Program - Represents the entire program
 */
class Program : public ASTNode {
    public :
        Program() = default;
        
        /**
         * Adds a class to the program
         * @param cls The class node to add
         */
        void addClass(std::unique_ptr<ClassNode> cls);
        std::vector<std::unique_ptr<ClassNode>>& getClasses();
        
        std::string toString() const override;
        std::string toString2() const override;

    private :
        std::vector<std::unique_ptr<ClassNode>> classes;
};
/* ======================================================================== */
#endif //AST_H

/*========================================================================= *
* ========================= HERE WE FINISH ================================ *
* ========================================================================= */