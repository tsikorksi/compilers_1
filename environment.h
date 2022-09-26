#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cassert>
#include <map>
#include <string>
#include "value.h"
#include "node.h"

class Environment {
private:
  Environment *m_parent;
  std::map<std::string, Value> variables;

  // copy constructor and assignment operator prohibited
  Environment(const Environment &);
  Environment &operator=(const Environment &);

public:
  Environment(Environment *parent = nullptr);
  ~Environment();

  // TODO: add member functions allowing lookup, definition, and assignment

    Value get_variable(const std::string& identifier, const Location &loc);

    void set_variable(const std::string &identifier, const Value &value, const Location &loc);

    void bind(const std::string &identifier, const Location &loc, const Value &value);

    void new_variable(const std::string &identifier, const Location &loc, ValueKind kind);

    Function *get_function(Node *ast);
    IntrinsicFn * get_intrinsic(Node * ast);
};

#endif // ENVIRONMENT_H
