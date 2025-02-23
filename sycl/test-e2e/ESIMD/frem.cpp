//==---------------- frem.cpp - DPC++ ESIMD on-device test -------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES-INTEL-DRIVER: lin: 30623
// RUN: %{build} -fsycl-device-code-split=per_kernel -o %t.out
// RUN: %{run} %t.out

#include "esimd_test_utils.hpp"

#include <cmath>
#include <sycl/ext/intel/experimental/esimd/math.hpp>

using namespace sycl;

template <typename T> bool test(queue q) {
  std::cout << "Running case: T=" << esimd_test::type_name<T>() << std::endl;
  constexpr unsigned Size = 16;
  constexpr unsigned VL = 16;

  T *A = new T[Size];
  T *B = new T[Size];
  T *C = new T[Size];

  for (unsigned i = 0; i < Size; ++i) {
    A[i] = i + 500;
    B[i] = i + 1;
    C[i] = 0.0f;
  }

  try {
    buffer<T, 1> bufa(A, range<1>(Size));
    buffer<T, 1> bufb(B, range<1>(Size));
    buffer<T, 1> bufc(C, range<1>(Size));

    auto e = q.submit([&](handler &cgh) {
      auto PA = bufa.template get_access<access::mode::read>(cgh);
      auto PB = bufb.template get_access<access::mode::read>(cgh);
      auto PC = bufc.template get_access<access::mode::write>(cgh);
      cgh.single_task([=]() SYCL_ESIMD_KERNEL {
        using namespace sycl::ext::intel::esimd;
        unsigned int offset = 0;
        simd<T, VL> va;
        va.copy_from(PA, offset);
        simd<T, VL> vb;
        vb.copy_from(PB, offset);
        simd<T, VL> vc = ext::intel::experimental::esimd::frem(va, vb);
        vc.copy_to(PC, offset);
      });
    });
    e.wait();
  } catch (sycl::exception const &e) {
    std::cout << "SYCL exception caught: " << e.what() << '\n';

    delete[] A;
    delete[] B;
    delete[] C;
    return 0;
  }

  int err_cnt = 0;

  for (unsigned i = 0; i < Size; ++i) {
    T expected = std::remainder(A[i], B[i]);
    if (C[i] != expected) {
      if (++err_cnt < 10) {
        std::cout << "failed at index " << i << ", " << std::to_string(C[i])
                  << " != " << std::to_string(expected) << "\n";
      }
    }
  }
  if (err_cnt > 0) {
    std::cout << "  pass rate: "
              << ((float)(Size - err_cnt) / (float)Size) * 100.0f << "% ("
              << (Size - err_cnt) << "/" << Size << ")\n";
  }

  delete[] A;
  delete[] B;
  delete[] C;

  std::cout << (err_cnt > 0 ? "FAILED\n" : "Passed\n");
  return err_cnt == 0;
}

int main() {
  auto q = queue{gpu_selector_v};
  esimd_test::printTestLabel(q);
  bool passed = true;

  passed &= test<float>(q);
  // TODO: Enable when driver issue fixed
#if 0
  if (q.get_device().has(sycl::aspect::fp64))
    passed &= test<double>(q);
#endif
  return passed ? 0 : 1;
}
