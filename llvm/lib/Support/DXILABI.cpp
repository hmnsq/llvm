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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/DXILABI.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
using ନାମାକାଶ llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  ବାଛ (RC) {
  ସ୍ଥିତି dxil::ResourceClass::SRV:
    ଫେରନ୍ତୁ "SRV";
  ସ୍ଥିତି dxil::ResourceClass::UAV:
    ଫେରନ୍ତୁ "UAV";
  ସ୍ଥିତି dxil::ResourceClass::CBuffer:
    ଫେରନ୍ତୁ "CBV";
  ସ୍ଥିତି dxil::ResourceClass::Sampler:
    ଫେରନ୍ତୁ "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
