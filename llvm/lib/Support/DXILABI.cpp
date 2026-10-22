//===-- DXILABI.cpp - ABI Sensitive Values for DXIL -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains definitions of various constants and enums that are
// required to remain stable as per the DXIL format's requirements.
//
// Documentation for DXIL can be found in
// https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/DXIL.rst.
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/DXILABI.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
using নামাকাশ llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  বিকল্প (RC) {
  পক্ষ dxil::ResourceClass::SRV:
    প্রত্যাগম "SRV";
  পক্ষ dxil::ResourceClass::UAV:
    প্রত্যাগম "UAV";
  পক্ষ dxil::ResourceClass::CBuffer:
    প্রত্যাগম "CBV";
  পক্ষ dxil::ResourceClass::Sampler:
    প্রত্যাগম "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
