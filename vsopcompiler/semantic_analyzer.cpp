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
        
        // std::cout << "Checking class Inhiretence Finished ...... Done " << std::end;

        // Perform semantic checks on the entire program
        for (const auto& cls : program->getClasses()) {
            
            checkClass((ClassNode*) cls.get());
        }


        if (isAccepted)
            exit(1); // do not print syntax if semantic error is detected

    }
    bool isAccepted = true;
    std::string fileName;
    SemanticAnalyzer(std::string fileName) : fileName(std::move(fileName)) {}

private:
    ClassNode* class_in_question = nullptr;
    MethodNode* method_in_question = nullptr;
    SymbolTable symb_tab = SymbolTable();
    std::unordered_map<std::string, ClassNode*> classMap; ///TODO to be curfull here


    void checkClassInhiretence(const std::vector<std::unique_ptr<ClassNode>>& classes) {
        /***
         * check wether all the extended (parent) class exists ........... Done
         *  check if there no is a cyclic definition of classes .......... Done
         *  //TODO class initializer do not contain scope of fields themselves.
         *  no class is defined more than one time ....................... Done
         *  no class is named Object ..................................... Done
         *  report error:1:1 if no class Main, or Main has no main method. Done
         *  and check main args (signature) if are good .................. Done
         ***/

        std::unordered_map<std::string, bool> visited;

        // Populate class map and check for duplicate class definitions
        for (auto& cls : classes) {
            if (classMap.find(cls->name) != classMap.end()) {
                reportSemanticError("Class " + cls->name + " is defined more than once.");
                continue;
            }
            if (cls->name == "Object") {
                reportSemanticError("Class cannot be named 'Object'.");
                continue;
            }
            for (const std::string& s : {"int32", "bool", "string", "unit"}) {//TODO this is additional from my mind
                if (s == cls->name)
                reportSemanticError("Class cannot be named 'int32', 'bool', 'string', or 'unit'.");
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
                    reportSemanticError("Cyclic inheritance detected involving class " + className + ".");
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
            if (!cls->parent.empty() && cls->parent != "Object" && classMap.find(cls->parent) == classMap.end()) {
                reportSemanticError("Parent class " + cls->parent + " of class " + cls->name + " is not declared.");
            }
        }

        // Check for the existence of class Main
        auto mainIt = classMap.find("Main");
        if (mainIt == classMap.end()) {
            reportSemanticError("No class 'Main' defined.");
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
            reportSemanticError("Class 'Main' must have a 'main' method with no arguments and return type 'int32'.");
        }
    }
    
    // Check class-level semantics: fields and method cannot be redeclared twice ... Done
    void checkClass(ClassNode* cls) {

        class_in_question = cls; // 

        symb_tab.enterScope();
        
        // std::cout << "Checking class: " + cls->name << std::end;

        ///// check fields ................................................ Done
        for (auto& field : cls->getFields()) {
            
            // check cannot redfine its parent fields(no different type) .. Done
            if (cls->parent != "Object") {
                const auto& pcls = classMap[cls->parent];
                for (auto& pfield : pcls->getFields()){
                    if (pfield->getName() == field->getName() && pfield->getTypeName() != field->getTypeName())
                        reportSemanticError("the inhireted field '" + pfield->getName() + "' of type '" + pfield->getTypeName() + "' cannot be redefined with different type '" + field->getTypeName() +"' in the child class");                    
                }
            }

            symb_tab.declare(field->getName(), field->getTypeName());

            // check if the type  exists   ................................ Done
            checkField(field.get());
        }

        for (auto& method : cls->getMethods()) {
            //  must have parent class same methods args and return ....... Done
            if (cls->parent != "Object")
                compareMethodsSignature(cls, classMap[cls->parent]);

            checkMethod(method.get());

        }
        symb_tab.enterScope();
    }

    // Check field-level semantics (e.g., type validity) .................. Done
    void checkField(FieldNode* field) {
        // check if the type is existing, ................................. Done
        // ex: { field : classA}, classA must be declared
        std::string ftype = field->getTypeName();

        for (const std::string& s : {"int32", "bool", "string", "unit"}) {
            if (s == ftype)
                return;
        }

        // std::cout << "Checking field: " + field->getName() << std::end;
        if (classMap.count(ftype) == 0)
        reportSemanticError("class type '"+ftype+"' does not exist", field->getColumn(), field->getLine());
            reportSemanticError("class type '" + ftype + "' does not exist");
    }
    
    // if extending a parent class, must have the same methods args and return type (another type..)..... Done
    void compareMethodsSignature(ClassNode* child, ClassNode* parent){
        for (auto& pmethod : parent->getMethods()) {
            for (auto& cmethod : child->getMethods()) {
                if(pmethod->getName() == cmethod->getName()) {
                    if (cmethod->getReturnType().getName() != pmethod->getReturnType().getName()){
                        reportSemanticError("parent class method return type is not the same as the child return type");
                        
                    }
                    if (cmethod->getFormals().size() != pmethod->getFormals().size()) {
                        reportSemanticError("parent class method signature is not the same as the child signature");
                        return;
                    }
                    for (size_t i = 0; i < cmethod->getFormals().size() - 1; ++i) {
                        if (cmethod->getFormals()[i]->getName() == cmethod->getFormals()[i]->getName()) {
                            continue;
                        } else {
                            reportSemanticError("parent class method signature names is not the same as the child");
                        }
                        if (cmethod->getFormals()[i]->getTypeName() == cmethod->getFormals()[i]->getTypeName()) {
                            continue;
                        } else {
                            reportSemanticError("parent class method signature type is not the same as the child");
                        }
                    }
                }
            }
        }
    }

    // Check method-level semantics (e.g., parameter types, return type)
    void checkMethod(MethodNode* method) {
        // std::cout << "Checking method: " + method->getName() << std::end;
        method_in_question = method;
        symb_tab.enterScope();
        
        // no several formal arguments with the same name ................. Done
        std::unordered_map<std::string, bool> visitedFromalName;
        for (auto& formal : method->getFormals()) {
            std::string fname = formal->getName();
            std::string ftype = formal->getType().getName();
            // reportSemanticError("the method " + method->getName() + " several fname'" + fname +"'");

            if(visitedFromalName.count(fname) == 0){
                
                visitedFromalName[fname] = true;
                // Declare forals in the current scope
                symb_tab.declare(fname, ftype);  //TODO what if a formal is already in the previous (class field) scope with  different type??
            }
            else {
                reportSemanticError("the method " + method->getName() + " has several formals with the same name'" + fname +"'");
            }
        }
        
        checkExpression(method->getBlock());

        symb_tab.exitScope();

    }
    // Helper function to get the ancestry chain of a class
    std::vector<std::string> getAncestry(const std::string& className) {
        std::vector<std::string> ancestry;
        std::string currentClass = className;
        while (!currentClass.empty() && currentClass != "Object" && classMap.count(currentClass)) {
            ancestry.push_back(currentClass);
            currentClass = classMap[currentClass]->parent;
        }
        return ancestry;
    }

    // this function getMostCommonAncestor, use classMap to access the parent, the function returns the first common ancestor class before 'Object'
    std::string getMostCommonAncestor(std::string classA, std::string classB) {

        // Get the ancestry chains for both classes
        std::vector<std::string> ancestryA = getAncestry(classA);
        std::vector<std::string> ancestryB = getAncestry(classB);

        // Find the first common ancestor by comparing the chains
        std::string commonAncestor = "Object";
        auto itA = ancestryA.rbegin();
        auto itB = ancestryB.rbegin();

        while (itA != ancestryA.rend() && itB != ancestryB.rend() && *itA == *itB) {
            commonAncestor = *itA;
            ++itA;
            ++itB;
        }

        return commonAncestor;
    }
    void checkExpression(Expr* expr) {
        // check operands are of the same type given operator .............. Done
        if (auto binOp = dynamic_cast<BinaryOperation*>(expr)) {
            checkExpression(binOp->getLeft());
            checkExpression(binOp->getRight());
            std::string op = binOp->getOperator();
            std::string left_type = binOp->getLeft()->getTypeName();
            std::string right_type = binOp->getRight()->getTypeName();

            if (op == "=" or op == "<" or op == "<=") {
                if (left_type != "int32" || right_type != "int32") {
                    reportSemanticError("Binary operation requires int32 operands, but found "+op+"("+ left_type +","+right_type+")");
                }
                binOp->setTypeByName("bool");
            }
            else if (op != "and"){
                if (left_type != "int32" || right_type != "int32") {
                    reportSemanticError("Binary operation requires int32 operands, but found "+op+"("+ left_type +","+right_type+")");
                }
                binOp->setTypeByName("int32");
            }
            else if(op == "and"){
                if (left_type != "bool" || right_type != "bool") {
                    reportSemanticError("Binary operation requires bool operands, but found "+op+"("+ left_type +","+right_type+")");
                }
                binOp->setTypeByName("bool");
            }
            else{
                reportSemanticError("Binary operation, unknown operand '"+ op + "'");
            }
        }
        // same branches type and bool condition ........................... Done 
        else if (auto cond = dynamic_cast<Conditional*>(expr)) {
            checkExpression(cond->getCond_expr());
            checkExpression(cond->getThen_expr());
            
            // check condition is Bool type  .............................. Done
            if (cond->getCond_expr()->getTypeName() != "bool") {
                reportSemanticError("Condition must be of type bool.");
            }

            // Check both branches are of the same types .................. Done
            std::string then_type = cond->getThen_expr()->getTypeName();
            if (cond->getElse_expr()) {
                checkExpression(cond->getElse_expr());
                std::string else_type = cond->getElse_expr()->getTypeName();

                if (then_type == "unit" || else_type == "unit"){
                    cond->setTypeByName("unit");
                }else if (classMap.count(then_type)){
                    std::string first_ancestor = getMostCommonAncestor(then_type, else_type);
                    cond->setTypeByName(first_ancestor);
                }else if (then_type != else_type) {
                    reportSemanticError("semantic error: then and else branches must be of the same return types.");
                }else{
                    cond->setTypeByName(then_type);
                }
            }else{
                cond->setTypeByName(then_type);
            }  
        }
        // call method, verify recursively existence and signature ......... Done
        else if (auto call = dynamic_cast<Call*>(expr)) {

            // verify if the called method exists in the class hierarchy .... Done
            ClassNode* currentClass = nullptr;
            MethodNode* method = nullptr;

            //calling a method inside the same class ==> self, omit checking class existance
            if (call->getClassName() == "self"){ //NOTE class_in_question is updated in 'checkClass'
                for (const auto& mt : class_in_question->getMethods()) {
                    if (mt->getName() == call->getMethodName()) {
                        method = mt.get();
                        break;
                    }
                }
            } else {
                checkExpression(call->getExprObjectIdentifier());

                currentClass = classMap[call->getClassName()]; // getClassName only possible if ExprObjIden is evaluated
                while (currentClass) {
                    for (auto &m : currentClass->getMethods()) {
                        if (m->getName() == call->getMethodName()) {
                            method = m.get();
                            break;
                        }
                    }
                    if (method)
                        break;
                    if (currentClass->parent.empty())
                        break;
                    if(currentClass->parent == "Object")
                        break; //TODO problem is we use Object methods like print()...
                    
                    currentClass = classMap[currentClass->parent];
                }
            }

            if (!method) {
            reportSemanticError("method '" + call->getMethodName() 
                  + "' not found in class hierarchy of '" + call->getClassName() + "'.");
            return;
            }

            // verify the arguments match the method's signature ............. Done
            const auto& formals = method->getFormals();
            const auto& args = call->getArgs();

            if (formals.size() != args.size()) {
                reportSemanticError("method '" + call->getMethodName() 
                    + "' expects " + std::to_string(formals.size()) + " arguments, but " 
                    + std::to_string(args.size()) + " were provided.");
                return;
            }

            for (size_t i = 0; i < args.size(); ++i) {
                checkExpression(args[i].get());
                if (args[i]->getTypeName() != formals[i]->getTypeName()) {
                    reportSemanticError("Argument in position" + std::to_string(i+1)  
                        + " of method '" + call->getMethodName() 
                        + "' expects type '" + formals[i]->getTypeName() 
                        + "', but got type '" + args[i]->getTypeName() + "'.");
                }
            }

            call->setTypeByName(method->getReturnType().getName());
        }
        // verify variable exists and the type of the assigned expression matches its type ....... Done
        else if (auto assign = dynamic_cast<Assign*>(expr)) {
            checkExpression(assign->getExpr());
            // verify if the variable exists ............................... Done
            if (symb_tab.lookup(assign->getName()).empty()) {
                reportSemanticError("You must to declare the variable '"+ assign->getName()
                + "' before assignment.");
            }
            // verify if the type of expression matches the variable type ... Done
            if (symb_tab.lookup(assign->getName()) != assign->getExpr()->getTypeName())
                reportSemanticError("You cannot assign a different type '"
                + assign->getExpr()->getTypeName() 
                + "' to variable '"+ assign->getName()
                + "' of original type '" + symb_tab.lookup(assign->getName()));

        }

        else if (auto intLiteral = dynamic_cast<IntegerLiteral*>(expr)) {
            intLiteral->setTypeByName("int32");
        } else if (auto strLiteral = dynamic_cast<StringLiteral*>(expr)) {
            strLiteral->setTypeByName("string");
        } else if (auto boolLiteral = dynamic_cast<BooleanLiteral*>(expr)) {
            boolLiteral->setTypeByName("bool");
        }

        // check if condition epression returns bool ......................... Done
        else if (auto whileLoop = dynamic_cast<WhileLoop*>(expr)) {
            // check if condition epression returns bool ..................... Done
            checkExpression(whileLoop->getCond_expr());
            if (whileLoop->getCond_expr()->getTypeName() != "bool") {
                reportSemanticError("While loop condition must be of type bool.");
            }
            checkExpression(whileLoop->getBody_expr());
            whileLoop->setTypeByName("unit"); //TODO always unit or the return type of last expr in block?
        }
        
        // just set the return type to thetype of the last expression ........ Done
        else if (auto block = dynamic_cast<Block*>(expr)) {
            
            symb_tab.enterScope();

            for (auto& innerExpr : block->getExprs()) {
                checkExpression(innerExpr.get());
                
            }
            if (!block->getExprs().empty()) {
                block->setTypeByName(block->getExprs().back()->getTypeName());
            } else {
                block->setTypeByName("unit");
            }
            
            symb_tab.exitScope();
        
        }
        
        //TODO see vsop manual for let .. in
        else if (auto let = dynamic_cast<Let*>(expr)) {

            //TODO determine in which on the scope
            symb_tab.declare(let->getName(), let->getType().getName());
            checkExpression(let->getScopeExpr());
            
            if (let->getInitExpr()) {
                checkExpression(let->getInitExpr());
                if (let->getType().getName() != let->getInitExpr()->getTypeName()) {
                reportSemanticError("the type of let '"+ let->getType().getName() + "' must be the same as its Initializer, found :" + let->getInitExpr()->getTypeName());
            }
            }
            // does must body scope return the same type as the type of Let?? 
            // if (let->getType().getName() != let->getScopeExpr()->getTypeName()) {
            //     reportSemanticError("the type of let '"+ let->getType().getName() + "' must be the same as its scope body, found :" + let->getInitExpr()->getTypeName());
            // }

            let->setTypeByName(let->getType().getName());

        }

        // TODO 
        else if (auto unOp = dynamic_cast<UnOp*>(expr)) {
            checkExpression(unOp->getExpr());
            if(unOp->getOp() == "isnull")
                unOp->setTypeByName("bool");
            else if(unOp->getOp() == "-")
                unOp->setTypeByName(unOp->getExpr()->getTypeName());
            else if(unOp->getOp() == "not")
                unOp->setTypeByName(unOp->getExpr()->getTypeName());
            // Verify the type of the operand and set the result type
        }
        
        // Verify if the object identifier is declared and set its type
        else if (auto objIden = dynamic_cast<ObjectIdentifier*>(expr)) {
            std::string objType = symb_tab.lookup(objIden->getName());
            // std::cout << "objType ----------> "+objType+std::end << std::end;
            // std::cout << "obobjIden->getName()jType ----------> "+objIden->getName()+std::end << std::end;
            
            if(objType == "")
                reportSemanticError("the object '" + objIden->toString()+"' is not defined in the scope.");
            else
                objIden->setTypeByName(objType);
            
            // std::cout << "objType agiµain ----------> "+objIden->getTypeName()+std::end << std::end;
            
        }
        //TODO
        else if (auto self = dynamic_cast<Self*>(expr)) {
            self->setTypeByName(method_in_question->getName());
        }
        // Verify if the class being instantiated exists ..................... Done
        else if (auto newExpr = dynamic_cast<New*>(expr)) {
            // Verify if the class being instantiated exists
            // std::cout << "newExpr->getClassName() : " + newExpr->toString() << std::end;
            if (classMap.count(newExpr->getClassName()))
                newExpr->setTypeByName(newExpr->getClassName());
            else
                reportSemanticError("the class '" + newExpr->getTypeName() + "' does not exists to be instanciated.");
        }
        //TODO this bellow is not clear
        else if (auto parenthesis = dynamic_cast<Parenthesis*>(expr)) {
            parenthesis->setTypeByName("unit");
        }
        //TODO
        else {
            reportSemanticError("Unknown expression type.");
        }
    }

    void reportSemanticError(std::string message,  unsigned int column=0, unsigned int line=0) {
        std::cerr << fileName << ":" << line << ":" << column 
                  << ": semantic error: "<< message << std::endl;
        // exit(1); // Exit the program with an error code
    }
};