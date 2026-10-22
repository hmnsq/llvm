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
استعمال نامخلا llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  انتخاب (RC) {
  حالت dxil::ResourceClass::SRV:
    واپس "SRV";
  حالت dxil::ResourceClass::UAV:
    واپس "UAV";
  حالت dxil::ResourceClass::CBuffer:
    واپس "CBV";
  حالت dxil::ResourceClass::Sampler:
    واپس "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
