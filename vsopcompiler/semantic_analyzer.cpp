#include "AST.hpp"
#include <unordered_map>
#include <string>
#include <iostream>

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
    void checkClassInhiretence(std::vector<std::unique_ptr<ClassNode>> classes){
        // check wether all the extended (parent) class exists
        // check if there no is a cyclic definition of classes 
        // class initializer do not contain scope of fields themselves
        // no class is defined more than one time
        // no class is named Object
        // report error:1:1 if no class Main, or Main has no main method
        //and check main args (signature) if are good

    }
    void checkClass(ClassNode* cls) {
        // Check class-level semantics : fields and method can not be declared two times.
        std::cout << "Checking class: " << cls->name << std::endl;

        for (auto& field : cls->getFields()) {
            // ensure 'new' Expr has an exiting class declared. )
            // if extending a parent class -> cannot redfine its fields (with different type..).
            checkField(field.get());
        }

        for (auto& method : cls->getMethods()) {
            // if extending a parent class, must have the same methods args and return type (another type..).
            // no several formal arguments with the same name 
            ///////// check scopes? where? //////////////
            checkMethod(method.get());

        }
    }

    void checkField(FieldNode* field) {
        // Check field-level semantics (e.g., type validity)
        // check if the type is existing, such for class ex: { field : classA}, classA must be declared
        std::cout << "Checking field: " << field->getName() << std::endl;
    }

    void checkMethod(MethodNode* method) {
        // Check method-level semantics (e.g., parameter types, return type)
        std::cout << "Checking method: " << method->toString() << std::endl;
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
        }
        else if (auto call = dynamic_cast<Call*>(expr)) {
            // verifiy if called method is set with appropriate Arguments
            // std::vector<std::unique_ptr<Type>> arg_types = call->getArgs()->getType();
        }
        else if (auto assign = dynamic_cast<Assign*>(expr)) {
            // verifiy if type
        }
        
    }
};