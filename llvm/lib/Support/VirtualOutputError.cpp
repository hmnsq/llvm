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

#अन्तर्भाव "llvm/Support/VirtualOutputError.h"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::vfs;

शून्य OutputError::anchor() {}
शून्य OutputConfigError::anchor() {}
शून्य अस्थायी_File_Output_Error::anchor() {}

वर्ण OutputError::ID = 0;
वर्ण OutputConfigError::ID = 0;
वर्ण अस्थायी_File_Output_Error::ID = 0;

शून्य OutputError::लघुगणकः(raw_ostream &OS) ध्रुव {
  OS << get_Output_मार्गः() << ": ";
  ECError::लघुगणकः(OS);
}

शून्य OutputConfigError::लघुगणकः(raw_ostream &OS) ध्रुव {
  OutputError::लघुगणकः(OS);
  OS << ": " << Config;
}

शून्य अस्थायी_File_Output_Error::लघुगणकः(raw_ostream &OS) ध्रुव {
  OS << get_अस्थायी_मार्गः() << " => ";
  OutputError::लघुगणकः(OS);
}

नामाकाश {
वर्ग OutputErrorCategory : सार्वजनिक std::error_category {
सार्वजनिक:
  ध्रुव वर्ण *name() ध्रुव noexcept override;
  std::string सन्देशः_2(पूर्णाङ्क EV) ध्रुव override;
};
} // end namespace

ध्रुव std::error_category &vfs::output_category() {
  स्थिर OutputErrorCategory ErrorCategory;
  प्रत्यागम ErrorCategory;
}

ध्रुव वर्ण *OutputErrorCategory::name() ध्रुव noexcept {
  प्रत्यागम "llvm.vfs.output";
}

std::string OutputErrorCategory::सन्देशः_2(पूर्णाङ्क EV) ध्रुव {
  Output_Error_संहिता E = static_cast<Output_Error_संहिता>(EV);
  विकल्प (E) {
  पक्ष Output_Error_संहिता::invalid_config:
    प्रत्यागम "invalid config";
  पक्ष Output_Error_संहिता::not_closed:
    प्रत्यागम "output not closed";
  पक्ष Output_Error_संहिता::already_closed:
    प्रत्यागम "output already closed";
  पक्ष Output_Error_संहिता::has_open_proxy:
    प्रत्यागम "output has open proxy";
  }
  llvm_unreachable(
      "An enumerator of OutputErrorCode does not have a message defined.");
}
