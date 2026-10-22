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
ورتوں نطاق llvm;

std::string Twine::str() ثابت {
  // If we're storing only a std::string, just return it.
  إذا (LHSKind == Std_String_قسم && RHSKind == خالی_تھاواں_قسم)
    أرجع *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  إذا (LHSKind == Formatv_Object_قسم && RHSKind == خالی_تھاواں_قسم)
    أرجع LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  أرجع منزل_String_Ref(Vec).str();
}

فراغ Twine::منزل_سمتیہ(Small_سمتیہ_Impl<حرف> &باہرلا) ثابت {
  raw_svector_ostream OS(باہرلا);
  print(OS);
}

StringRef Twine::منزل_Null_Terminated_String_Ref(Small_سمتیہ_Impl<حرف> &باہرلا) ثابت {
  إذا (isUnary()) {
    بدل (getLHSKind()) {
    حالة CString_قسم:
      // Already null terminated, yay!
      أرجع StringRef(LHS.cString);
    حالة Std_String_قسم: {
      ثابت std::string *str = LHS.stdString;
      أرجع StringRef(str->c_str(), str->ناپ_2());
    }
    حالة String_Literal_قسم:
      أرجع StringRef(LHS.ptr_And_لمائی.ptr, LHS.ptr_And_لمائی.لمائی_2);
    افتراضي:
      اكسر;
    }
  }
  منزل_سمتیہ(باہرلا);
  باہرلا.ڈالو_پٹھ(0);
  باہرلا.نکالو_پٹھ();
  أرجع StringRef(باہرلا.انکڑے(), باہرلا.ناپ_2());
}

فراغ Twine::print_One_بچہ(raw_ostream &OS, بچہ Ptr, گرہ_قسم قسم_2) ثابت {
  بدل (قسم_2) {
  حالة Twine::Null_قسم:
    اكسر;
  حالة Twine::خالی_تھاواں_قسم:
    اكسر;
  حالة Twine::Twine_قسم:
    Ptr.twine->print(OS);
    اكسر;
  حالة Twine::CString_قسم:
    OS << Ptr.cString;
    اكسر;
  حالة Twine::Std_String_قسم:
    OS << *Ptr.stdString;
    اكسر;
  حالة Twine::Ptr_And_لمائی_قسم:
  حالة Twine::String_Literal_قسم:
    OS << StringRef(Ptr.ptr_And_لمائی.ptr, Ptr.ptr_And_لمائی.لمائی_2);
    اكسر;
  حالة Twine::Formatv_Object_قسم:
    OS << *Ptr.formatvObject;
    اكسر;
  حالة Twine::Char_قسم:
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
  حالة Twine::UHex_قسم:
    OS.write_hex(Ptr.uHex);
    اكسر;
  }
}

فراغ Twine::print_One_بچہ_Repr(raw_ostream &OS, بچہ Ptr, گرہ_قسم قسم_2) ثابت {
  بدل (قسم_2) {
  حالة Twine::Null_قسم:
    OS << "null";
    اكسر;
  حالة Twine::خالی_تھاواں_قسم:
    OS << "empty";
    اكسر;
  حالة Twine::Twine_قسم:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    اكسر;
  حالة Twine::CString_قسم:
    OS << "cstring:\"" << Ptr.cString << "\"";
    اكسر;
  حالة Twine::Std_String_قسم:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    اكسر;
  حالة Twine::Ptr_And_لمائی_قسم:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_لمائی.ptr, Ptr.ptr_And_لمائی.لمائی_2) << "\"";
    اكسر;
  حالة Twine::String_Literal_قسم:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_لمائی.ptr, Ptr.ptr_And_لمائی.لمائی_2) << "\"";
    اكسر;
  حالة Twine::Formatv_Object_قسم:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    اكسر;
  حالة Twine::Char_قسم:
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
  حالة Twine::UHex_قسم:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    اكسر;
  }
}

فراغ Twine::print(raw_ostream &OS) ثابت {
  print_One_بچہ(OS, LHS, getLHSKind());
  print_One_بچہ(OS, RHS, getRHSKind());
}

فراغ Twine::printRepr(raw_ostream &OS) ثابت {
  OS << "(Twine ";
  print_One_بچہ_Repr(OS, LHS, getLHSKind());
  OS << " ";
  print_One_بچہ_Repr(OS, RHS, getRHSKind());
  OS << ")";
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ فراغ Twine::dump() ثابت { print(dbgs()); }

LLVM_DUMP_طریقہ فراغ Twine::dumpRepr() ثابت { printRepr(dbgs()); }
#جے_انت
