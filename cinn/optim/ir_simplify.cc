#include "cinn/optim/ir_simplify.h"

#include <ginac/ginac.h>
#include <glog/logging.h>

#include <map>
#include <string>

#include "cinn/common/arithmatic.h"
#include "cinn/common/cas.h"
#include "cinn/ir/ir_mutator.h"
#include "cinn/ir/ir_operators.h"
#include "cinn/ir/ir_printer.h"
#include "cinn/ir/ir_visitor.h"
#include "cinn/utils/string.h"

namespace cinn {
namespace optim {
using namespace ir;  // NOLINT
using common::ExprToGinacConerter;
using utils::GetStreamCnt;
using utils::Replace;

namespace {

//! Simplify some sub-expression in the `expr`. Due to the simplify strategy just fit several kinds of IR noedes, we
//! partition the original expression to several sub-expression those supported by simplify, and process each of them.
void PartialSimplify(Expr* expr, const std::unordered_map<std::string, common::CasInterval>& var_intervals = {}) {
  *expr = common::AutoSimplify(*expr, var_intervals);
}

//! Simplify the expression but Load.
struct SimplifyButStoreLoadMutator : public ir::IRMutator<ir::Expr*> {
  const common::cas_intervals_t& var_intervals;
  explicit SimplifyButStoreLoadMutator(const common::cas_intervals_t& var_intervals) : var_intervals(var_intervals) {}

  void operator()(Expr* x) { ir::IRMutator<ir::Expr*>::Visit(x, x); }

  using ir::IRMutator<>::Visit;

#define __(op__) \
  void Visit(const op__* op, Expr* expr) override { PartialSimplify(expr, var_intervals); }

  __(Add)
  __(Mul)
  __(Sub)
  __(Div)
#undef __

  void Visit(const Ramp* op, Expr* expr) override {
    auto* node = expr->As<Ramp>();
    CHECK(common::IsPureMath(node->base));
    CHECK(common::IsPureMath(node->stride));
    PartialSimplify(&node->base, var_intervals);
    PartialSimplify(&node->stride, var_intervals);
  }
};

struct SimplifyLoadMutator : public ir::IRMutator<ir::Expr*> {
  void operator()(Expr* x) { ir::IRMutator<ir::Expr*>::Visit(x, x); }

  void Visit(const Load* expr, Expr* op) override {
    auto* node = op->As<Load>();
    if (common::IsPureMath(node->index)) {
      PartialSimplify(&node->index);
    } else {
      SimplifyButStoreLoadMutator mutator(var_intervals_);
      mutator(&node->index);
    }
  }

  void Visit(const For* op, Expr* expr) override {
    auto* min_i    = op->min.As<IntImm>();
    auto* extent_i = op->extent.As<IntImm>();
    if (min_i && extent_i) {
      var_intervals_.emplace(op->loop_var->name, common::CasInterval{min_i->value, extent_i->value - 1});
      LOG(INFO) << "found interval " << op->loop_var->name;
    }

    auto* node = expr->As<For>();

    operator()(&node->body);
    operator()(&node->extent);

    if (min_i && extent_i) {
      var_intervals_.erase(op->loop_var->name);
    }
  }

  common::cas_intervals_t var_intervals_;
};

struct SimplifyStoreMutator : public ir::IRMutator<ir::Expr*> {
  void operator()(Expr* x) { ir::IRMutator<ir::Expr*>::Visit(x, x); }

  void Visit(const Store* expr, Expr* op) override {
    auto* node = op->As<Store>();

    if (common::IsPureMath(node->index)) {
      PartialSimplify(&node->index);
    } else {
      SimplifyButStoreLoadMutator mutator(var_intervals_);
      mutator(&node->index);
    }
  }

  void Visit(const For* op, Expr* expr) override {
    auto* min_i    = op->min.As<IntImm>();
    auto* extent_i = op->extent.As<IntImm>();
    if (min_i && extent_i) {
      var_intervals_.emplace(op->loop_var->name, common::CasInterval{min_i->value, extent_i->value - 1});
      LOG(INFO) << "found interval " << op->loop_var->name;
    }

    auto* node = expr->As<For>();

    operator()(&node->body);
    operator()(&node->extent);

    if (min_i && extent_i) {
      var_intervals_.erase(op->loop_var->name);
    }
  }

  common::cas_intervals_t var_intervals_;
};

struct SimplifyRampMutator : public ir::IRMutator<Expr*> {
  void operator()(Expr* x) { ir::IRMutator<ir::Expr*>::Visit(x, x); }

  void Visit(const Ramp* op, Expr* expr) override {
    auto* node = expr->As<ir::Ramp>();

    CHECK(common::IsPureMath(node->base));
    CHECK(common::IsPureMath(node->stride));
    Simplify(&node->base);
    Simplify(&node->stride);
  }
};

}  // namespace

void Simplify(Expr* expr) {
  SimplifyRampMutator()(expr);
  SimplifyLoadMutator()(expr);
  SimplifyStoreMutator()(expr);

  common::cas_intervals_t var_intervals;
  SimplifyButStoreLoadMutator mutator(var_intervals);
  mutator(expr);
}

}  // namespace optim
}  // namespace cinn
