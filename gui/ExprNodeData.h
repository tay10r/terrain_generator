#pragma once

#include "ForwardDecl.h"

#include <memory>

auto
ExprToNodeData(ir::Expr*) -> std::shared_ptr<QtNodes::NodeData>;

auto
NodeDataToExpr(const QtNodes::NodeData*) -> const ir::Expr*;
