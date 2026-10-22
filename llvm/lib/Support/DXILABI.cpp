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

#ಅನ್ತರ್ಭಾವ "llvm/Support/DXILABI.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  ಆಯ್ಕೆ (RC) {
  ಸ್ಥಿತಿ dxil::ResourceClass::SRV:
    ಹಿಂತಿರುಗಿ "SRV";
  ಸ್ಥಿತಿ dxil::ResourceClass::UAV:
    ಹಿಂತಿರುಗಿ "UAV";
  ಸ್ಥಿತಿ dxil::ResourceClass::CBuffer:
    ಹಿಂತಿರುಗಿ "CBV";
  ಸ್ಥಿತಿ dxil::ResourceClass::Sampler:
    ಹಿಂತಿರುಗಿ "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
