//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/Twine.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/FormatVariadic.h"
#شامل "llvm/Support/raw_ostream.h"
using فضاینام llvm;

std::string Twine::str() ثابت {
  // If we're storing only a std::string, just return it.
  اگر (LHSKind == StdStringKind && RHSKind == EmptyKind)
    بازگشت *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  اگر (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    بازگشت LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  بازگشت toStringRef(Vec).str();
}

تهی Twine::toVector(SmallVectorImpl<نویسه> &Out) ثابت {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<نویسه> &Out) ثابت {
  اگر (isUnary()) {
    انتخاب (getLHSKind()) {
    حالت CStringKind:
      // Already null terminated, yay!
      بازگشت StringRef(LHS.cString);
    حالت StdStringKind: {
      ثابت std::string *str = LHS.stdString;
      بازگشت StringRef(str->c_str(), str->size());
    }
    حالت StringLiteralKind:
      بازگشت StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    پیشفرض:
      بشکن;
    }
  }
  toVector(Out);
  Out.افزودن_back(0);
  Out.برداشت_back();
  بازگشت StringRef(Out.data(), Out.size());
}

تهی Twine::printOneChild(raw_ostream &OS, Child Ptr, گره_Kind Kind) ثابت {
  انتخاب (Kind) {
  حالت Twine::NullKind:
    بشکن;
  حالت Twine::EmptyKind:
    بشکن;
  حالت Twine::TwineKind:
    Ptr.twine->print(OS);
    بشکن;
  حالت Twine::CStringKind:
    OS << Ptr.cString;
    بشکن;
  حالت Twine::StdStringKind:
    OS << *Ptr.stdString;
    بشکن;
  حالت Twine::PtrAndLengthKind:
  حالت Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    بشکن;
  حالت Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    بشکن;
  حالت Twine::CharKind:
    OS << Ptr.character;
    بشکن;
  حالت Twine::DecUIKind:
    OS << Ptr.decUI;
    بشکن;
  حالت Twine::DecIKind:
    OS << Ptr.decI;
    بشکن;
  حالت Twine::DecULKind:
    OS << Ptr.decUL;
    بشکن;
  حالت Twine::DecLKind:
    OS << Ptr.decL;
    بشکن;
  حالت Twine::DecULLKind:
    OS << Ptr.decULL;
    بشکن;
  حالت Twine::DecLLKind:
    OS << Ptr.decLL;
    بشکن;
  حالت Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    بشکن;
  }
}

تهی Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, گره_Kind Kind) ثابت {
  انتخاب (Kind) {
  حالت Twine::NullKind:
    OS << "null";
    بشکن;
  حالت Twine::EmptyKind:
    OS << "empty";
    بشکن;
  حالت Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    بشکن;
  حالت Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    بشکن;
  حالت Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    بشکن;
  حالت Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    بشکن;
  حالت Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    بشکن;
  حالت Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    بشکن;
  حالت Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    بشکن;
  حالت Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    بشکن;
  حالت Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    بشکن;
  حالت Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    بشکن;
  حالت Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    بشکن;
  حالت Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    بشکن;
  حالت Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    بشکن;
  حالت Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    بشکن;
  }
}

تهی Twine::print(raw_ostream &OS) ثابت {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

تهی Twine::printRepr(raw_ostream &OS) ثابت {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD تهی Twine::dump() ثابت { print(dbgs()); }

LLVM_DUMP_METHOD تهی Twine::dumpRepr() ثابت { printRepr(dbgs()); }
#endif
