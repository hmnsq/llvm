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

#समावेश "llvm/Support/DXILABI.h"
#समावेश "llvm/Support/ErrorHandling.h"
using namespace llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  चयन (RC) {
  हाल dxil::ResourceClass::SRV:
    वापस "SRV";
  हाल dxil::ResourceClass::UAV:
    वापस "UAV";
  हाल dxil::ResourceClass::CBuffer:
    वापस "CBV";
  हाल dxil::ResourceClass::Sampler:
    वापस "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
