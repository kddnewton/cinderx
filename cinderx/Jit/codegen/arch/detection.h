// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#if defined(__x86_64__)

#define CINDER_X86_64

#elif defined(__aarch64__)

#define CINDER_AARCH64

#else

#define CINDER_UNKNOWN
#define CINDER_UNSUPPORTED

#endif
