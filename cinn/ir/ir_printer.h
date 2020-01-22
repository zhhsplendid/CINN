#pragma once
#include <string>

#include "cinn/ir/ir.h"
#include "cinn/ir/ir_visitor.h"

namespace cinn {
namespace ir {

struct IrPrinter : public IrVisitor {
  explicit IrPrinter(std::ostream &os) : os_(os) {}

  //! Emit an expression on the output stream.
  void Print(Expr e);
  //! Emit a statement on the output stream.
  void Print(Stmt s);
  //! Emit a expression list with , splitted.
  void Print(const std::vector<Expr> &exprs, const std::string &splitter = ", ");
  //! Emit a binary operator
  template <typename IRN>
  void PrintBinaryOp(const std::string &op, const BinaryOpNode<IRN> *x) {
    os_ << "(";
    Print(x->a);
    os_ << " " + op + " ";
    Print(x->b);
    os_ << ")";
  }

  //! Prefix the current line with `indent_` spaces.
  void DoIndent();
  //! Increase the indent size.
  void IncIndent() { ++indent_; }
  //! Decrease the indent size.
  void DescIndent() { --indent_; }

  void Visit(const IntImm *x) override;
  void Visit(const UIntImm *x) override;
  void Visit(const FloatImm *x) override;
  void Visit(const Add *x) override;
  void Visit(const Sub *x) override;
  void Visit(const Mul *x) override;
  void Visit(const Div *x) override;
  void Visit(const Mod *x) override;
  void Visit(const EQ *x) override;
  void Visit(const NE *x) override;
  void Visit(const LT *x) override;
  void Visit(const LE *x) override;
  void Visit(const GT *x) override;
  void Visit(const GE *x) override;
  void Visit(const And *x) override;
  void Visit(const Or *x) override;
  void Visit(const Not *x) override;
  void Visit(const Min *x) override;
  void Visit(const Max *x) override;
  void Visit(const For *x) override;
  void Visit(const IfThenElse *x) override;
  void Visit(const Block *x) override;
  void Visit(const Call *x) override;
  void Visit(const Cast *x) override;
  void Visit(const Module *x) override;
  void Visit(const Variable *x) override;
  void Visit(const Alloc *x) override;
  void Visit(const Select *x) override;
  void Visit(const Load *x) override;
  void Visit(const Store *x) override;
  void Visit(const Free *x) override;

 private:
  std::ostream &os_;
  uint16_t indent_{};
};

}  // namespace ir
}  // namespace cinn