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

std::string Twine::str() स्थिरांक {
  // If we're storing only a std::string, just return it.
  जर (LHSKind == StdStringKind && RHSKind == EmptyKind)
    परत *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  जर (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    परत LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  परत toStringRef(Vec).str();
}

शून्य Twine::toVector(SmallVectorImpl<अक्षर> &Out) स्थिरांक {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<अक्षर> &Out) स्थिरांक {
  जर (isUnary()) {
    निवड (getLHSKind()) {
    स्थिती CStringKind:
      // Already null terminated, yay!
      परत StringRef(LHS.cString);
    स्थिती StdStringKind: {
      स्थिरांक std::string *str = LHS.stdString;
      परत StringRef(str->c_str(), str->size());
    }
    स्थिती StringLiteralKind:
      परत StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    पूर्वनिर्धारित:
      थांबा;
    }
  }
  toVector(Out);
  Out.ढकला_back(0);
  Out.काढा_back();
  परत StringRef(Out.data(), Out.size());
}

शून्य Twine::printOneChild(raw_ostream &OS, Child Ptr, गाठ_Kind Kind) स्थिरांक {
  निवड (Kind) {
  स्थिती Twine::NullKind:
    थांबा;
  स्थिती Twine::EmptyKind:
    थांबा;
  स्थिती Twine::TwineKind:
    Ptr.twine->print(OS);
    थांबा;
  स्थिती Twine::CStringKind:
    OS << Ptr.cString;
    थांबा;
  स्थिती Twine::StdStringKind:
    OS << *Ptr.stdString;
    थांबा;
  स्थिती Twine::PtrAndLengthKind:
  स्थिती Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    थांबा;
  स्थिती Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    थांबा;
  स्थिती Twine::CharKind:
    OS << Ptr.character;
    थांबा;
  स्थिती Twine::DecUIKind:
    OS << Ptr.decUI;
    थांबा;
  स्थिती Twine::DecIKind:
    OS << Ptr.decI;
    थांबा;
  स्थिती Twine::DecULKind:
    OS << Ptr.decUL;
    थांबा;
  स्थिती Twine::DecLKind:
    OS << Ptr.decL;
    थांबा;
  स्थिती Twine::DecULLKind:
    OS << Ptr.decULL;
    थांबा;
  स्थिती Twine::DecLLKind:
    OS << Ptr.decLL;
    थांबा;
  स्थिती Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    थांबा;
  }
}

शून्य Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, गाठ_Kind Kind) स्थिरांक {
  निवड (Kind) {
  स्थिती Twine::NullKind:
    OS << "null";
    थांबा;
  स्थिती Twine::EmptyKind:
    OS << "empty";
    थांबा;
  स्थिती Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    थांबा;
  स्थिती Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    थांबा;
  स्थिती Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    थांबा;
  स्थिती Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    थांबा;
  स्थिती Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    थांबा;
  स्थिती Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    थांबा;
  स्थिती Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    थांबा;
  स्थिती Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    थांबा;
  स्थिती Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    थांबा;
  स्थिती Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    थांबा;
  स्थिती Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    थांबा;
  स्थिती Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    थांबा;
  स्थिती Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    थांबा;
  स्थिती Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    थांबा;
  }
}

शून्य Twine::print(raw_ostream &OS) स्थिरांक {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

शून्य Twine::printRepr(raw_ostream &OS) स्थिरांक {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#जर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य Twine::dump() स्थिरांक { print(dbgs()); }

LLVM_DUMP_METHOD शून्य Twine::dumpRepr() स्थिरांक { printRepr(dbgs()); }
#endif
