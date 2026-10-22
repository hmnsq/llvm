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
استفاده نطاق llvm;

std::string Twine::str() ثابت {
  // If we're storing only a std::string, just return it.
  إذا (LHSKind == Std_String_نوع && RHSKind == خالی_نوع)
    أرجع *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  إذا (LHSKind == Formatv_Object_نوع && RHSKind == خالی_نوع)
    أرجع LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  أرجع مقصد_String_Ref(Vec).str();
}

فراغ Twine::مقصد_وکتور(Small_وکتور_Impl<حرف> &بیرون) ثابت {
  raw_svector_ostream OS(بیرون);
  print(OS);
}

StringRef Twine::مقصد_Null_Terminated_String_Ref(Small_وکتور_Impl<حرف> &بیرون) ثابت {
  إذا (isUnary()) {
    بدل (getLHSKind()) {
    حالة CString_نوع:
      // Already null terminated, yay!
      أرجع StringRef(LHS.cString);
    حالة Std_String_نوع: {
      ثابت std::string *str = LHS.stdString;
      أرجع StringRef(str->c_str(), str->اندازه_2());
    }
    حالة String_Literal_نوع:
      أرجع StringRef(LHS.ptr_And_درازی.ptr, LHS.ptr_And_درازی.درازی_2);
    افتراضي:
      اكسر;
    }
  }
  مقصد_وکتور(بیرون);
  بیرون.افزودن_پشت(0);
  بیرون.برداشت_پشت();
  أرجع StringRef(بیرون.داده(), بیرون.اندازه_2());
}

فراغ Twine::print_One_فرزند(raw_ostream &OS, فرزند Ptr, گره_نوع نوع_3) ثابت {
  بدل (نوع_3) {
  حالة Twine::Null_نوع:
    اكسر;
  حالة Twine::خالی_نوع:
    اكسر;
  حالة Twine::Twine_نوع:
    Ptr.twine->print(OS);
    اكسر;
  حالة Twine::CString_نوع:
    OS << Ptr.cString;
    اكسر;
  حالة Twine::Std_String_نوع:
    OS << *Ptr.stdString;
    اكسر;
  حالة Twine::Ptr_And_درازی_نوع:
  حالة Twine::String_Literal_نوع:
    OS << StringRef(Ptr.ptr_And_درازی.ptr, Ptr.ptr_And_درازی.درازی_2);
    اكسر;
  حالة Twine::Formatv_Object_نوع:
    OS << *Ptr.formatvObject;
    اكسر;
  حالة Twine::Char_نوع:
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
  حالة Twine::UHex_نوع:
    OS.write_hex(Ptr.uHex);
    اكسر;
  }
}

فراغ Twine::print_One_فرزند_Repr(raw_ostream &OS, فرزند Ptr, گره_نوع نوع_3) ثابت {
  بدل (نوع_3) {
  حالة Twine::Null_نوع:
    OS << "null";
    اكسر;
  حالة Twine::خالی_نوع:
    OS << "empty";
    اكسر;
  حالة Twine::Twine_نوع:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    اكسر;
  حالة Twine::CString_نوع:
    OS << "cstring:\"" << Ptr.cString << "\"";
    اكسر;
  حالة Twine::Std_String_نوع:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    اكسر;
  حالة Twine::Ptr_And_درازی_نوع:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_درازی.ptr, Ptr.ptr_And_درازی.درازی_2) << "\"";
    اكسر;
  حالة Twine::String_Literal_نوع:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_درازی.ptr, Ptr.ptr_And_درازی.درازی_2) << "\"";
    اكسر;
  حالة Twine::Formatv_Object_نوع:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    اكسر;
  حالة Twine::Char_نوع:
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
  حالة Twine::UHex_نوع:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    اكسر;
  }
}

فراغ Twine::print(raw_ostream &OS) ثابت {
  print_One_فرزند(OS, LHS, getLHSKind());
  print_One_فرزند(OS, RHS, getRHSKind());
}

فراغ Twine::printRepr(raw_ostream &OS) ثابت {
  OS << "(Twine ";
  print_One_فرزند_Repr(OS, LHS, getLHSKind());
  OS << " ";
  print_One_فرزند_Repr(OS, RHS, getRHSKind());
  OS << ")";
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_روش فراغ Twine::dump() ثابت { print(dbgs()); }

LLVM_DUMP_روش فراغ Twine::dumpRepr() ثابت { printRepr(dbgs()); }
#پایان_اگر
