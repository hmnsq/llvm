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

#അന്തര്ഭാവ "llvm/Support/DXILABI.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
using നാമാകാശ llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  തിരഞ്ഞെടുപ്പ് (RC) {
  അവസ്ഥ dxil::ResourceClass::SRV:
    തിരികെ "SRV";
  അവസ്ഥ dxil::ResourceClass::UAV:
    തിരികെ "UAV";
  അവസ്ഥ dxil::ResourceClass::CBuffer:
    തിരികെ "CBV";
  അവസ്ഥ dxil::ResourceClass::Sampler:
    തിരികെ "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
