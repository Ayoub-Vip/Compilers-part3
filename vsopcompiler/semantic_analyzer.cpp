#include "AST.hpp"
#include "symbol_table.cpp"

#include <unordered_map>
#include <string>
#include <iostream>
#include <functional>

// Type Checking: Ensure expressions have valid types and operations are applied to compatible types.
// Scope Checking: Validate variable and method declarations and ensure identifiers are used within their declared scope.
// Inheritance Rules: Ensure class inheritance is valid (e.g., no circular inheritance).
// Method Overriding: Check that overridden methods have compatible signatures.
// Field Initialization: Ensure fields are properly initialized.

class SemanticAnalyzer {
public:
    void analyze(Program* program) {

        checkClassInhiretence(program->getClasses());
        

        // Perform semantic checks on the entire program
        for (auto& cls : program->getClasses()) {
            checkClass(cls.get());
            // cls.get()->name; //check Class inhireteanc / iscyclic ?
        }

    }
    bool isAccepted = false;

private:
    SymbolTable symb_tab = SymbolTable();
    std::unordered_map<std::string, ClassNode*> classMap; ///TODO to be curfull here


    void checkClassInhiretence(const std::vector<std::unique_ptr<ClassNode>>& classes) {
        /***
         * check wether all the extended (parent) class exists ........... Done
         *  check if there no is a cyclic definition of classes .......... Done
         *  class initializer do not contain scope of fields themselves ......... //TODO
         *  no class is defined more than one time ....................... Done
         *  no class is named Object ..................................... Done
         *  report error:1:1 if no class Main, or Main has no main method. Done
         *  and check main args (signature) if are good .................. Done
         ***/

        std::unordered_map<std::string, bool> visited;

        // Populate class map and check for duplicate class definitions
        for (auto& cls : classes) {
            if (classMap.find(cls->name) != classMap.end()) {
                std::cerr << "Error: Class " << cls->name << " is defined more than once." << std::endl;
                continue;
            }
            if (cls->name == "Object") {
                std::cerr << "Error: Class cannot be named 'Object'." << std::endl;
                continue;
            }
            for (const std::string& s : {"int32", "bool", "string", "unit"}) {//TODO this is additional from my mind
                if (s == cls->name)
                std::cerr << "Error: Class cannot be named 'int32', 'bool', 'string', or 'unit'." << std::endl;
                continue;
            }
            classMap[cls->name] = cls.get();
            visited[cls->name] = false;
        }

        // Check for cyclic inheritance
        std::function<bool(const std::string&)> isCyclic = [&](const std::string& className) {
            if (visited[className]) return true;
            visited[className] = true;

            auto cls = classMap[className];
            if (cls->parent != "" && classMap.find(cls->parent) != classMap.end()) {
                if (isCyclic(cls->parent)) {
                    std::cerr << "Error: Cyclic inheritance detected involving class " << className << "." << std::endl;
                    return true;
                }
            }

            visited[className] = false;
            return false;
        };

        for (auto& cls : classes) {
            if (isCyclic(cls->name)) {
                // Already reported in the recursive function
                continue;
            }
        }

        // Check for undefined parent classes
        for (auto& cls : classes) {
            if (!cls->parent.empty() && classMap.find(cls->parent) == classMap.end()) {
                std::cerr << "Error: Parent class " << cls->parent << " of class " << cls->name << " is not defined." << std::endl;
            }
        }

        // Check for the existence of class Main
        auto mainIt = classMap.find("Main");
        if (mainIt == classMap.end()) {
            std::cerr << "Error: No class 'Main' defined." << std::endl;
            return;
        }

        // Check if Main has a main method with the correct signature
        auto mainClass = mainIt->second;
        bool hasMainMethod = false;
        for (auto& method : mainClass->getMethods()) {
            if (method->getName() == "main" 
                && method->getFormals().empty()
                && method->getReturnType().getName() == "int32"
            ) {
                hasMainMethod = true;
                break;
            }
        }

        if (!hasMainMethod) {
            std::cerr << "Error: Class 'Main' must have a 'main' method with no arguments and return type 'int32'." << std::endl;
        }
    }
    
    // Check class-level semantics : fields and method can not be declared two times.
    void checkClass(ClassNode* cls) {
        symb_tab.enterScope();
        std::cout << "Checking class: " << cls->name << std::endl;

        for (auto& field : cls->getFields()) {
            // ensure 'new' Expr has an exiting class declared. )
            // if extending a parent class -> cannot redfine its fields (with different type..).
            symb_tab.declare(field->getName(), field->getTypeName());

            checkField(field.get());
        }

        for (auto& method : cls->getMethods()) {
            // if extending a parent class, must have the same methods args and return type (another type..).
            if (cls->parent != "Object")
                compareMethodsSignature(cls, classMap[cls->parent]);

            ///////// check scopes? where? //////////////
            checkMethod(method.get());

        }
        symb_tab.enterScope();
    }

    // Check field-level semantics (e.g., type validity)
    void checkField(FieldNode* field) {
        // check if the type is existing, ................................. Done
        // ex: { field : classA}, classA must be declared
        std::string ftype = field->getTypeName();

        for (const std::string& s : {"int32", "bool", "string", "unit"}) {
            if (s == ftype)
                return;
        }

        std::cout << "Checking field: " << field->getName() << std::endl;
        if (classMap.count(ftype) == 0)
            std::cerr << "semantic error : class type" << ftype << "does not exist" << std::endl;
    }
    
    // if extending a parent class, must have the same methods args and return type (another type..).
    void compareMethodsSignature(ClassNode* child, ClassNode* parent){
        for (auto& pmethod : parent->getMethods()) {
            for (auto& cmethod : child->getMethods()) {
                if(pmethod->getName() == cmethod->getName()) {
                    if (cmethod->getReturnType().getName() != pmethod->getReturnType().getName()){
                        std::cerr << "semantic error : parent class method return type is not the same as the child return type" << std::endl;
                        
                    }
                    if (cmethod->getFormals().size() != pmethod->getFormals().size()) {
                        std::cerr << "semantic error : parent class method signature is not the same as the child signature" << std::endl;
                        return;
                    }
                    for (size_t i = 0; i < cmethod->getFormals().size() - 1; ++i) {
                        if (cmethod->getFormals()[i]->getName() == cmethod->getFormals()[i]->getName()) {
                            continue;
                        } else {
                            std::cerr << "semantic error : parent class method signature names is not the same as the child" << std::endl;
                        }
                        if (cmethod->getFormals()[i]->getTypeName() == cmethod->getFormals()[i]->getTypeName()) {
                            continue;
                        } else {
                            std::cerr << "semantic error : parent class method signature type is not the same as the child" << std::endl;
                        }
                    }
                }
            }
        }
    }

    // Check method-level semantics (e.g., parameter types, return type)
    void checkMethod(MethodNode* method) {
        // check expressions Scope and types

        symb_tab.enterScope();

        // no several formal arguments with the same name 
        std::cout << "Checking method: " << method->toString() << std::endl;
        std::unordered_map<std::string, bool> visitedFromalName;
        for (auto& formal : method->getFormals()) {
            std::string fname = formal->getName();
            std::string ftype = formal->getTypeName();
            if(visitedFromalName.find(fname) != visitedFromalName.end()){
                
                visitedFromalName[fname] = true;

                symb_tab.declare(fname, ftype);  //TODO what if a formal is already in the previous scope with  different type??
            }
            else {
                std::cerr << "semantic error : the method " << method->toString() << "has several formals with the same name'" << fname <<"'" << std::endl;

            }
        }

        symb_tab.exitScope();

    }

    void checkExpression(Expr* expr) {
        if (auto binOp = dynamic_cast<BinaryOperation*>(expr)) {
            checkExpression(binOp->getLeft());
            checkExpression(binOp->getRight());
    
            if (binOp->getLeft()->getTypeName() != "int32" || binOp->getRight()->getTypeName() != "int32") {
                std::cerr << "Type error: Binary operation requires int32 operands." << std::endl;
            }
    
            binOp->setTypeByName("int32"); // Assume result is int32
        } else if (auto cond = dynamic_cast<Conditional*>(expr)) {
            checkExpression(cond->getCond_expr());
            checkExpression(cond->getThen_expr());
            if (cond->getElse_expr()) {
                checkExpression(cond->getElse_expr());
            }
    
            if (cond->getCond_expr()->getTypeName() != "bool") {
                std::cerr << "Type error: Condition must be of type bool." << std::endl;
            }
    
            // Assume the type of the conditional is the type of the "then" branch
            cond->setTypeByName(cond->getThen_expr()->getTypeName());

            // branchs of: {if (condition) then 42 else false}  should return the same type not int32 or bool
        } else if (auto call = dynamic_cast<Call*>(expr)) {
            // Verify if called method is set with appropriate arguments
            for (auto& arg : call->getArgs()) {
                checkExpression(arg.get());
            }
        } else if (auto assign = dynamic_cast<Assign*>(expr)) {
            checkExpression(assign->getExpr());
            // Verify if the type of the assigned expression matches the variable type
        } else if (auto intLiteral = dynamic_cast<IntegerLiteral*>(expr)) {
            intLiteral->setTypeByName("int32");
        } else if (auto strLiteral = dynamic_cast<StringLiteral*>(expr)) {
            strLiteral->setTypeByName("string");
        } else if (auto boolLiteral = dynamic_cast<BooleanLiteral*>(expr)) {
            boolLiteral->setTypeByName("bool");
        } else if (auto whileLoop = dynamic_cast<WhileLoop*>(expr)) {
            checkExpression(whileLoop->getCond_expr());
            checkExpression(whileLoop->getBody_expr());
            if (whileLoop->getCond_expr()->getTypeName() != "bool") {
                std::cerr << "Type error: While loop condition must be of type bool." << std::endl;
            }
            whileLoop->setTypeByName("unit");
        } else if (auto block = dynamic_cast<Block*>(expr)) {
            for (auto& innerExpr : block->getExprs()) {
                checkExpression(innerExpr.get());
            }
            if (!block->getExprs().empty()) {
                block->setTypeByName(block->getExprs().back()->getTypeName());
            } else {
                block->setTypeByName("unit");
            }
        } else if (auto let = dynamic_cast<Let*>(expr)) {
            if (let->getInitExpr()) {
                checkExpression(let->getInitExpr());
            }
            if (let->getScopeExpr()) {
                checkExpression(let->getScopeExpr());
            }
            let->setTypeByName(let->getScopeExpr()->getTypeName());
        } else if (auto unOp = dynamic_cast<UnOp*>(expr)) {
            checkExpression(unOp->getExpr());
            // Verify the type of the operand and set the result type
        } else if (auto objId = dynamic_cast<ObjectIdentifier*>(expr)) {
            // Verify if the object identifier is declared and set its type
        } else if (auto self = dynamic_cast<Self*>(expr)) {
            self->setTypeByName("self");
        } else if (auto newExpr = dynamic_cast<New*>(expr)) {
            // Verify if the class being instantiated exists
        } else if (auto parenthesis = dynamic_cast<Parenthesis*>(expr)) {
            parenthesis->setTypeByName("unit");
        } else if (auto formula = dynamic_cast<Formula*>(expr)) {
            // Verify the formula's type and name
        } else {
            std::cerr << "Error: Unknown expression type." << std::endl;
        }
    }
};