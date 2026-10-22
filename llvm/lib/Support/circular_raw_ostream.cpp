//===- circular_raw_ostream.cpp - Implement circular_raw_ostream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This implements support for circular buffered streams.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/circular_raw_ostream.h"
#అన్తర్భావ <algorithm>
ఉపయోగించి నామాకాశ llvm;

శూన్య circular_raw_ostream::write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  ఒకవేళ (బఫరు_కొలత == 0) {
    The_ప్రవాహం->లిఖ(Ptr, కొలత);
    తిరిగి;
  }

  // Write into the buffer, wrapping if necessary.
  అయితే (కొలత != 0) {
    గుర్తులేని బైట్లు_2 =
      std::min(గుర్తులేని(కొలత), గుర్తులేని(బఫరు_కొలత - (Cur - బఫరు_Array)));
    జ్ఞాపకం_నకలు(Cur, Ptr, బైట్లు_2);
    కొలత -= బైట్లు_2;
    Cur += బైట్లు_2;
    ఒకవేళ (Cur == బఫరు_Array + బఫరు_కొలత) {
      // Reset the output pointer to the start of the buffer.
      Cur = బఫరు_Array;
      Filled = నిజం;
    }
  }
}

శూన్య circular_raw_ostream::flush_బఫరు_With_Banner() {
  ఒకవేళ (బఫరు_కొలత != 0) {
    // Write out the buffer
    The_ప్రవాహం->లిఖ(Banner, std::అక్షరశ్రేణి_పొడవు(Banner));
    flush_బఫరు();
  }
}
