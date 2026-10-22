//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/FormatVariadic.h"
#ضمن "llvm/Support/raw_ostream.h"
using نطاق llvm;

std::string Twine::str() ثابت {
  // If we're storing only a std::string, just return it.
  إذا (LHSKind == StdStringKind && RHSKind == EmptyKind)
    أرجع *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  إذا (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    أرجع LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  أرجع toStringRef(Vec).str();
}

فراغ Twine::toVector(SmallVectorImpl<حرف> &Out) ثابت {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<حرف> &Out) ثابت {
  إذا (isUnary()) {
    بدل (getLHSKind()) {
    حالة CStringKind:
      // Already null terminated, yay!
      أرجع StringRef(LHS.cString);
    حالة StdStringKind: {
      ثابت std::string *str = LHS.stdString;
      أرجع StringRef(str->c_str(), str->size());
    }
    حالة StringLiteralKind:
      أرجع StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    افتراضي:
      اكسر;
    }
  }
  toVector(Out);
  Out.ئىتتىرىش_back(0);
  Out.چىقىرىش_back();
  أرجع StringRef(Out.data(), Out.size());
}

فراغ Twine::printOneChild(raw_ostream &OS, Child Ptr, تۈگۈن_Kind Kind) ثابت {
  بدل (Kind) {
  حالة Twine::NullKind:
    اكسر;
  حالة Twine::EmptyKind:
    اكسر;
  حالة Twine::TwineKind:
    Ptr.twine->print(OS);
    اكسر;
  حالة Twine::CStringKind:
    OS << Ptr.cString;
    اكسر;
  حالة Twine::StdStringKind:
    OS << *Ptr.stdString;
    اكسر;
  حالة Twine::PtrAndLengthKind:
  حالة Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    اكسر;
  حالة Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    اكسر;
  حالة Twine::CharKind:
    OS << Ptr.character;
    اكسر;
  حالة Twine::DecUIKind:
    OS << Ptr.decUI;
    اكسر;
  حالة Twine::DecIKind:
    OS << Ptr.decI;
    اكسر;
  حالة Twine::DecULKind:
    OS << Ptr.decUL;
    اكسر;
  حالة Twine::DecLKind:
    OS << Ptr.decL;
    اكسر;
  حالة Twine::DecULLKind:
    OS << Ptr.decULL;
    اكسر;
  حالة Twine::DecLLKind:
    OS << Ptr.decLL;
    اكسر;
  حالة Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    اكسر;
  }
}

فراغ Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, تۈگۈن_Kind Kind) ثابت {
  بدل (Kind) {
  حالة Twine::NullKind:
    OS << "null";
    اكسر;
  حالة Twine::EmptyKind:
    OS << "empty";
    اكسر;
  حالة Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    اكسر;
  حالة Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    اكسر;
  حالة Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    اكسر;
  حالة Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    اكسر;
  حالة Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    اكسر;
  حالة Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    اكسر;
  حالة Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    اكسر;
  حالة Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    اكسر;
  حالة Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    اكسر;
  حالة Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    اكسر;
  حالة Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    اكسر;
  حالة Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    اكسر;
  حالة Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    اكسر;
  حالة Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    اكسر;
  }
}

فراغ Twine::print(raw_ostream &OS) ثابت {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

فراغ Twine::printRepr(raw_ostream &OS) ثابت {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ Twine::dump() ثابت { print(dbgs()); }

LLVM_DUMP_METHOD فراغ Twine::dumpRepr() ثابت { printRepr(dbgs()); }
#endif
