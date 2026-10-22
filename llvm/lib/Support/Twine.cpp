//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/Twine.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/FormatVariadic.h"
#समावेश "llvm/Support/raw_ostream.h"
using namespace llvm;

std::string Twine::str() स्थिर {
  // If we're storing only a std::string, just return it.
  अगर (LHSKind == StdStringKind && RHSKind == EmptyKind)
    वापस *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  अगर (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    वापस LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  वापस toStringRef(Vec).str();
}

व्योम Twine::toVector(SmallVectorImpl<अक्षर> &Out) स्थिर {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<अक्षर> &Out) स्थिर {
  अगर (isUnary()) {
    चयन (getLHSKind()) {
    हाल CStringKind:
      // Already null terminated, yay!
      वापस StringRef(LHS.cString);
    हाल StdStringKind: {
      स्थिर std::string *str = LHS.stdString;
      वापस StringRef(str->c_str(), str->आकार_2());
    }
    हाल StringLiteralKind:
      वापस StringRef(LHS.ptr_And_लंबाई.ptr, LHS.ptr_And_लंबाई.लंबाई_2);
    शेष:
      अवरोध;
    }
  }
  toVector(Out);
  Out.धकेलो_back(0);
  Out.निकालो_back();
  वापस StringRef(Out.तथ्य(), Out.आकार_2());
}

व्योम Twine::printOneChild(raw_ostream &OS, Child Ptr, गाँठ_Kind Kind) स्थिर {
  चयन (Kind) {
  हाल Twine::NullKind:
    अवरोध;
  हाल Twine::EmptyKind:
    अवरोध;
  हाल Twine::TwineKind:
    Ptr.twine->print(OS);
    अवरोध;
  हाल Twine::CStringKind:
    OS << Ptr.cString;
    अवरोध;
  हाल Twine::StdStringKind:
    OS << *Ptr.stdString;
    अवरोध;
  हाल Twine::Ptr_And_लंबाई_Kind:
  हाल Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptr_And_लंबाई.ptr, Ptr.ptr_And_लंबाई.लंबाई_2);
    अवरोध;
  हाल Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    अवरोध;
  हाल Twine::CharKind:
    OS << Ptr.character;
    अवरोध;
  हाल Twine::DecUIKind:
    OS << Ptr.decUI;
    अवरोध;
  हाल Twine::DecIKind:
    OS << Ptr.decI;
    अवरोध;
  हाल Twine::DecULKind:
    OS << Ptr.decUL;
    अवरोध;
  हाल Twine::DecLKind:
    OS << Ptr.decL;
    अवरोध;
  हाल Twine::DecULLKind:
    OS << Ptr.decULL;
    अवरोध;
  हाल Twine::DecLLKind:
    OS << Ptr.decLL;
    अवरोध;
  हाल Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    अवरोध;
  }
}

व्योम Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, गाँठ_Kind Kind) स्थिर {
  चयन (Kind) {
  हाल Twine::NullKind:
    OS << "null";
    अवरोध;
  हाल Twine::EmptyKind:
    OS << "empty";
    अवरोध;
  हाल Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    अवरोध;
  हाल Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    अवरोध;
  हाल Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    अवरोध;
  हाल Twine::Ptr_And_लंबाई_Kind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_लंबाई.ptr, Ptr.ptr_And_लंबाई.लंबाई_2) << "\"";
    अवरोध;
  हाल Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_लंबाई.ptr, Ptr.ptr_And_लंबाई.लंबाई_2) << "\"";
    अवरोध;
  हाल Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    अवरोध;
  हाल Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    अवरोध;
  हाल Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    अवरोध;
  हाल Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    अवरोध;
  हाल Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    अवरोध;
  हाल Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    अवरोध;
  हाल Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    अवरोध;
  हाल Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    अवरोध;
  हाल Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    अवरोध;
  }
}

व्योम Twine::print(raw_ostream &OS) स्थिर {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

व्योम Twine::printRepr(raw_ostream &OS) स्थिर {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम Twine::dump() स्थिर { print(dbgs()); }

LLVM_DUMP_METHOD व्योम Twine::dumpRepr() स्थिर { printRepr(dbgs()); }
#पूर्ण_अगर
