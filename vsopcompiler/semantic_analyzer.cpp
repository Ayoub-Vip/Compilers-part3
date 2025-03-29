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
        // Perform semantic checks on the entire program
        for (auto& cls : program->getClasses()) {
            checkClass(cls.get());
        }
    }
    bool isAccepted = false;

private:
    void checkClass(ClassNode* cls) {
        // Check class-level semantics (e.g., inheritance, field/method declarations)
        std::cout << "Checking class: " << cls->name << std::endl;

        for (auto& field : cls->getFields()) {
            checkField(field.get());
        }

        for (auto& method : cls->getMethods()) {
            checkMethod(method.get());
        }
    }

    void checkField(FieldNode* field) {
        // Check field-level semantics (e.g., type validity)
        std::cout << "Checking field: " << field->getName() << std::endl;
    }

    void checkMethod(MethodNode* method) {
        // Check method-level semantics (e.g., parameter types, return type)
        std::cout << "Checking method: " << method->toString() << std::endl;
    }
};