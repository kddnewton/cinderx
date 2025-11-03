// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "cinderx/Jit/codegen/arch.h"

#if defined(CINDER_AARCH64)

using namespace asmjit;

namespace jit::codegen::arch {

a64::Mem ptr_offset(const a64::Gp& base, int32_t offset) {
  JIT_CHECK(
      (offset >= 0 ? (offset < 2048) : (offset >= -256 && offset < 256)),
      "offset out of range");
  return asmjit::a64::ptr(base, offset);
}

a64::Mem ptr_resolve(
    a64::Builder* as,
    const a64::Gp& base,
    int32_t offset,
    const a64::Gp& scratch) {
  if (offset >= -256 && offset < 256) {
    return asmjit::a64::ptr(base, offset);
  }

  if (offset >= 0 && arm::Utils::isAddSubImm(static_cast<uint64_t>(offset))) {
    as->add(scratch, base, offset);
  } else if (
      offset < 0 && arm::Utils::isAddSubImm(static_cast<uint64_t>(-offset))) {
    as->sub(scratch, base, -offset);
  } else if (offset >= 0) {
    as->mov(scratch, offset);
    as->add(scratch, base, scratch);
  } else {
    as->mov(scratch, -offset);
    as->sub(scratch, base, scratch);
  }

  return a64::ptr(scratch);
}

} // namespace jit::codegen::arch

#endif

namespace jit::codegen {

std::ostream& operator<<(std::ostream& out, const PhyLocation& loc) {
  return out << loc.toString();
}

} // namespace jit::codegen
