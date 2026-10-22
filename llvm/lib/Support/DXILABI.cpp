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

#அந்தர்பாவ "llvm/Support/DXILABI.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
using நாமாகாஶ llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  தேர்வு (RC) {
  நிலை dxil::ResourceClass::SRV:
    திரும்பு "SRV";
  நிலை dxil::ResourceClass::UAV:
    திரும்பு "UAV";
  நிலை dxil::ResourceClass::CBuffer:
    திரும்பு "CBV";
  நிலை dxil::ResourceClass::Sampler:
    திரும்பு "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
