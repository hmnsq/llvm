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
کارول نطاق llvm;

std::string Twine::str() ثابت {
  // If we're storing only a std::string, just return it.
  إذا (LHSKind == Std_String_ډول && RHSKind == تشه_ډول)
    أرجع *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  إذا (LHSKind == Formatv_Object_ډول && RHSKind == تشه_ډول)
    أرجع LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  أرجع منزل_String_Ref(Vec).str();
}

فراغ Twine::منزل_ویکټور(Small_ویکټور_Impl<حرف> &بهر) ثابت {
  raw_svector_ostream OS(بهر);
  print(OS);
}

StringRef Twine::منزل_Null_Terminated_String_Ref(Small_ویکټور_Impl<حرف> &بهر) ثابت {
  إذا (isUnary()) {
    بدل (getLHSKind()) {
    حالة CString_ډول:
      // Already null terminated, yay!
      أرجع StringRef(LHS.cString);
    حالة Std_String_ډول: {
      ثابت std::string *str = LHS.stdString;
      أرجع StringRef(str->c_str(), str->اندازه_2());
    }
    حالة String_Literal_ډول:
      أرجع StringRef(LHS.ptr_And_اوږدتیا.ptr, LHS.ptr_And_اوږدتیا.اوږدتیا_2);
    افتراضي:
      اكسر;
    }
  }
  منزل_ویکټور(بهر);
  بهر.ټېل_شا(0);
  بهر.راوباسه_شا();
  أرجع StringRef(بهر.معلومات(), بهر.اندازه_2());
}

فراغ Twine::print_One_ماشوم(raw_ostream &OS, ماشوم Ptr, غوټه_ډول ډول_2) ثابت {
  بدل (ډول_2) {
  حالة Twine::Null_ډول:
    اكسر;
  حالة Twine::تشه_ډول:
    اكسر;
  حالة Twine::Twine_ډول:
    Ptr.twine->print(OS);
    اكسر;
  حالة Twine::CString_ډول:
    OS << Ptr.cString;
    اكسر;
  حالة Twine::Std_String_ډول:
    OS << *Ptr.stdString;
    اكسر;
  حالة Twine::Ptr_And_اوږدتیا_ډول:
  حالة Twine::String_Literal_ډول:
    OS << StringRef(Ptr.ptr_And_اوږدتیا.ptr, Ptr.ptr_And_اوږدتیا.اوږدتیا_2);
    اكسر;
  حالة Twine::Formatv_Object_ډول:
    OS << *Ptr.formatvObject;
    اكسر;
  حالة Twine::Char_ډول:
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
  حالة Twine::UHex_ډول:
    OS.write_hex(Ptr.uHex);
    اكسر;
  }
}

فراغ Twine::print_One_ماشوم_Repr(raw_ostream &OS, ماشوم Ptr, غوټه_ډول ډول_2) ثابت {
  بدل (ډول_2) {
  حالة Twine::Null_ډول:
    OS << "null";
    اكسر;
  حالة Twine::تشه_ډول:
    OS << "empty";
    اكسر;
  حالة Twine::Twine_ډول:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    اكسر;
  حالة Twine::CString_ډول:
    OS << "cstring:\"" << Ptr.cString << "\"";
    اكسر;
  حالة Twine::Std_String_ډول:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    اكسر;
  حالة Twine::Ptr_And_اوږدتیا_ډول:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_اوږدتیا.ptr, Ptr.ptr_And_اوږدتیا.اوږدتیا_2) << "\"";
    اكسر;
  حالة Twine::String_Literal_ډول:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_اوږدتیا.ptr, Ptr.ptr_And_اوږدتیا.اوږدتیا_2) << "\"";
    اكسر;
  حالة Twine::Formatv_Object_ډول:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    اكسر;
  حالة Twine::Char_ډول:
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
  حالة Twine::UHex_ډول:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    اكسر;
  }
}

فراغ Twine::print(raw_ostream &OS) ثابت {
  print_One_ماشوم(OS, LHS, getLHSKind());
  print_One_ماشوم(OS, RHS, getRHSKind());
}

فراغ Twine::printRepr(raw_ostream &OS) ثابت {
  OS << "(Twine ";
  print_One_ماشوم_Repr(OS, LHS, getLHSKind());
  OS << " ";
  print_One_ماشوم_Repr(OS, RHS, getRHSKind());
  OS << ")";
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_لاره فراغ Twine::dump() ثابت { print(dbgs()); }

LLVM_DUMP_لاره فراغ Twine::dumpRepr() ثابت { printRepr(dbgs()); }
#د_که_پای
