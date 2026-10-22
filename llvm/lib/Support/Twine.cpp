//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Twine.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

std::string Twine::str() const {
  // If we're storing only a std::string, just return it.
  ᮚᮦᮔ᮪ (LHSKind == StdStringKind && RHSKind == EmptyKind)
    ᮝᮜᮤ *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  ᮚᮦᮔ᮪ (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    ᮝᮜᮤ LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  ᮝᮜᮤ toStringRef(Vec).str();
}

void Twine::toVector(SmallVectorImpl<var_na> &Out) const {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<var_na> &Out) const {
  ᮚᮦᮔ᮪ (isUnary()) {
    switch (getLHSKind()) {
    pak_sa CStringKind:
      // Already null terminated, yay!
      ᮝᮜᮤ StringRef(LHS.cString);
    pak_sa StdStringKind: {
      const std::string *str = LHS.stdString;
      ᮝᮜᮤ StringRef(str->c_str(), str->size());
    }
    pak_sa StringLiteralKind:
      ᮝᮜᮤ StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    default:
      bha_nga;
    }
  }
  toVector(Out);
  Out.push_back(0);
  Out.pop_back();
  ᮝᮜᮤ StringRef(Out.data(), Out.size());
}

void Twine::printOneChild(raw_ostream &OS, Child Ptr, NodeKind Kind) const {
  switch (Kind) {
  pak_sa Twine::NullKind:
    bha_nga;
  pak_sa Twine::EmptyKind:
    bha_nga;
  pak_sa Twine::TwineKind:
    Ptr.twine->print(OS);
    bha_nga;
  pak_sa Twine::CStringKind:
    OS << Ptr.cString;
    bha_nga;
  pak_sa Twine::StdStringKind:
    OS << *Ptr.stdString;
    bha_nga;
  pak_sa Twine::PtrAndLengthKind:
  pak_sa Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    bha_nga;
  pak_sa Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    bha_nga;
  pak_sa Twine::CharKind:
    OS << Ptr.character;
    bha_nga;
  pak_sa Twine::DecUIKind:
    OS << Ptr.decUI;
    bha_nga;
  pak_sa Twine::DecIKind:
    OS << Ptr.decI;
    bha_nga;
  pak_sa Twine::DecULKind:
    OS << Ptr.decUL;
    bha_nga;
  pak_sa Twine::DecLKind:
    OS << Ptr.decL;
    bha_nga;
  pak_sa Twine::DecULLKind:
    OS << Ptr.decULL;
    bha_nga;
  pak_sa Twine::DecLLKind:
    OS << Ptr.decLL;
    bha_nga;
  pak_sa Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    bha_nga;
  }
}

void Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, NodeKind Kind) const {
  switch (Kind) {
  pak_sa Twine::NullKind:
    OS << "null";
    bha_nga;
  pak_sa Twine::EmptyKind:
    OS << "empty";
    bha_nga;
  pak_sa Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    bha_nga;
  pak_sa Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    bha_nga;
  pak_sa Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    bha_nga;
  pak_sa Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    bha_nga;
  pak_sa Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    bha_nga;
  pak_sa Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    bha_nga;
  pak_sa Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    bha_nga;
  pak_sa Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    bha_nga;
  pak_sa Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    bha_nga;
  pak_sa Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    bha_nga;
  pak_sa Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    bha_nga;
  pak_sa Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    bha_nga;
  pak_sa Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    bha_nga;
  pak_sa Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    bha_nga;
  }
}

void Twine::print(raw_ostream &OS) const {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

void Twine::printRepr(raw_ostream &OS) const {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#ᮚᮦᮔ᮪ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void Twine::dump() const { print(dbgs()); }

LLVM_DUMP_METHOD void Twine::dumpRepr() const { printRepr(dbgs()); }
#endif
