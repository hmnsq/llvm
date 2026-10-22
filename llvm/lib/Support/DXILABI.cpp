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

#అన్తర్భావ "llvm/Support/DXILABI.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
ఉపయోగించి నామాకాశ llvm;

StringRef dxil::getResourceClassName(dxil::ResourceClass RC) {
  ఎంపిక (RC) {
  స్థితి dxil::ResourceClass::SRV:
    తిరిగి "SRV";
  స్థితి dxil::ResourceClass::UAV:
    తిరిగి "UAV";
  స్థితి dxil::ResourceClass::CBuffer:
    తిరిగి "CBV";
  స్థితి dxil::ResourceClass::Sampler:
    తిరిగి "Sampler";
  }
  llvm_unreachable("Invalid ResourceClass enum value");
}
