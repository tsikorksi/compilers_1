#include "environment.h"
#include "exceptions.h"

Environment::Environment(Environment *parent)
  : m_parent(parent) {
  assert(m_parent != this);
}

Environment::~Environment() = default;


// Add new variable to environment
void Environment::new_variable(const std::string& identifier, int value, const Location &loc) {
    if (Environment::variables.find(identifier) != Environment::variables.end()) {
        SemanticError::raise(loc, "Variable %s already exists", identifier.c_str());
    }
    Environment::variables.insert({identifier, value});
}


// Get value of variable from environment
int Environment::get_variable(const std::string& identifier, const Location &loc) {
    if (Environment::variables.find(identifier) == Environment::variables.end()) {
        EvaluationError::raise(loc, "Variable %s not found", identifier.c_str());
    }
    return Environment::variables[identifier];
}
