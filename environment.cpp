#include "environment.h"
#include "exceptions.h"

Environment::Environment(Environment *parent)
  : m_parent(parent) {
  assert(m_parent != this);
}

Environment::~Environment() = default;


// Add new variable to environment
void Environment::new_variable(const std::string& identifier, const Location &loc) {
    if (Environment::variables.find(identifier) != Environment::variables.end()) {
        SemanticError::raise(loc, "Variable %s already exists", identifier.c_str());
    }
    Environment::variables.insert({identifier, 0});
}

// set value of variable in environment
void Environment::set_variable(const std::string& identifier, const Value& value) {
    Environment::variables.erase(Environment::variables.find(identifier));
    Environment::variables.insert({identifier, value});
}


// Get value of variable from environment
Value Environment::get_variable(const std::string& identifier, const Location &loc) {
    if (Environment::variables.find(identifier) == Environment::variables.end()) {
        SemanticError::raise(loc, "Tried to access variable %s, not found", identifier.c_str());
    }
    return Environment::variables[identifier];
}

void Environment::bind(const std::string& identifier, const Location &loc, const Value& value){
    new_variable(identifier, loc);
    set_variable(identifier, value);
}
