#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cassert>
#include <map>
#include <string>
#include "value.h"

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

    void new_variable(const std::string &identifier, const Location &loc);

    void set_variable(const std::string &identifier, const Value& value);

    void bind(const std::string &identifier, const Location &loc, const Value &value);
};

#endif // ENVIRONMENT_H
