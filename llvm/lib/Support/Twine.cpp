//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/FormatVariadic.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
using नामाकाश llvm;

std::string Twine::str() ध्रुव {
  // If we're storing only a std::string, just return it.
  जइ (LHSKind == StdStringKind && RHSKind == EmptyKind)
    प्रत्यागम *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  जइ (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    प्रत्यागम LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  प्रत्यागम toStringRef(Vec).str();
}

शून्य Twine::toVector(SmallVectorImpl<वर्ण> &Out) ध्रुव {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<वर्ण> &Out) ध्रुव {
  जइ (isUnary()) {
    विकल्प (getLHSKind()) {
    पक्ष CStringKind:
      // Already null terminated, yay!
      प्रत्यागम StringRef(LHS.cString);
    पक्ष StdStringKind: {
      ध्रुव std::string *str = LHS.stdString;
      प्रत्यागम StringRef(str->c_str(), str->size());
    }
    पक्ष StringLiteralKind:
      प्रत्यागम StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    स्वभाव:
      भङ्ग;
    }
  }
  toVector(Out);
  Out.प्रक्षिप_back(0);
  Out.निष्कासय_back();
  प्रत्यागम StringRef(Out.data(), Out.size());
}

शून्य Twine::printOneChild(raw_ostream &OS, Child Ptr, ग्रन्थिः_Kind Kind) ध्रुव {
  विकल्प (Kind) {
  पक्ष Twine::NullKind:
    भङ्ग;
  पक्ष Twine::EmptyKind:
    भङ्ग;
  पक्ष Twine::TwineKind:
    Ptr.twine->print(OS);
    भङ्ग;
  पक्ष Twine::CStringKind:
    OS << Ptr.cString;
    भङ्ग;
  पक्ष Twine::StdStringKind:
    OS << *Ptr.stdString;
    भङ्ग;
  पक्ष Twine::PtrAndLengthKind:
  पक्ष Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    भङ्ग;
  पक्ष Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    भङ्ग;
  पक्ष Twine::CharKind:
    OS << Ptr.character;
    भङ्ग;
  पक्ष Twine::DecUIKind:
    OS << Ptr.decUI;
    भङ्ग;
  पक्ष Twine::DecIKind:
    OS << Ptr.decI;
    भङ्ग;
  पक्ष Twine::DecULKind:
    OS << Ptr.decUL;
    भङ्ग;
  पक्ष Twine::DecLKind:
    OS << Ptr.decL;
    भङ्ग;
  पक्ष Twine::DecULLKind:
    OS << Ptr.decULL;
    भङ्ग;
  पक्ष Twine::DecLLKind:
    OS << Ptr.decLL;
    भङ्ग;
  पक्ष Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    भङ्ग;
  }
}

शून्य Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, ग्रन्थिः_Kind Kind) ध्रुव {
  विकल्प (Kind) {
  पक्ष Twine::NullKind:
    OS << "null";
    भङ्ग;
  पक्ष Twine::EmptyKind:
    OS << "empty";
    भङ्ग;
  पक्ष Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    भङ्ग;
  पक्ष Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    भङ्ग;
  पक्ष Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    भङ्ग;
  पक्ष Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    भङ्ग;
  पक्ष Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    भङ्ग;
  पक्ष Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    भङ्ग;
  पक्ष Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    भङ्ग;
  पक्ष Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    भङ्ग;
  पक्ष Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    भङ्ग;
  पक्ष Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    भङ्ग;
  पक्ष Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    भङ्ग;
  पक्ष Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    भङ्ग;
  पक्ष Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    भङ्ग;
  पक्ष Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    भङ्ग;
  }
}

शून्य Twine::print(raw_ostream &OS) ध्रुव {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

शून्य Twine::printRepr(raw_ostream &OS) ध्रुव {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#जइ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य Twine::dump() ध्रुव { print(dbgs()); }

LLVM_DUMP_METHOD शून्य Twine::dumpRepr() ध्रुव { printRepr(dbgs()); }
#endif
