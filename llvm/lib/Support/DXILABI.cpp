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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/DXILABI.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  ਚੋਣ (RC) {
  ਹਾਲਤ dxil::ResourceClass::SRV:
    ਵਾਪਸ "SRV";
  ਹਾਲਤ dxil::ResourceClass::UAV:
    ਵਾਪਸ "UAV";
  ਹਾਲਤ dxil::ResourceClass::CBuffer:
    ਵਾਪਸ "CBV";
  ਹਾਲਤ dxil::ResourceClass::Sampler:
    ਵਾਪਸ "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
