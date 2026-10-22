//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/ADT/Twine.h"
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/FormatVariadic.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
using নামাকাশ llvm;

std::string Twine::str() ধ্রুবক {
  // If we're storing only a std::string, just return it.
  যদি (LHSKind == StdStringKind && RHSKind == EmptyKind)
    ফেরত *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  যদি (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    ফেরত LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  ফেরত toStringRef(Vec).str();
}

শূন্য Twine::toVector(SmallVectorImpl<অক্ষর> &Out) ধ্রুবক {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<অক্ষর> &Out) ধ্রুবক {
  যদি (isUnary()) {
    নির্বাচন (getLHSKind()) {
    ক্ষেত্র CStringKind:
      // Already null terminated, yay!
      ফেরত StringRef(LHS.cString);
    ক্ষেত্র StdStringKind: {
      ধ্রুবক std::string *str = LHS.stdString;
      ফেরত StringRef(str->c_str(), str->আকার_3());
    }
    ক্ষেত্র StringLiteralKind:
      ফেরত StringRef(LHS.ptr_And_দৈর্ঘ্য.ptr, LHS.ptr_And_দৈর্ঘ্য.দৈর্ঘ্য_2);
    পূর্বনির্ধারিত:
      বিরতি;
    }
  }
  toVector(Out);
  Out.ঠেলো_back(0);
  Out.বের_করো_back();
  ফেরত StringRef(Out.তথ্য(), Out.আকার_3());
}

শূন্য Twine::printOneChild(raw_ostream &OS, Child Ptr, গ্রন্থি_Kind Kind) ধ্রুবক {
  নির্বাচন (Kind) {
  ক্ষেত্র Twine::NullKind:
    বিরতি;
  ক্ষেত্র Twine::EmptyKind:
    বিরতি;
  ক্ষেত্র Twine::TwineKind:
    Ptr.twine->print(OS);
    বিরতি;
  ক্ষেত্র Twine::CStringKind:
    OS << Ptr.cString;
    বিরতি;
  ক্ষেত্র Twine::StdStringKind:
    OS << *Ptr.stdString;
    বিরতি;
  ক্ষেত্র Twine::Ptr_And_দৈর্ঘ্য_Kind:
  ক্ষেত্র Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptr_And_দৈর্ঘ্য.ptr, Ptr.ptr_And_দৈর্ঘ্য.দৈর্ঘ্য_2);
    বিরতি;
  ক্ষেত্র Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    বিরতি;
  ক্ষেত্র Twine::CharKind:
    OS << Ptr.character;
    বিরতি;
  ক্ষেত্র Twine::DecUIKind:
    OS << Ptr.decUI;
    বিরতি;
  ক্ষেত্র Twine::DecIKind:
    OS << Ptr.decI;
    বিরতি;
  ক্ষেত্র Twine::DecULKind:
    OS << Ptr.decUL;
    বিরতি;
  ক্ষেত্র Twine::DecLKind:
    OS << Ptr.decL;
    বিরতি;
  ক্ষেত্র Twine::DecULLKind:
    OS << Ptr.decULL;
    বিরতি;
  ক্ষেত্র Twine::DecLLKind:
    OS << Ptr.decLL;
    বিরতি;
  ক্ষেত্র Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    বিরতি;
  }
}

শূন্য Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, গ্রন্থি_Kind Kind) ধ্রুবক {
  নির্বাচন (Kind) {
  ক্ষেত্র Twine::NullKind:
    OS << "null";
    বিরতি;
  ক্ষেত্র Twine::EmptyKind:
    OS << "empty";
    বিরতি;
  ক্ষেত্র Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    বিরতি;
  ক্ষেত্র Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    বিরতি;
  ক্ষেত্র Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    বিরতি;
  ক্ষেত্র Twine::Ptr_And_দৈর্ঘ্য_Kind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_দৈর্ঘ্য.ptr, Ptr.ptr_And_দৈর্ঘ্য.দৈর্ঘ্য_2) << "\"";
    বিরতি;
  ক্ষেত্র Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_দৈর্ঘ্য.ptr, Ptr.ptr_And_দৈর্ঘ্য.দৈর্ঘ্য_2) << "\"";
    বিরতি;
  ক্ষেত্র Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    বিরতি;
  ক্ষেত্র Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    বিরতি;
  ক্ষেত্র Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    বিরতি;
  ক্ষেত্র Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    বিরতি;
  ক্ষেত্র Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    বিরতি;
  ক্ষেত্র Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    বিরতি;
  ক্ষেত্র Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    বিরতি;
  ক্ষেত্র Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    বিরতি;
  ক্ষেত্র Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    বিরতি;
  }
}

শূন্য Twine::print(raw_ostream &OS) ধ্রুবক {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

শূন্য Twine::printRepr(raw_ostream &OS) ধ্রুবক {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য Twine::dump() ধ্রুবক { print(dbgs()); }

LLVM_DUMP_METHOD শূন্য Twine::dumpRepr() ধ্রুবক { printRepr(dbgs()); }
#endif
