//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FormatVariadic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
using ನಾಮಾಕಾಶ llvm;

std::string Twine::str() ಧ್ರುವ {
  // If we're storing only a std::string, just return it.
  ಯದಿ (LHSKind == StdStringKind && RHSKind == EmptyKind)
    ಪ್ರತ್ಯಾಗಮ *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  ಯದಿ (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    ಪ್ರತ್ಯಾಗಮ LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  ಪ್ರತ್ಯಾಗಮ toStringRef(Vec).str();
}

ಶೂನ್ಯ Twine::toVector(SmallVectorImpl<ವರ್ಣ> &Out) ಧ್ರುವ {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<ವರ್ಣ> &Out) ಧ್ರುವ {
  ಯದಿ (isUnary()) {
    ವಿಕಲ್ಪ (getLHSKind()) {
    ಪಕ್ಷ CStringKind:
      // Already null terminated, yay!
      ಪ್ರತ್ಯಾಗಮ StringRef(LHS.cString);
    ಪಕ್ಷ StdStringKind: {
      ಧ್ರುವ std::string *str = LHS.stdString;
      ಪ್ರತ್ಯಾಗಮ StringRef(str->c_str(), str->size());
    }
    ಪಕ್ಷ StringLiteralKind:
      ಪ್ರತ್ಯಾಗಮ StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    ಸ್ವಭಾವ:
      ಭಙ್ಗ;
    }
  }
  toVector(Out);
  Out.ತಳ್ಳು_back(0);
  Out.ತೆಗೆ_back();
  ಪ್ರತ್ಯಾಗಮ StringRef(Out.data(), Out.size());
}

ಶೂನ್ಯ Twine::printOneChild(raw_ostream &OS, Child Ptr, ಗಂಟು_Kind Kind) ಧ್ರುವ {
  ವಿಕಲ್ಪ (Kind) {
  ಪಕ್ಷ Twine::NullKind:
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::EmptyKind:
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::TwineKind:
    Ptr.twine->print(OS);
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::CStringKind:
    OS << Ptr.cString;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::StdStringKind:
    OS << *Ptr.stdString;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::PtrAndLengthKind:
  ಪಕ್ಷ Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::CharKind:
    OS << Ptr.character;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecUIKind:
    OS << Ptr.decUI;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecIKind:
    OS << Ptr.decI;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecULKind:
    OS << Ptr.decUL;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecLKind:
    OS << Ptr.decL;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecULLKind:
    OS << Ptr.decULL;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecLLKind:
    OS << Ptr.decLL;
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    ಭಙ್ಗ;
  }
}

ಶೂನ್ಯ Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, ಗಂಟು_Kind Kind) ಧ್ರುವ {
  ವಿಕಲ್ಪ (Kind) {
  ಪಕ್ಷ Twine::NullKind:
    OS << "null";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::EmptyKind:
    OS << "empty";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    ಭಙ್ಗ;
  ಪಕ್ಷ Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    ಭಙ್ಗ;
  }
}

ಶೂನ್ಯ Twine::print(raw_ostream &OS) ಧ್ರುವ {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

ಶೂನ್ಯ Twine::printRepr(raw_ostream &OS) ಧ್ರುವ {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ಶೂನ್ಯ Twine::dump() ಧ್ರುವ { print(dbgs()); }

LLVM_DUMP_METHOD ಶೂನ್ಯ Twine::dumpRepr() ಧ್ರುವ { printRepr(dbgs()); }
#endif
