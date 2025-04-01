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
        
        std::cout << "Checking class Inhiretence Finished ...... Done " << std::endl;

        // Perform semantic checks on the entire program
        for (const auto& cls : program->getClasses()) {
            
            checkClass((ClassNode*) cls.get());
        }

    }
    bool isAccepted = false;
    ClassNode* class_in_question = nullptr;
    MethodNode* method_in_question = nullptr;
private:
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
            if (!cls->parent.empty() && cls->parent != "Object" && classMap.find(cls->parent) == classMap.end()) {
                std::cerr << "Error: Parent class " << cls->parent << " of class " << cls->name << " is not declared." << std::endl;
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
    
    // Check class-level semantics: fields and method cannot be redeclared twice ... Done
    void checkClass(ClassNode* cls) {

        class_in_question = cls; // 

        symb_tab.enterScope();
        
        std::cout << "Checking class: " << cls->name << std::endl;

        ///// check fields ................................................ Done
        for (auto& field : cls->getFields()) {
            
            // check cannot redfine its parent fields(no different type) .. Done
            if (cls->parent != "Object") {
                const auto& pcls = classMap[cls->parent];
                for (auto& pfield : pcls->getFields()){
                    if (pfield->getName() == field->getName() && pfield->getTypeName() != field->getTypeName())
                        std::cerr << "semantic error : the inhireted field '" << pfield->getName() << "' of type '" << pfield->getTypeName() << "' cannot be redefined with different type '" << field->getTypeName() <<"' in the child class" << std::endl;                    
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

        std::cout << "Checking field: " << field->getName() << std::endl;
        if (classMap.count(ftype) == 0)
            std::cerr << "semantic error : class type" << ftype << "does not exist" << std::endl;
    }
    
    // if extending a parent class, must have the same methods args and return type (another type..)..... Done
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
        std::cout << "Checking method: " << method->getName() << std::endl;
        method_in_question = method;
        symb_tab.enterScope();
        
        // no several formal arguments with the same name ................. Done
        std::unordered_map<std::string, bool> visitedFromalName;
        for (auto& formal : method->getFormals()) {
            std::string fname = formal->getName();
            std::string ftype = formal->getType().getName();
            // std::cerr << "semantic error : the method " << method->getName() << " several fname'" << fname <<"'" << std::endl;

            if(visitedFromalName.count(fname) == 0){
                
                visitedFromalName[fname] = true;
                // Declare forals in the current scope
                symb_tab.declare(fname, ftype);  //TODO what if a formal is already in the previous (class field) scope with  different type??
            }
            else {
                std::cerr << "semantic error : the method " << method->getName() << " has several formals with the same name'" << fname <<"'" << std::endl;
            }
        }
        
        //TODO check expressions Scope and types
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
                    std::cerr << "Type error: Binary operation requires int32 operands." 
                    << "but found "<<op<<"("<< left_type <<","<<left_type << std::endl;
                }
                binOp->setTypeByName("bool");
            }
            else if (op != "and"){
                if (left_type != "int32" || right_type != "int32") {
                    std::cerr << "Type error: Binary operation requires int32 operands." 
                    << "but found "<<op<<"("<< left_type <<","<<left_type << std::endl;
                }
                binOp->setTypeByName("int32");
            }
            else if(op == "and"){
                if (left_type != "bool" || right_type != "bool") {
                    std::cerr << "Type error: Binary operation requires bool operands." 
                    << "but found "<<op<<"("<< left_type <<","<<left_type << std::endl;
                }
                binOp->setTypeByName("bool");
            }
            else{
                std::cerr << "Type error: Binary operation, unknown operand '"<< op << "'"<< std::endl;
            }
        }
        // same branches type and bool condition ........................... Done 
        else if (auto cond = dynamic_cast<Conditional*>(expr)) {
            checkExpression(cond->getCond_expr());
            checkExpression(cond->getThen_expr());
            
            // check condition is Bool type  .............................. Done
            if (cond->getCond_expr()->getTypeName() != "bool") {
                std::cerr << "Type error: Condition must be of type bool." << std::endl;
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
                    std::cerr << "semantic error: then and else branches must be of the same return types." << std::endl;
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
            std::cerr << "semantic error : method '" << call->getMethodName() 
                  << "' not found in class hierarchy of '" << call->getClassName() << "'." << std::endl;
            return;
            }

            // verify the arguments match the method's signature ............. Done
            const auto& formals = method->getFormals();
            const auto& args = call->getArgs();

            if (formals.size() != args.size()) {
                std::cerr << "semantic error : method '" << call->getMethodName() 
                    << "' expects " << formals.size() << " arguments, but " 
                    << args.size() << " were provided." << std::endl;
                return;
            }

            for (size_t i = 0; i < args.size(); ++i) {
                checkExpression(args[i].get());
                if (args[i]->getTypeName() != formals[i]->getTypeName()) {
                    std::cerr << "semantic error: Argument " << i + 1 
                        << " of method '" << call->getMethodName() 
                        << "' expects type '" << formals[i]->getTypeName() 
                        << "', but got type '" << args[i]->getTypeName() << "'." << std::endl;
                }
            }

            call->setTypeByName(method->getReturnType().getName());
        }
        // verify variable exists and the type of the assigned expression matches its type ....... Done
        else if (auto assign = dynamic_cast<Assign*>(expr)) {
            checkExpression(assign->getExpr());
            // verify if the variable exists ............................... Done
            if (symb_tab.lookup(assign->getName()).empty()) {
                std::cerr << "semantic error: You must to declare the variable '"<< assign->getName()
                << "' before assignment." << std::endl;
            }
            // verify if the type of expression matches the variable type ... Done
            if (symb_tab.lookup(assign->getName()) != assign->getExpr()->getTypeName())
                std::cerr << "semantic error: You cannot assign a different type '"
                << assign->getExpr()->getTypeName() 
                << "' to variable '"<< assign->getName()
                << "' of original type '" << symb_tab.lookup(assign->getName()) << std::endl;

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
                std::cerr << "Type error: While loop condition must be of type bool." << std::endl;
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
            checkExpression(let->getScopeExpr());
            
            if (let->getInitExpr()) {
                checkExpression(let->getInitExpr());
                if (let->getType().getName() != let->getInitExpr()->getTypeName()) {
                std::cerr << "semantic error: the type of let '"<< let->getType().getName() << "' must be the same as its Initializer, found :" << let->getInitExpr()->getTypeName() << std::endl;
            }
            }
            // does must body scope return the same type as the type of Let?? 
            // if (let->getType().getName() != let->getScopeExpr()->getTypeName()) {
            //     std::cerr << "semantic error: the type of let '"<< let->getType().getName() << "' must be the same as its scope body, found :" << let->getInitExpr()->getTypeName() << std::endl;
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
            // std::cout << "objType ----------> "<<objType<<std::endl;
            // std::cout << "obobjIden->getName()jType ----------> "<<objIden->getName()<<std::endl;
            
            if(objType == "")
                std::cout << "semantic error : the object '" << objIden->toString()<<"' is not defined in the scope." <<std::endl;
            else
                objIden->setTypeByName(objType);
            
            // std::cout << "objType agiµain ----------> "<<objIden->getTypeName()<<std::endl;
            
        }
        //TODO
        else if (auto self = dynamic_cast<Self*>(expr)) {
            self->setTypeByName(method_in_question->getName());
        }
        // Verify if the class being instantiated exists ..................... Done
        else if (auto newExpr = dynamic_cast<New*>(expr)) {
            // Verify if the class being instantiated exists
            std::cout << "newExpr->getClassName() : " << newExpr->toString() <<std::endl;
            if (classMap.count(newExpr->getClassName()))
                newExpr->setTypeByName(newExpr->getClassName());
            else
                std::cerr << "semantic error : the class '" << newExpr->getTypeName() << "' does not exists to be instanciated." << std::endl;
        }
        //TODO this bellow is not clear
        else if (auto parenthesis = dynamic_cast<Parenthesis*>(expr)) {
            parenthesis->setTypeByName("unit");
        }
        //TODO
        else {
            std::cerr << "Error: Unknown expression type." << std::endl;
        }
    }
};