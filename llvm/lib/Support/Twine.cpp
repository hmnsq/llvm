//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/Twine.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/FormatVariadic.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
using નામાકાશ llvm;

std::string Twine::str() અચલ {
  // If we're storing only a std::string, just return it.
  જો (LHSKind == StdStringKind && RHSKind == EmptyKind)
    પાછા *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  જો (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    પાછા LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  પાછા toStringRef(Vec).str();
}

શૂન્ય Twine::toVector(SmallVectorImpl<અક્ષર> &Out) અચલ {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<અક્ષર> &Out) અચલ {
  જો (isUnary()) {
    પસંદગી (getLHSKind()) {
    સ્થિતિ CStringKind:
      // Already null terminated, yay!
      પાછા StringRef(LHS.cString);
    સ્થિતિ StdStringKind: {
      અચલ std::string *str = LHS.stdString;
      પાછા StringRef(str->c_str(), str->size());
    }
    સ્થિતિ StringLiteralKind:
      પાછા StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    મૂળભૂત:
      તોડો;
    }
  }
  toVector(Out);
  Out.ધકેલો_back(0);
  Out.કાઢો_back();
  પાછા StringRef(Out.data(), Out.size());
}

શૂન્ય Twine::printOneChild(raw_ostream &OS, Child Ptr, ગાંઠ_Kind Kind) અચલ {
  પસંદગી (Kind) {
  સ્થિતિ Twine::NullKind:
    તોડો;
  સ્થિતિ Twine::EmptyKind:
    તોડો;
  સ્થિતિ Twine::TwineKind:
    Ptr.twine->print(OS);
    તોડો;
  સ્થિતિ Twine::CStringKind:
    OS << Ptr.cString;
    તોડો;
  સ્થિતિ Twine::StdStringKind:
    OS << *Ptr.stdString;
    તોડો;
  સ્થિતિ Twine::PtrAndLengthKind:
  સ્થિતિ Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    તોડો;
  સ્થિતિ Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    તોડો;
  સ્થિતિ Twine::CharKind:
    OS << Ptr.character;
    તોડો;
  સ્થિતિ Twine::DecUIKind:
    OS << Ptr.decUI;
    તોડો;
  સ્થિતિ Twine::DecIKind:
    OS << Ptr.decI;
    તોડો;
  સ્થિતિ Twine::DecULKind:
    OS << Ptr.decUL;
    તોડો;
  સ્થિતિ Twine::DecLKind:
    OS << Ptr.decL;
    તોડો;
  સ્થિતિ Twine::DecULLKind:
    OS << Ptr.decULL;
    તોડો;
  સ્થિતિ Twine::DecLLKind:
    OS << Ptr.decLL;
    તોડો;
  સ્થિતિ Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    તોડો;
  }
}

શૂન્ય Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, ગાંઠ_Kind Kind) અચલ {
  પસંદગી (Kind) {
  સ્થિતિ Twine::NullKind:
    OS << "null";
    તોડો;
  સ્થિતિ Twine::EmptyKind:
    OS << "empty";
    તોડો;
  સ્થિતિ Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    તોડો;
  સ્થિતિ Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    તોડો;
  સ્થિતિ Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    તોડો;
  સ્થિતિ Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    તોડો;
  સ્થિતિ Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    તોડો;
  સ્થિતિ Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    તોડો;
  સ્થિતિ Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    તોડો;
  સ્થિતિ Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    તોડો;
  સ્થિતિ Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    તોડો;
  સ્થિતિ Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    તોડો;
  સ્થિતિ Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    તોડો;
  સ્થિતિ Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    તોડો;
  સ્થિતિ Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    તોડો;
  સ્થિતિ Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    તોડો;
  }
}

શૂન્ય Twine::print(raw_ostream &OS) અચલ {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

શૂન્ય Twine::printRepr(raw_ostream &OS) અચલ {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD શૂન્ય Twine::dump() અચલ { print(dbgs()); }

LLVM_DUMP_METHOD શૂન્ય Twine::dumpRepr() અચલ { printRepr(dbgs()); }
#endif
