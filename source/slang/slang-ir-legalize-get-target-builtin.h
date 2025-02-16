// slang-ir-legalize-get-target-builtin.h
#pragma once

#include "slang-ir.h"

namespace Slang
{

/// Performs legalization on `GetTargetBuiltin` instructions.
///
/// This instruction represents globally available built-in values, but the compilation target
/// requires built-in values to be passed in explicitly through entry point function parameters with
/// built-in decorations. `GetTargetbuiltin` instructions are hoistable and transformed to global
/// vars and are assigned the built-in value. The relevant entry point function parameters are
/// updated to contain the built-ins.
void legalizeGetTargetBuiltins(IRModule* module);

} // namespace Slang
