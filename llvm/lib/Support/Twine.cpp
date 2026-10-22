//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/ADT/Twine.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/FormatVariadic.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
using നാമാകാശ llvm;

std::string Twine::str() സ്ഥിരം {
  // If we're storing only a std::string, just return it.
  എങ്കിൽ (LHSKind == StdStringKind && RHSKind == EmptyKind)
    തിരികെ *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  എങ്കിൽ (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    തിരികെ LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  തിരികെ toStringRef(Vec).str();
}

ശൂന്യം Twine::toVector(SmallVectorImpl<അക്ഷരം> &Out) സ്ഥിരം {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<അക്ഷരം> &Out) സ്ഥിരം {
  എങ്കിൽ (isUnary()) {
    തിരഞ്ഞെടുപ്പ് (getLHSKind()) {
    അവസ്ഥ CStringKind:
      // Already null terminated, yay!
      തിരികെ StringRef(LHS.cString);
    അവസ്ഥ StdStringKind: {
      സ്ഥിരം std::string *str = LHS.stdString;
      തിരികെ StringRef(str->c_str(), str->size());
    }
    അവസ്ഥ StringLiteralKind:
      തിരികെ StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    സ്ഥിരസ്ഥിതി:
      നിർത്തുക;
    }
  }
  toVector(Out);
  Out.തള്ളുക_back(0);
  Out.എടുക്കുക_back();
  തിരികെ StringRef(Out.data(), Out.size());
}

ശൂന്യം Twine::printOneChild(raw_ostream &OS, Child Ptr, കണ്ണി_Kind Kind) സ്ഥിരം {
  തിരഞ്ഞെടുപ്പ് (Kind) {
  അവസ്ഥ Twine::NullKind:
    നിർത്തുക;
  അവസ്ഥ Twine::EmptyKind:
    നിർത്തുക;
  അവസ്ഥ Twine::TwineKind:
    Ptr.twine->print(OS);
    നിർത്തുക;
  അവസ്ഥ Twine::CStringKind:
    OS << Ptr.cString;
    നിർത്തുക;
  അവസ്ഥ Twine::StdStringKind:
    OS << *Ptr.stdString;
    നിർത്തുക;
  അവസ്ഥ Twine::PtrAndLengthKind:
  അവസ്ഥ Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    നിർത്തുക;
  അവസ്ഥ Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    നിർത്തുക;
  അവസ്ഥ Twine::CharKind:
    OS << Ptr.character;
    നിർത്തുക;
  അവസ്ഥ Twine::DecUIKind:
    OS << Ptr.decUI;
    നിർത്തുക;
  അവസ്ഥ Twine::DecIKind:
    OS << Ptr.decI;
    നിർത്തുക;
  അവസ്ഥ Twine::DecULKind:
    OS << Ptr.decUL;
    നിർത്തുക;
  അവസ്ഥ Twine::DecLKind:
    OS << Ptr.decL;
    നിർത്തുക;
  അവസ്ഥ Twine::DecULLKind:
    OS << Ptr.decULL;
    നിർത്തുക;
  അവസ്ഥ Twine::DecLLKind:
    OS << Ptr.decLL;
    നിർത്തുക;
  അവസ്ഥ Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    നിർത്തുക;
  }
}

ശൂന്യം Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, കണ്ണി_Kind Kind) സ്ഥിരം {
  തിരഞ്ഞെടുപ്പ് (Kind) {
  അവസ്ഥ Twine::NullKind:
    OS << "null";
    നിർത്തുക;
  അവസ്ഥ Twine::EmptyKind:
    OS << "empty";
    നിർത്തുക;
  അവസ്ഥ Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    നിർത്തുക;
  അവസ്ഥ Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    നിർത്തുക;
  അവസ്ഥ Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    നിർത്തുക;
  }
}

ശൂന്യം Twine::print(raw_ostream &OS) സ്ഥിരം {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

ശൂന്യം Twine::printRepr(raw_ostream &OS) സ്ഥിരം {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ശൂന്യം Twine::dump() സ്ഥിരം { print(dbgs()); }

LLVM_DUMP_METHOD ശൂന്യം Twine::dumpRepr() സ്ഥിരം { printRepr(dbgs()); }
#endif
