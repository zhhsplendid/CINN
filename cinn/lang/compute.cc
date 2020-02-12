#include "cinn/lang/compute.h"

#include "cinn/poly/dim.h"
#include "cinn/poly/domain.h"
#include "cinn/poly/element.h"

namespace cinn {
namespace lang {

using ir::Expr;

template <>
ir::Tensor Compute<compute_handle_1_t>(const std::vector<int> &dims, compute_handle_1_t handle) {
  CHECK_EQ(dims.size(), 1);
  Var i(common::axis_name(0), Int(32));
  auto expr = handle(i);

  std::vector<Expr> shape;
  for (int v : dims) shape.emplace_back(v);

  ir::Tensor tensor(shape, {i}, expr.type(), expr);
  return std::move(tensor);
}

template <>
ir::Tensor Compute<compute_handle_2_t>(const std::vector<int> &dims, compute_handle_2_t handle) {
  CHECK_EQ(dims.size(), 2);
  poly::Dim dim("i", 0, dims[0] - 1);
  Var i(common::axis_name(0), Int(32));
  Var j(common::axis_name(1), Int(32));
  auto expr = handle(i, j);

  std::vector<Expr> shape;
  for (int v : dims) shape.emplace_back(v);

  ir::Tensor tensor(shape, {i, j}, expr.type(), expr);
  CHECK(tensor.get());
  return std::move(tensor);
}

template <>
ir::Tensor Compute<compute_handle_3_t>(const std::vector<int> &dims, compute_handle_3_t handle) {
  CHECK_EQ(dims.size(), 3);
  Var i(common::axis_name(0), Int(32));
  Var j(common::axis_name(1), Int(32));
  Var k(common::axis_name(2), Int(32));
  auto expr = handle(i, j, k);

  std::vector<Expr> shape;
  for (int v : dims) shape.emplace_back(v);

  ir::Tensor tensor(shape, {i, j}, expr.type(), expr);
  return std::move(tensor);
}

}  // namespace lang

namespace ir {}  // namespace ir
}  // namespace cinn