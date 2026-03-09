#pragma once

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace cymbolic {

class Expr;
using ExprPtr = std::shared_ptr<const Expr>;
using Env     = std::unordered_map<std::string, double>;

// ---------------------------------------------------------------------------
// Base
// ---------------------------------------------------------------------------

class Expr {
public:
    virtual ~Expr() = default;
    virtual double      eval(const Env& env) const = 0;
    virtual std::string to_string()          const = 0;
    virtual ExprPtr     diff(const std::string& var) const = 0;
};

// ---------------------------------------------------------------------------
// Leaf nodes
// ---------------------------------------------------------------------------

class Constant : public Expr {
public:
    explicit Constant(double v) : val_(v) {}

    double eval(const Env&) const override { return val_; }

    std::string to_string() const override {
        // strip trailing zeros for integers
        if (val_ == static_cast<long long>(val_))
            return std::to_string(static_cast<long long>(val_));
        return std::to_string(val_);
    }

    ExprPtr diff(const std::string&) const override;

    double value() const { return val_; }

private:
    double val_;
};

class Variable : public Expr {
public:
    explicit Variable(std::string name) : name_(std::move(name)) {}

    double eval(const Env& env) const override {
        auto it = env.find(name_);
        if (it == env.end())
            throw std::runtime_error("Unbound variable: " + name_);
        return it->second;
    }

    std::string to_string() const override { return name_; }

    ExprPtr diff(const std::string& var) const override;

    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// ---------------------------------------------------------------------------
// Unary node
// ---------------------------------------------------------------------------

class UnaryOp : public Expr {
public:
    explicit UnaryOp(ExprPtr operand) : operand_(std::move(operand)) {}
    const ExprPtr& operand() const { return operand_; }

protected:
    ExprPtr operand_;
};

// ---------------------------------------------------------------------------
// Binary node
// ---------------------------------------------------------------------------

class BinaryOp : public Expr {
public:
    BinaryOp(ExprPtr lhs, ExprPtr rhs) : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    const ExprPtr& lhs() const { return lhs_; }
    const ExprPtr& rhs() const { return rhs_; }

protected:
    ExprPtr lhs_, rhs_;
};

// ---------------------------------------------------------------------------
// Factory helpers (forward declarations so node impls can call them)
// ---------------------------------------------------------------------------

inline ExprPtr constant(double v);
inline ExprPtr var(std::string name);
inline ExprPtr add(ExprPtr l, ExprPtr r);
inline ExprPtr sub(ExprPtr l, ExprPtr r);
inline ExprPtr mul(ExprPtr l, ExprPtr r);
inline ExprPtr div(ExprPtr l, ExprPtr r);
inline ExprPtr pow(ExprPtr base, ExprPtr exp);
inline ExprPtr neg(ExprPtr x);
inline ExprPtr sin(ExprPtr x);
inline ExprPtr cos(ExprPtr x);
inline ExprPtr exp(ExprPtr x);
inline ExprPtr log(ExprPtr x);

// ---------------------------------------------------------------------------
// Binary ops
// ---------------------------------------------------------------------------

class Add : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    double      eval(const Env& env) const override { return lhs_->eval(env) + rhs_->eval(env); }
    std::string to_string()          const override { return "(" + lhs_->to_string() + " + " + rhs_->to_string() + ")"; }
    ExprPtr     diff(const std::string& v) const override { return add(lhs_->diff(v), rhs_->diff(v)); }
};

class Sub : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    double      eval(const Env& env) const override { return lhs_->eval(env) - rhs_->eval(env); }
    std::string to_string()          const override { return "(" + lhs_->to_string() + " - " + rhs_->to_string() + ")"; }
    ExprPtr     diff(const std::string& v) const override { return sub(lhs_->diff(v), rhs_->diff(v)); }
};

class Mul : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    double      eval(const Env& env) const override { return lhs_->eval(env) * rhs_->eval(env); }
    std::string to_string()          const override { return "(" + lhs_->to_string() + " * " + rhs_->to_string() + ")"; }
    // product rule: (f*g)' = f'*g + f*g'
    ExprPtr diff(const std::string& v) const override {
        return add(mul(lhs_->diff(v), rhs_), mul(lhs_, rhs_->diff(v)));
    }
};

class Div : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    double eval(const Env& env) const override {
        double d = rhs_->eval(env);
        if (d == 0.0) throw std::runtime_error("Division by zero");
        return lhs_->eval(env) / d;
    }
    std::string to_string() const override { return "(" + lhs_->to_string() + " / " + rhs_->to_string() + ")"; }
    // quotient rule: (f/g)' = (f'*g - f*g') / g^2
    ExprPtr diff(const std::string& v) const override {
        return div(sub(mul(lhs_->diff(v), rhs_), mul(lhs_, rhs_->diff(v))),
                   mul(rhs_, rhs_));
    }
};

class Pow : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    double      eval(const Env& env) const override { return std::pow(lhs_->eval(env), rhs_->eval(env)); }
    std::string to_string()          const override { return "(" + lhs_->to_string() + " ^ " + rhs_->to_string() + ")"; }
    // general power rule via exp/log: d/dv (f^g) = f^g * (g'*ln(f) + g*f'/f)
    ExprPtr diff(const std::string& v) const override {
        return mul(pow(lhs_, rhs_),
                   add(mul(rhs_->diff(v), log(lhs_)),
                       mul(rhs_, div(lhs_->diff(v), lhs_))));
    }
};

// ---------------------------------------------------------------------------
// Unary ops
// ---------------------------------------------------------------------------

class Neg : public UnaryOp {
public:
    using UnaryOp::UnaryOp;
    double      eval(const Env& env) const override { return -operand_->eval(env); }
    std::string to_string()          const override { return "(-" + operand_->to_string() + ")"; }
    ExprPtr     diff(const std::string& v) const override { return neg(operand_->diff(v)); }
};

class Sin : public UnaryOp {
public:
    using UnaryOp::UnaryOp;
    double      eval(const Env& env) const override { return std::sin(operand_->eval(env)); }
    std::string to_string()          const override { return "sin(" + operand_->to_string() + ")"; }
    // chain rule: cos(f) * f'
    ExprPtr diff(const std::string& v) const override { return mul(cos(operand_), operand_->diff(v)); }
};

class Cos : public UnaryOp {
public:
    using UnaryOp::UnaryOp;
    double      eval(const Env& env) const override { return std::cos(operand_->eval(env)); }
    std::string to_string()          const override { return "cos(" + operand_->to_string() + ")"; }
    // chain rule: -sin(f) * f'
    ExprPtr diff(const std::string& v) const override { return mul(neg(sin(operand_)), operand_->diff(v)); }
};

class Exp : public UnaryOp {
public:
    using UnaryOp::UnaryOp;
    double      eval(const Env& env) const override { return std::exp(operand_->eval(env)); }
    std::string to_string()          const override { return "exp(" + operand_->to_string() + ")"; }
    // chain rule: exp(f) * f'
    ExprPtr diff(const std::string& v) const override {
        return mul(exp(operand_), operand_->diff(v));
    }
};

class Log : public UnaryOp {
public:
    using UnaryOp::UnaryOp;
    double      eval(const Env& env) const override { return std::log(operand_->eval(env)); }
    std::string to_string()          const override { return "log(" + operand_->to_string() + ")"; }
    // chain rule: f' / f
    ExprPtr diff(const std::string& v) const override { return div(operand_->diff(v), operand_); }
};

// ---------------------------------------------------------------------------
// Factory definitions
// ---------------------------------------------------------------------------

inline ExprPtr constant(double v)            { return std::make_shared<Constant>(v); }
inline ExprPtr var(std::string name)         { return std::make_shared<Variable>(std::move(name)); }
inline ExprPtr add(ExprPtr l, ExprPtr r)     { return std::make_shared<Add>(std::move(l), std::move(r)); }
inline ExprPtr sub(ExprPtr l, ExprPtr r)     { return std::make_shared<Sub>(std::move(l), std::move(r)); }
inline ExprPtr mul(ExprPtr l, ExprPtr r)     { return std::make_shared<Mul>(std::move(l), std::move(r)); }
inline ExprPtr div(ExprPtr l, ExprPtr r)     { return std::make_shared<Div>(std::move(l), std::move(r)); }
inline ExprPtr pow(ExprPtr base, ExprPtr e)  { return std::make_shared<Pow>(std::move(base), std::move(e)); }
inline ExprPtr neg(ExprPtr x)                { return std::make_shared<Neg>(std::move(x)); }
inline ExprPtr sin(ExprPtr x)                { return std::make_shared<Sin>(std::move(x)); }
inline ExprPtr cos(ExprPtr x)                { return std::make_shared<Cos>(std::move(x)); }
inline ExprPtr exp(ExprPtr x)                { return std::make_shared<Exp>(std::move(x)); }
inline ExprPtr log(ExprPtr x)                { return std::make_shared<Log>(std::move(x)); }

// ---------------------------------------------------------------------------
// Leaf diff impls (need factories to be defined first)
// ---------------------------------------------------------------------------

inline ExprPtr Constant::diff(const std::string&) const { return constant(0.0); }

inline ExprPtr Variable::diff(const std::string& v) const {
    return constant(name_ == v ? 1.0 : 0.0);
}

// ---------------------------------------------------------------------------
// Operator overloads for ergonomic expression building
// ---------------------------------------------------------------------------

inline ExprPtr operator+(ExprPtr l, ExprPtr r) { return add(std::move(l), std::move(r)); }
inline ExprPtr operator-(ExprPtr l, ExprPtr r) { return sub(std::move(l), std::move(r)); }
inline ExprPtr operator*(ExprPtr l, ExprPtr r) { return mul(std::move(l), std::move(r)); }
inline ExprPtr operator/(ExprPtr l, ExprPtr r) { return div(std::move(l), std::move(r)); }
inline ExprPtr operator-(ExprPtr x)            { return neg(std::move(x)); }

} // namespace cymbolic
