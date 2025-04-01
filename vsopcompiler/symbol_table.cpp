#include <stack>
#include <unordered_map>
#include <string>
#include <vector>

class SymbolTable {
    // stack table:  (name -> hash, type)
    std::stack<std::unordered_map<std::string, std::string>> scopes;

public:
    void enterScope()
    {
        if (!scopes.empty()){
            auto prev_scope = scopes.top(); // Safe access to the top element
            std::cout << "enterScope()" << std::endl;
            scopes.push(prev_scope);
        }
        else {
            std::cout << "Stack is empty; cannot copy the top element." << std::endl;
            scopes.push(std::unordered_map<std::string, std::string>()); // Push an empty scope
        }
    }

    void exitScope() {
        std::cout << "exitScope() "  << std::endl;
        if (!scopes.empty()) {
            scopes.pop();
        }
    }

    bool declare(const std::string name, const std::string type) {
        // if (scopes.top().count(name) != 0) {
        //     return false; // Already declared in the current scope
        // }
        scopes.top()[name] = type;
        return true;
    }

    std::string lookup(const std::string& name) {
        const auto& actual_scp = scopes.top();
        
        auto item = actual_scp.find(name);
        if ( item != actual_scp.end()) {
            return item->second;
        }
        return ""; // not found
    }
};