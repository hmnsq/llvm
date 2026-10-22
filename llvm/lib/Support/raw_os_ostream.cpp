//===--- raw_os_ostream.cpp - Implement the raw_os_ostream class ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This implements support adapting raw_ostream to std::ostream.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/raw_os_ostream.h"
#అన్తర్భావ <ostream>
ఉపయోగించి నామాకాశ llvm;

//===----------------------------------------------------------------------===//
//  raw_os_ostream
//===----------------------------------------------------------------------===//

raw_os_ostream::~raw_os_ostream() {
  flush();
}

శూన్య raw_os_ostream::write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  OS.లిఖ(Ptr, కొలత);
}

uint64_t raw_os_ostream::current_pos() స్థిరరాశి { తిరిగి OS.tellp(); }
