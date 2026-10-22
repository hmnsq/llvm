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

#অন্তর্ভাৱ "llvm/Support/DXILABI.h"
#অন্তর্ভাৱ "llvm/Support/ErrorHandling.h"
using নামাকাশ llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  ৱিকল্প (RC) {
  পক্ষ dxil::ResourceClass::SRV:
    ঘূৰাই "SRV";
  পক্ষ dxil::ResourceClass::UAV:
    ঘূৰাই "UAV";
  পক্ষ dxil::ResourceClass::CBuffer:
    ঘূৰাই "CBV";
  পক্ষ dxil::ResourceClass::Sampler:
    ঘূৰাই "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
