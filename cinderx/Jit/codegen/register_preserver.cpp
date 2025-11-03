// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "cinderx/Jit/codegen/register_preserver.h"

namespace jit::codegen {

#if defined(CINDER_X86_64)
// Stack alignment requirement for x86-64
constexpr size_t kConstStackAlignmentRequirement = 16;
#elif defined(CINDER_AARCH64)
// Stack alignment requirement for aarch64
constexpr int32_t kConstStackAlignmentRequirement = 16;
#else
CINDER_UNSUPPORTED
#endif

RegisterPreserver::RegisterPreserver(
    arch::Builder* as,
    const std::vector<std::pair<const arch::Reg&, const arch::Reg&>>& save_regs)
    : as_(as), save_regs_(save_regs), align_stack_(false) {}

void RegisterPreserver::preserve() {
#if defined(CINDER_X86_64)
  size_t rsp_offset = 0;
  for (const auto& pair : save_regs_) {
    if (pair.first.isGpq()) {
      as_->push((asmjit::x86::Gpq&)pair.first);
    } else if (pair.first.isXmm()) {
      as_->sub(asmjit::x86::rsp, pair.first.size());
      as_->movdqu(
          asmjit::x86::dqword_ptr(asmjit::x86::rsp),
          (asmjit::x86::Xmm&)pair.first);
    } else {
      JIT_ABORT("unsupported saved register type");
    }
    rsp_offset += pair.first.size();
  }
  align_stack_ = rsp_offset % kConstStackAlignmentRequirement;
  if (align_stack_) {
    as_->push(asmjit::x86::rax);
  }
#elif defined(CINDER_AARCH64)
  for (const auto& pair : save_regs_) {
    if (pair.first.isGpX()) {
      as_->str(
          static_cast<const asmjit::a64::Gp&>(pair.first),
          asmjit::a64::ptr_pre(
              asmjit::a64::sp, -kConstStackAlignmentRequirement));
    } else if (pair.first.isVecD()) {
      as_->str(
          static_cast<const asmjit::a64::VecD&>(pair.first),
          asmjit::a64::ptr_pre(
              asmjit::a64::sp, -kConstStackAlignmentRequirement));
    } else {
      JIT_ABORT("unsupported saved register type");
    }
  }
#else
  CINDER_UNSUPPORTED
#endif
}

void RegisterPreserver::remap() {
#if defined(CINDER_X86_64)
  for (const auto& pair : save_regs_) {
    if (pair.first != pair.second) {
      if (pair.first.isGpq()) {
        JIT_DCHECK(pair.second.isGpq(), "can't mix and match register types");
        as_->mov(
            static_cast<const asmjit::x86::Gpq&>(pair.second),
            static_cast<const asmjit::x86::Gpq&>(pair.first));
      } else if (pair.first.isXmm()) {
        JIT_DCHECK(pair.second.isXmm(), "can't mix and match register types");
        as_->movsd(
            static_cast<const asmjit::x86::Xmm&>(pair.second),
            static_cast<const asmjit::x86::Xmm&>(pair.first));
      }
    }
  }
#elif defined(CINDER_AARCH64)
  for (const auto& pair : save_regs_) {
    if (pair.first != pair.second) {
      if (pair.first.isGpX()) {
        JIT_DCHECK(pair.second.isGpX(), "can't mix and match register types");
        as_->mov(
            static_cast<const asmjit::a64::Gp&>(pair.second),
            static_cast<const asmjit::a64::Gp&>(pair.first));
      } else if (pair.first.isVecD()) {
        JIT_DCHECK(pair.second.isVecD(), "can't mix and match register types");
        as_->fmov(
            static_cast<const asmjit::a64::VecD&>(pair.second),
            static_cast<const asmjit::a64::VecD&>(pair.first));
      }
    }
  }
#else
  CINDER_UNSUPPORTED
#endif
}

void RegisterPreserver::restore() {
#if defined(CINDER_X86_64)
  if (align_stack_) {
    as_->add(asmjit::x86::rsp, 8);
  }
  for (auto iter = save_regs_.rbegin(); iter != save_regs_.rend(); ++iter) {
    if (iter->second.isGpq()) {
      as_->pop((asmjit::x86::Gpq&)iter->second);
    } else if (iter->second.isXmm()) {
      as_->movdqu(
          (asmjit::x86::Xmm&)iter->second,
          asmjit::x86::dqword_ptr(asmjit::x86::rsp));
      as_->add(asmjit::x86::rsp, 16);
    } else {
      JIT_ABORT("unsupported saved register type");
    }
  }
#elif defined(CINDER_AARCH64)
  for (auto iter = save_regs_.rbegin(); iter != save_regs_.rend(); ++iter) {
    if (iter->second.isGpX()) {
      as_->ldr(
          static_cast<const asmjit::a64::Gp&>(iter->second),
          asmjit::a64::ptr_post(
              asmjit::a64::sp, kConstStackAlignmentRequirement));
    } else if (iter->second.isVecD()) {
      as_->ldr(
          static_cast<const asmjit::a64::VecD&>(iter->second),
          asmjit::a64::ptr_post(
              asmjit::a64::sp, kConstStackAlignmentRequirement));
    } else {
      JIT_ABORT("unsupported saved register type");
    }
  }
#else
  CINDER_UNSUPPORTED
#endif
}

} // namespace jit::codegen
