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

#شامل "llvm/Support/DXILABI.h"
#شامل "llvm/Support/ErrorHandling.h"
using فضاینام llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  انتخاب (RC) {
  حالت dxil::ResourceClass::SRV:
    بازگشت "SRV";
  حالت dxil::ResourceClass::UAV:
    بازگشت "UAV";
  حالت dxil::ResourceClass::CBuffer:
    بازگشت "CBV";
  حالت dxil::ResourceClass::Sampler:
    بازگشت "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
