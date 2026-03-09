#pragma once

// Expression tree – skeleton header.
// Fill this in as you implement the library.

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cymbolic {

// Forward declaration
class Expr;
using ExprPtr = std::shared_ptr<Expr>;
using Env     = std::unordered_map<std::string, double>;

// Base class for all expression nodes
class Expr {
public:
    virtual ~Expr() = default;

    // Evaluate the expression given variable bindings
    virtual double eval(const Env& env) const = 0;

    // Return a human-readable string
    virtual std::string to_string() const = 0;

    // Differentiate with respect to a variable
    virtual ExprPtr diff(const std::string& var) const = 0;
};

} // namespace cymbolic
