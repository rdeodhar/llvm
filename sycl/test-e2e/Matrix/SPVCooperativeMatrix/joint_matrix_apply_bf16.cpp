//==----------- joint_matrix_apply_bf16.cpp  - DPC++ joint_matrix-----------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES: aspect-ext_intel_matrix

// RUN: %{build} -D__SPIRV_USE_COOPERATIVE_MATRIX -o %t.out
// RUN: %{run} %t.out

// XFAIL: gpu
// XFAIL: cpu

#include "../common.hpp"
#include "../joint_matrix_apply_bf16_impl.hpp"
