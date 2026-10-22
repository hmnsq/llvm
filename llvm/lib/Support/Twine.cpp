//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Twine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FormatVariadic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
using ନାମାକାଶ llvm;

std::string Twine::str() ଅଚଳ {
  // If we're storing only a std::string, just return it.
  ଯଦି (LHSKind == StdStringKind && RHSKind == EmptyKind)
    ଫେରନ୍ତୁ *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  ଯଦି (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    ଫେରନ୍ତୁ LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  ଫେରନ୍ତୁ toStringRef(Vec).str();
}

ଶୂନ୍ୟ Twine::toVector(SmallVectorImpl<ଅକ୍ଷର> &Out) ଅଚଳ {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<ଅକ୍ଷର> &Out) ଅଚଳ {
  ଯଦି (isUnary()) {
    ବାଛ (getLHSKind()) {
    ସ୍ଥିତି CStringKind:
      // Already null terminated, yay!
      ଫେରନ୍ତୁ StringRef(LHS.cString);
    ସ୍ଥିତି StdStringKind: {
      ଅଚଳ std::string *str = LHS.stdString;
      ଫେରନ୍ତୁ StringRef(str->c_str(), str->size());
    }
    ସ୍ଥିତି StringLiteralKind:
      ଫେରନ୍ତୁ StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଭାଙ୍ଗ;
    }
  }
  toVector(Out);
  Out.ଠେଲ_back(0);
  Out.କାଢ଼_back();
  ଫେରନ୍ତୁ StringRef(Out.data(), Out.size());
}

ଶୂନ୍ୟ Twine::printOneChild(raw_ostream &OS, Child Ptr, ଗଣ୍ଠି_Kind Kind) ଅଚଳ {
  ବାଛ (Kind) {
  ସ୍ଥିତି Twine::NullKind:
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::EmptyKind:
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::TwineKind:
    Ptr.twine->print(OS);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::CStringKind:
    OS << Ptr.cString;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::StdStringKind:
    OS << *Ptr.stdString;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::PtrAndLengthKind:
  ସ୍ଥିତି Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::CharKind:
    OS << Ptr.character;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecUIKind:
    OS << Ptr.decUI;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecIKind:
    OS << Ptr.decI;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecULKind:
    OS << Ptr.decUL;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecLKind:
    OS << Ptr.decL;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecULLKind:
    OS << Ptr.decULL;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecLLKind:
    OS << Ptr.decLL;
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    ଭାଙ୍ଗ;
  }
}

ଶୂନ୍ୟ Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, ଗଣ୍ଠି_Kind Kind) ଅଚଳ {
  ବାଛ (Kind) {
  ସ୍ଥିତି Twine::NullKind:
    OS << "null";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::EmptyKind:
    OS << "empty";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    ଭାଙ୍ଗ;
  ସ୍ଥିତି Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    ଭାଙ୍ଗ;
  }
}

ଶୂନ୍ୟ Twine::print(raw_ostream &OS) ଅଚଳ {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

ଶୂନ୍ୟ Twine::printRepr(raw_ostream &OS) ଅଚଳ {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ଶୂନ୍ୟ Twine::dump() ଅଚଳ { print(dbgs()); }

LLVM_DUMP_METHOD ଶୂନ୍ୟ Twine::dumpRepr() ଅଚଳ { printRepr(dbgs()); }
#endif
