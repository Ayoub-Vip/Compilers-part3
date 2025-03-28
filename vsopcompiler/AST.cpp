/*========================================================================= *
* @file AST.cpp
*
* @brief: This file is mplementation of the Abstract Syntax Tree (AST) classes.
*
* @authors: jamaa JAIR & Ayoub Assaoud
* @date  : 23-3-2025
* @projet: INFO0085 : Implementing a VSOP Compiler
*
* ========================================================================= */


/*========================================================================= *
* ========================= HERE WE START ================================= *
* ========================================================================= */

#include "AST.hpp"

// Default implementation for the toString method of the ASTNode base class
std::string ASTNode::toString() const{ return " ";}


/*=============================  Integers ============================== */
/**
* IntegerLiteral - Represents an integer literal value in the source program
*/
IntegerLiteral::IntegerLiteral(int value) : value(value) {}
   
/**
* Returns the stored integer value
*/
int IntegerLiteral::getValue() const {
   return value;
}

/**
* Converts the integer value to a string for AST representation
*/
std::string IntegerLiteral::toString() const {
   return std::to_string(value);
}

/*====================================================================== */

/*=============================  Strings ============================== */
/**
* StringLiteral - Represents a string literal in the source program
*/
StringLiteral::StringLiteral(const std::string &value) : str(value) {}

/**
* Returns the stored string value
*/
std::string StringLiteral::getString() const {
   return str;
}

/**
* Returns the string representation
*/
std::string StringLiteral::toString() const {
   return "\"" + str + "\"";
}
/*==================================================================== */

/*=============================  Booleans ============================== */
/**
* BooleanLiteral - Represents a boolean literal in the source program
*/
BooleanLiteral::BooleanLiteral(bool value) : value(value) {}

/**
* Returns the string representation of the boolean value
*/
std::string BooleanLiteral::toString() const {
   return value ? "true" : "false";
}

/**
* Returns the stored boolean value
*/
bool BooleanLiteral::getValue() const {
   return value;
}
/*====================================================================== */

/*=============================  Binary Operations ============================== */
/**
* BinaryOperation - Represents a binary operation between two expressions
*/
BinaryOperation::BinaryOperation(const std::string &op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
   : op(op), left(std::move(left)), right(std::move(right)) {}

/**
* Returns the operation symbol
*/
std::string BinaryOperation::getOperator() const {
   return op;
}

/**
* Returns a pointer to the left operand expression
*/
Expr* BinaryOperation::getLeft() const {
   return left.get();
}

/**
* Returns a pointer to the right operand expression
*/
Expr* BinaryOperation::getRight() const {
   return right.get();
}

/**
* Returns a string representation of the binary operation in the AST
*/
std::string BinaryOperation::toString() const {
   return "BinOp(" + op +", " + left->toString() + ", " + right->toString() + ")";
}
/*====================================================================== */


/*=============================  Conditional ============================== */
/**
* Conditional - Represents an if-then-else expression
*/
Conditional::Conditional(std::unique_ptr<Expr> cond_expr, std::unique_ptr<Expr> then_expr, std::unique_ptr<Expr> else_expr)
   : cond_expr(std::move(cond_expr)), then_expr(std::move(then_expr)), else_expr(std::move(else_expr)) {}

/**
* Returns a string representation of the conditional expression in the AST
*/
std::string Conditional::toString() const {
   return "If(" + cond_expr->toString() + ", " + then_expr->toString() + (else_expr ? ", " + else_expr->toString() : "") + ")";
}

/**
* Returns a pointer to the condition expression
*/
Expr* Conditional::getCond_expr() const {
   return cond_expr.get();
}

/**
* Returns a pointer to the "then" branch expression
*/
Expr* Conditional::getThen_expr() const {
   return then_expr.get();
}

/**
* Returns a pointer to the "else" branch expression
*/
Expr* Conditional::getElse_expr() const {
   return else_expr.get();
}
/*====================================================================== */

/*=============================  While ============================== */
/**
* WhileLoop - Represents a while loop construct
*/
WhileLoop::WhileLoop(std::unique_ptr<Expr> cond_expr, std::unique_ptr<Expr> body_expr)
   : cond_expr(std::move(cond_expr)), body_expr(std::move(body_expr)) {
       // std::cout << "WhileLoop" << std::endl;
   }

/**
* Returns a string representation of the while loop in the AST
*/
std::string WhileLoop::toString() const {
   return "While(" + cond_expr->toString() + ", " + body_expr->toString() + ")";
}

/**
* Returns a pointer to the condition expression
*/
Expr* WhileLoop::getCond_expr() const {
   return cond_expr.get();
}

/**
* Returns a pointer to the body expression
*/
Expr* WhileLoop::getBody_expr() const {
   return body_expr.get();
}

/*====================================================================== */

/*============================= Formula =================================*/
/**
* Formal - Represents a formal parameter in a method definition
*/
Formal::Formal(const std::string& n, const Type& t) : name(n), type(t) {}

/**
* Returns the parameter name
*/
std::string Formal::getName() const { return name; }

/**
* Returns the parameter type
*/
Type Formal::getType() const { return type; }
   
/**
* Returns a string representation of the formal parameter
*/
std::string Formal::toString() const {
   return name + " : " + type.toString() ;
}
/*====================================================================== */

// /*=============================  Block ============================== */
/**
* Block - Represents a block of expressions
*/
Block::Block(std::vector<std::unique_ptr<Expr>> exprs) : exprs(std::move(exprs)) {}

/**
* Returns the vector of expressions in this block
*/
std::vector<std::unique_ptr<Expr>>& Block::getExprs() {
   return exprs;
}

/**
* Adds an expression to the block
*/
void Block::addExpr(std::unique_ptr<Expr> expr) {
   if (!expr) {
       std::cerr << "Erreur : Tentative d'ajout d'un Expr null !" << std::endl;
       return;
   }
   exprs.push_back(std::move(expr));
}

/**
* Returns a string representation of the block
*/
std::string Block::toString() const {
   std::string result;
   if (exprs.size() > 0)
       result = "\n\t[";
   else
       result = "[";    
   
   int E_size = exprs.size();
   int cpt = 0;

   for (const auto& expr : exprs) {
       if (expr) {
           result += expr->toString();
           cpt++;
       } else {
           result += "null";
       }
       if (E_size - cpt > 0)
           result += ", ";
       
   }
   result += "]";
   return result;
}

/*====================================================================== */

/*=============================  MethodNode ============================== */
/**
* MethodNode - Represents a method definition in a class
*/
std::string MethodNode::toString() const {
   std::string paramStr = "[";
   for (size_t i = 0; i < formals.size(); ++i) {
       paramStr += formals[i]->toString();
       if (i != formals.size() - 1) paramStr += ", ";
   }
   paramStr += "]";
   return "Method(" + name + ", " + paramStr + ", " + returnType.toString() + ", " + bloc->toString() + ")";
}

/*====================================================================== */

/*========================== Expr ===================================== */
/**
* Base class for all expressions in the AST
*/
std::string Expr::toString() const { return " ";}

/*============================================================================ */
/*=========================== Type =========================================== */
/**
* Type - Represents a data type
*/
Type::Type(const std::string &name) : type_name(name) {}

/**
* Returns the type name
*/
std::string Type::getName() const { return type_name; }

/**
* Returns a string representation of the type
*/
std::string Type::toString() const {
   return  type_name;
}
/*====================================================================== */

/*=========================== Let =========================================== */
/**
* Let - Represents a let binding expression
*/
Let::Let(std::string n, Type t, std::unique_ptr<Expr> expr, std::unique_ptr<Expr> scope)
   : name(std::move(n)), type(std::move(t)), init_expr(std::move(expr)), scope_expr(std::move(scope)) {}

/**
* Returns a string representation of the let expression
*/
std::string Let::toString() const {
   return init_expr ? "Let(" + name + ", " + type.toString() + ", " + init_expr->toString() + ", " + scope_expr->toString() + ")"
                    : "Let(" + name + ", " + type.toString() + ", " + scope_expr->toString() + ")";
}

/**
* Returns the variable name
*/
std::string Let::getName() const {
   return name;
}

/**
* Returns the variable type
*/
Type Let::getType() const {
   return type.toString();
}

/**
* Returns a pointer to the initialization expression
*/
Expr* Let::getInitExpr() const {
   return init_expr.get();
}

/**
* Returns a pointer to the scope expression
*/
Expr* Let::getScopeExpr() const {
   return scope_expr.get();
}

/*==================================================================================== */

/*============================== Assign ============================================ */
/**
* Assign - Represents an assignment
*/
Assign::Assign(std::string n, std::unique_ptr<Expr> exprs): name(std::move(n)), expr(std::move(exprs)){}

/**
* Returns the variable name being assigned to
*/
std::string Assign::getName(){
   return name;
}

/**
* Returns a pointer to the expression being assigned
*/
Expr* Assign::getExpr() const{
   return expr.get();
}

/**
* Returns a string representation of the assignment
*/
std::string Assign::toString() const {
   return "Assign(" + name + ", " + expr->toString() + ")";
}
/* ================================================================================== */

/* ==========================   UnOp      =========================================== */
/**
* UnOp - Represents a unary operation
*/
UnOp::UnOp(std::string oper, std::unique_ptr<Expr> expr): op(std::move(oper)), expr(std::move(expr)) {}

/**
* Returns a string representation of the unary operation
*/
std::string UnOp::toString() const {
   return "UnOp(" + op + ", " + expr->toString() + ")"; 
}

/**
* Returns the operator
*/
std::string UnOp::getOp(){
   return op;
}

/**
* Returns a pointer to the operand expression
*/
Expr* UnOp::getExpr(){
   return expr.get();
}

/* ================================================================================== */

/* ============================ Call =============================================== */
/**
* Call - Represents a method call
*/
Call::Call(std::string n, std::vector<std::unique_ptr<Expr>> args, std::unique_ptr<Expr> exprobject_ident)
   : method_name(std::move(n)), args(std::move(args)), exprobject_ident(std::move(exprobject_ident)) {}

/**
* Returns the method name
*/
std::string Call::getMethodName() const {
   return method_name;
}

/**
* Returns a string representation of the method call
*/
std::string Call::toString() const {
   std::string result = "Call(" + exprobject_ident->toString() + ", " + method_name + ", [";
   for (size_t i = 0; i < args.size(); ++i) {
       result += args[i]->toString();
       if (i != args.size() - 1) result += ", ";
   }
   result += "])";
   return result;
}

/**
* Returns the vector of arguments
*/
std::vector<std::unique_ptr<Expr>>& Call::getArgs() {
   return args;
}

/* =========================== Field Node =========================================== */

/* =============================  ObjectIdentifier ================================== */
/**
* ObjectIdentifier - Represents a reference to an object or variable
*/
ObjectIdentifier::ObjectIdentifier(std::string n) : name(std::move(n)) {}

/**
* Returns a string representation of the identifier
*/
std::string ObjectIdentifier::toString() const {
   return name;
}

/**
* Returns the name
*/
std::string ObjectIdentifier::getName() const {
   return name;
}
/*================================================================================= */

/* =============================  Self ============================================= */
/**
* Self - Represents the "self" keyword
*/
Self::Self(std::string n) : name_self(std::move(n)) {}

/**
* Returns a string representation of self
*/
std::string Self::toString() const {
   return name_self;
}
/*================================================================================= */
/* =============================  Parenthesis ====================================== */
/**
* Parenthesis - Represents an empty pair of parentheses
*/
Parenthesis::Parenthesis() {}

/**
* Returns a string representation of the parentheses
*/
std::string Parenthesis::toString() const {
   return "()";
}
/*================================================================================= */

/* =============================  New ============================================= */
/**
* New - Represents object instantiation
*/
New::New(std::string n) : name(std::move(n)) {}

/**
* Returns a string representation of the new expression
*/
std::string New::toString() const {
   return "New(" + name + ")";
}
/*================================================================================= */
/* ====================== FieldNode ========================================== */
/**
* FieldNode - Represents a field declaration in a class
*/
FieldNode::FieldNode(std::string n, Type t, std::unique_ptr<Expr> expr)
   : name(std::move(n)), type(std::move(t)), init_expr(std::move(expr)) {}

/**
* Returns a string representation of the field
*/
std::string FieldNode::toString() const {
   return init_expr ? "Field(" + name + ", " + type.toString() + ", " + init_expr->toString() + ")"
                    : "Field(" + name + ", " + type.toString() + ")";
}

/**
* Returns the field name
*/
std::string FieldNode::getName() const {
   return name;
}

/**
* Returns the field type
*/
Type FieldNode::getType() const {
   return type.toString();
}

/*========================================================================== */

/* =========================== ClassNode =================================== */
/**
* ClassNode - Represents a class definition
*/
std::string ClassNode::toString() const {
   int cpt = 0;
   int f_size = fields.size();
   int m_size = methods.size();
   // std::cout << "Fiels : " << fields.size() << std::endl;
   std::string result = "Class(" + name + ", " + parent + ", [";
   
   for (auto it = fields.rbegin(); it != fields.rend(); ++it ){
       cpt++;
       result += (*it)->toString();
       if(f_size-cpt>0)
           result += ", ";
   }

   if(m_size) result += "], \n\t[";
   else result += "], [";

   cpt = 0;
   for (auto it = methods.rbegin(); it != methods.rend(); ++it) {
       cpt++;
       result += (*it)->toString();
       if(m_size - cpt>0)
           result += ", ";
   }
   result += "])";
   
   // std::cout<< "# methods : " << methods.size() << std::endl;
   return result;
}

/**
* Constructor for ClassNode
*/
ClassNode::ClassNode(std::string n, std::string p, std::vector<std::unique_ptr<FieldNode>>* f, std::vector<std::unique_ptr<MethodNode>>* m)
   : name(n), parent(p) {
       if (!f) {
           // std::cout << "WARNING: f est NULL, initialisation de fields avec un vector vide." << std::endl;
           fields = std::vector<std::unique_ptr<FieldNode>>();
       } else {
           // std::cout << "INFO: f contient " << f->size() << " éléments." << std::endl;
           fields = std::move(*f);
       }
       if (!m) {
           // std::cout << "WARNING: m est NULL, initialisation de methods avec un vector vide." << std::endl;
           methods = std::vector<std::unique_ptr<MethodNode>>();
       } else {
           // std::cout << "INFO: m contient " << m->size() << " éléments." << std::endl;
           methods = std::move(*m);
       }
   }


/**
* Adds a field to the class
*/
void ClassNode::addField(std::unique_ptr<FieldNode> field) {
   if (!field) {
       std::cerr << "Erreur : Tentative d'ajout d'un FieldNode null !" << std::endl;
       return;
   }
   fields.push_back(std::move(field));
   // std::cout << "Field ajouté !" << std::endl;
   // std::cout << "Fields : " << fields.size() << std::endl;
   // for (const auto& field : fields) std::cout << field->toString() << std::endl;
}
   
/**
* Adds a method to the class
*/
void ClassNode::addMethod(std::unique_ptr<MethodNode> method) {
   if (!method) {
       std::cerr << "Erreur : Tentative d'ajout d'un FieldNode null !" << std::endl;
       return;
   }
   methods.push_back(std::move(method));
}

/**
* Returns the vector of fields
*/
std::vector<std::unique_ptr<FieldNode>>& ClassNode::getFields() { 
   return fields;
}

/**
* Returns the vector of methods
*/
std::vector<std::unique_ptr<MethodNode>>& ClassNode::getMethods() { 
   return methods;
}
/*========================================================================== */
/* ================================== Program ===================================== */
/**
* Program - Represents the entire program
* The root node of the AST
*/

/**
* Adds a class to the program
*/
void Program::addClass(std::unique_ptr<ClassNode> cls){
   classes.push_back(std::move(cls));
}

/**
* Returns a string representation of the entire program
*/
std::string Program::toString() const {
   int cpt = 0;
   int C_size = classes.size();
   std::string str = "[";
   for (const auto& cls : classes)
   {
       cpt++;
       str += cls->toString();
       if (C_size - cpt > 0) str += ", \n";
       
   }
   str += "]";
   return str;  
}
/* ====================================================================================== */

/*========================================================================= *
* ========================= HERE WE FINISH ================================ *
* ========================================================================= */