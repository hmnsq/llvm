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

#અન્તર્ભાવ "llvm/Support/DXILABI.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
using નામાકાશ llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  પસંદગી (RC) {
  સ્થિતિ dxil::ResourceClass::SRV:
    પાછા "SRV";
  સ્થિતિ dxil::ResourceClass::UAV:
    પાછા "UAV";
  સ્થિતિ dxil::ResourceClass::CBuffer:
    પાછા "CBV";
  સ્થિતિ dxil::ResourceClass::Sampler:
    પાછા "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
