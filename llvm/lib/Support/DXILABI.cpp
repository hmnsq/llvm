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

#अन्तर्भाव "llvm/Support/DXILABI.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
उपयुज्य नामाकाश llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  विकल्प (RC) {
  पक्ष dxil::ResourceClass::SRV:
    प्रत्यागम "SRV";
  पक्ष dxil::ResourceClass::UAV:
    प्रत्यागम "UAV";
  पक्ष dxil::ResourceClass::CBuffer:
    प्रत्यागम "CBV";
  पक्ष dxil::ResourceClass::Sampler:
    प्रत्यागम "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
