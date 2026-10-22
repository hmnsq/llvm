//===- MemoryBufferRef.cpp - Memory Buffer Reference ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the MemoryBufferRef interface.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/MemoryBufferRef.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"

ఉపయోగించి నామాకాశ llvm;

జ్ఞాపకం_బఫరు_Ref::జ్ఞాపకం_బఫరు_Ref(స్థిరరాశి జ్ఞాపకం_బఫరు &బఫరు)
    : బఫరు(బఫరు.get_బఫరు()), Identifier(బఫరు.get_బఫరు_Identifier()) {}
