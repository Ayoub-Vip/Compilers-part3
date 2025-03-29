#include <stack>
#include <unordered_map>
#include <string>

class SymbolTable {
    // hash table
    std::stack<std::unordered_map<std::string, std::string>> scopes;


public:
    void enterScope() {
        scopes.push({});
    }

    void exitScope() {
        if (!scopes.empty()) {
            scopes.pop();
        }
    }

    bool declare(const std::string& name, const std::string& type) {
        if (scopes.top().count(name)) {
            return false; // Already declared in the current scope
        }
        scopes.top()[name] = type;
        return true;
    }

    // std::string lookup(const std::string& name) {
    //     for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    //         if (it->count(name)) {
    //             return it->at(name);
    //         }
    //     }
    //     return ""; // Not found
    // }
};