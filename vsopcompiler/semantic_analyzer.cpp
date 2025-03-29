#include "AST.hpp"
// #include "symbol_table.cpp"

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
    void checkClassInhiretence(const std::vector<std::unique_ptr<ClassNode>>& classes) {
        std::unordered_map<std::string, ClassNode*> classMap;
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
            if (method->getName() == "main" && method->getFormals().empty() && method->getReturnType().getName() == "int32") {
                hasMainMethod = true;
                break;
            }
        }

        if (!hasMainMethod) {
            std::cerr << "Error: Class 'Main' must have a 'main' method with no arguments and return type 'int32'." << std::endl;
        }
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
        // check expressions
        // no several formal arguments with the same name 
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
            // std::vector<std::unique_ptr<Type>> arg_types = call->getFormals()->getType();
        }
        else if (auto assign = dynamic_cast<Assign*>(expr)) {
            // verifiy if type
        }
        
    }
};