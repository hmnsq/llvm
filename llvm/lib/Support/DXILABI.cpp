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

#ضمن "llvm/Support/DXILABI.h"
#ضمن "llvm/Support/ErrorHandling.h"
ورتوں نطاق llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  بدل (RC) {
  حالة dxil::ResourceClass::SRV:
    أرجع "SRV";
  حالة dxil::ResourceClass::UAV:
    أرجع "UAV";
  حالة dxil::ResourceClass::CBuffer:
    أرجع "CBV";
  حالة dxil::ResourceClass::Sampler:
    أرجع "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
