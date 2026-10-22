//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/ADT/Twine.h"
#অন্তর্ভাৱ "llvm/ADT/SmallString.h"
#অন্তর্ভাৱ "llvm/Config/llvm-config.h"
#অন্তর্ভাৱ "llvm/Support/Debug.h"
#অন্তর্ভাৱ "llvm/Support/FormatVariadic.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
using নামাকাশ llvm;

std::string Twine::str() ধ্রুৱ {
  // If we're storing only a std::string, just return it.
  যদি (LHSKind == StdStringKind && RHSKind == EmptyKind)
    ঘূৰাই *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  যদি (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    ঘূৰাই LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  ঘূৰাই toStringRef(Vec).str();
}

শূন্য Twine::toVector(SmallVectorImpl<ৱর্ণ> &Out) ধ্রুৱ {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<ৱর্ণ> &Out) ধ্রুৱ {
  যদি (isUnary()) {
    ৱিকল্প (getLHSKind()) {
    পক্ষ CStringKind:
      // Already null terminated, yay!
      ঘূৰাই StringRef(LHS.cString);
    পক্ষ StdStringKind: {
      ধ্রুৱ std::string *str = LHS.stdString;
      ঘূৰাই StringRef(str->c_str(), str->size());
    }
    পক্ষ StringLiteralKind:
      ঘূৰাই StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    স্ৱভাৱ:
      ভঙ্গ;
    }
  }
  toVector(Out);
  Out.ঠেলো_back(0);
  Out.বেৰ_কৰো_back();
  ঘূৰাই StringRef(Out.data(), Out.size());
}

শূন্য Twine::printOneChild(raw_ostream &OS, Child Ptr, গ্ৰন্থি_Kind Kind) ধ্রুৱ {
  ৱিকল্প (Kind) {
  পক্ষ Twine::NullKind:
    ভঙ্গ;
  পক্ষ Twine::EmptyKind:
    ভঙ্গ;
  পক্ষ Twine::TwineKind:
    Ptr.twine->print(OS);
    ভঙ্গ;
  পক্ষ Twine::CStringKind:
    OS << Ptr.cString;
    ভঙ্গ;
  পক্ষ Twine::StdStringKind:
    OS << *Ptr.stdString;
    ভঙ্গ;
  পক্ষ Twine::PtrAndLengthKind:
  পক্ষ Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    ভঙ্গ;
  পক্ষ Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    ভঙ্গ;
  পক্ষ Twine::CharKind:
    OS << Ptr.character;
    ভঙ্গ;
  পক্ষ Twine::DecUIKind:
    OS << Ptr.decUI;
    ভঙ্গ;
  পক্ষ Twine::DecIKind:
    OS << Ptr.decI;
    ভঙ্গ;
  পক্ষ Twine::DecULKind:
    OS << Ptr.decUL;
    ভঙ্গ;
  পক্ষ Twine::DecLKind:
    OS << Ptr.decL;
    ভঙ্গ;
  পক্ষ Twine::DecULLKind:
    OS << Ptr.decULL;
    ভঙ্গ;
  পক্ষ Twine::DecLLKind:
    OS << Ptr.decLL;
    ভঙ্গ;
  পক্ষ Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    ভঙ্গ;
  }
}

শূন্য Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, গ্ৰন্থি_Kind Kind) ধ্রুৱ {
  ৱিকল্প (Kind) {
  পক্ষ Twine::NullKind:
    OS << "null";
    ভঙ্গ;
  পক্ষ Twine::EmptyKind:
    OS << "empty";
    ভঙ্গ;
  পক্ষ Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    ভঙ্গ;
  পক্ষ Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    ভঙ্গ;
  পক্ষ Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    ভঙ্গ;
  পক্ষ Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    ভঙ্গ;
  পক্ষ Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    ভঙ্গ;
  পক্ষ Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    ভঙ্গ;
  পক্ষ Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    ভঙ্গ;
  পক্ষ Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    ভঙ্গ;
  পক্ষ Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    ভঙ্গ;
  পক্ষ Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    ভঙ্গ;
  পক্ষ Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    ভঙ্গ;
  পক্ষ Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    ভঙ্গ;
  পক্ষ Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    ভঙ্গ;
  পক্ষ Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    ভঙ্গ;
  }
}

শূন্য Twine::print(raw_ostream &OS) ধ্রুৱ {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

শূন্য Twine::printRepr(raw_ostream &OS) ধ্রুৱ {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য Twine::dump() ধ্রুৱ { print(dbgs()); }

LLVM_DUMP_METHOD শূন্য Twine::dumpRepr() ধ্রুৱ { printRepr(dbgs()); }
#endif
