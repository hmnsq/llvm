//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the errors for output virtualization.
///
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/VirtualOutputError.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::vfs;

శూన్య OutputError::anchor() {}
శూన్య OutputConfigError::anchor() {}
శూన్య తాత్కాలిక_File_Output_Error::anchor() {}

అక్షరం OutputError::ID = 0;
అక్షరం OutputConfigError::ID = 0;
అక్షరం తాత్కాలిక_File_Output_Error::ID = 0;

శూన్య OutputError::సంవర్గమానం(raw_ostream &OS) స్థిరరాశి {
  OS << get_Output_మార్గం() << ": ";
  ECError::సంవర్గమానం(OS);
}

శూన్య OutputConfigError::సంవర్గమానం(raw_ostream &OS) స్థిరరాశి {
  OutputError::సంవర్గమానం(OS);
  OS << ": " << Config;
}

శూన్య తాత్కాలిక_File_Output_Error::సంవర్గమానం(raw_ostream &OS) స్థిరరాశి {
  OS << get_తాత్కాలిక_మార్గం() << " => ";
  OutputError::సంవర్గమానం(OS);
}

నామాకాశ {
తరగతి OutputErrorCategory : బహిరంగ std::error_category {
బహిరంగ:
  స్థిరరాశి అక్షరం *name() స్థిరరాశి noexcept override;
  std::string సందేశం_2(పూర్ణాంకం EV) స్థిరరాశి override;
};
} // end namespace

స్థిరరాశి std::error_category &vfs::output_category() {
  స్థిర OutputErrorCategory ErrorCategory;
  తిరిగి ErrorCategory;
}

స్థిరరాశి అక్షరం *OutputErrorCategory::name() స్థిరరాశి noexcept {
  తిరిగి "llvm.vfs.output";
}

std::string OutputErrorCategory::సందేశం_2(పూర్ణాంకం EV) స్థిరరాశి {
  Output_Error_సంకేతం E = static_cast<Output_Error_సంకేతం>(EV);
  ఎంపిక (E) {
  స్థితి Output_Error_సంకేతం::invalid_config:
    తిరిగి "invalid config";
  స్థితి Output_Error_సంకేతం::not_closed:
    తిరిగి "output not closed";
  స్థితి Output_Error_సంకేతం::already_closed:
    తిరిగి "output already closed";
  స్థితి Output_Error_సంకేతం::has_open_proxy:
    తిరిగి "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
