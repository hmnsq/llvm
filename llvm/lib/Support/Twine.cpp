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

std::string Twine::str() अचर {
  // If we're storing only a std::string, just return it.
  यदि (LHSKind == StdStringKind && RHSKind == EmptyKind)
    फर्काऊ *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  यदि (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    फर्काऊ LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  फर्काऊ toStringRef(Vec).str();
}

शून्य Twine::toVector(SmallVectorImpl<वर्ण> &Out) अचर {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<वर्ण> &Out) अचर {
  यदि (isUnary()) {
    विकल्प (getLHSKind()) {
    पक्ष CStringKind:
      // Already null terminated, yay!
      फर्काऊ StringRef(LHS.cString);
    पक्ष StdStringKind: {
      अचर std::string *str = LHS.stdString;
      फर्काऊ StringRef(str->c_str(), str->size());
    }
    पक्ष StringLiteralKind:
      फर्काऊ StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    स्वभाव:
      रोक;
    }
  }
  toVector(Out);
  Out.थप_back(0);
  Out.झिक_back();
  फर्काऊ StringRef(Out.data(), Out.size());
}

शून्य Twine::printOneChild(raw_ostream &OS, Child Ptr, गाँठो_Kind Kind) अचर {
  विकल्प (Kind) {
  पक्ष Twine::NullKind:
    रोक;
  पक्ष Twine::EmptyKind:
    रोक;
  पक्ष Twine::TwineKind:
    Ptr.twine->print(OS);
    रोक;
  पक्ष Twine::CStringKind:
    OS << Ptr.cString;
    रोक;
  पक्ष Twine::StdStringKind:
    OS << *Ptr.stdString;
    रोक;
  पक्ष Twine::PtrAndLengthKind:
  पक्ष Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    रोक;
  पक्ष Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    रोक;
  पक्ष Twine::CharKind:
    OS << Ptr.character;
    रोक;
  पक्ष Twine::DecUIKind:
    OS << Ptr.decUI;
    रोक;
  पक्ष Twine::DecIKind:
    OS << Ptr.decI;
    रोक;
  पक्ष Twine::DecULKind:
    OS << Ptr.decUL;
    रोक;
  पक्ष Twine::DecLKind:
    OS << Ptr.decL;
    रोक;
  पक्ष Twine::DecULLKind:
    OS << Ptr.decULL;
    रोक;
  पक्ष Twine::DecLLKind:
    OS << Ptr.decLL;
    रोक;
  पक्ष Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    रोक;
  }
}

शून्य Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, गाँठो_Kind Kind) अचर {
  विकल्प (Kind) {
  पक्ष Twine::NullKind:
    OS << "null";
    रोक;
  पक्ष Twine::EmptyKind:
    OS << "empty";
    रोक;
  पक्ष Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    रोक;
  पक्ष Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    रोक;
  पक्ष Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    रोक;
  पक्ष Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    रोक;
  पक्ष Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    रोक;
  पक्ष Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    रोक;
  पक्ष Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    रोक;
  पक्ष Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    रोक;
  पक्ष Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    रोक;
  पक्ष Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    रोक;
  पक्ष Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    रोक;
  पक्ष Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    रोक;
  पक्ष Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    रोक;
  पक्ष Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    रोक;
  }
}

शून्य Twine::print(raw_ostream &OS) अचर {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

शून्य Twine::printRepr(raw_ostream &OS) अचर {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य Twine::dump() अचर { print(dbgs()); }

LLVM_DUMP_METHOD शून्य Twine::dumpRepr() अचर { printRepr(dbgs()); }
#endif
