#include <stack>
#include <unordered_map>
#include <string>
#include <vector>

class SymbolTable {
    // stack table:  (name -> hash, type)
    std::stack<std::unordered_map<std::string, std::string>> scopes;


public:
    void enterScope() {
        auto prev_scope = scopes.top(); // copy symbole table
        scopes.push(prev_scope);
    }

    void exitScope() {
        if (!scopes.empty()) {
            scopes.pop();
        }
    }

    bool declare(const std::string& name, const std::string& type) {
        // if (scopes.top().count(name) != 0) {
        //     return false; // Already declared in the current scope
        // }
        scopes.top()[name] = type;
        return true;
    }

    std::string lookup(const std::string& name) {
        auto actual_scp = scopes.top();
        while (!actual_scp.empty()) {
            
            // if (it->count(name)) {
            //     return it->at(name);
            // }

        }
        return ""; // Not found
    }
};