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
استعمال نامخلا llvm;

std::string Twine::str() ثابت {
  // If we're storing only a std::string, just return it.
  اگر (LHSKind == Std_String_قسم && RHSKind == خالی_خانے_قسم)
    واپس *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  اگر (LHSKind == Formatv_Object_قسم && RHSKind == خالی_خانے_قسم)
    واپس LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  واپس مقصد_String_Ref(Vec).str();
}

خالی Twine::مقصد_سمتیہ(Small_سمتیہ_Impl<حرف> &برآمد) ثابت {
  raw_svector_ostream OS(برآمد);
  print(OS);
}

StringRef Twine::مقصد_Null_Terminated_String_Ref(Small_سمتیہ_Impl<حرف> &برآمد) ثابت {
  اگر (isUnary()) {
    انتخاب (getLHSKind()) {
    حالت CString_قسم:
      // Already null terminated, yay!
      واپس StringRef(LHS.cString);
    حالت Std_String_قسم: {
      ثابت std::string *str = LHS.stdString;
      واپس StringRef(str->c_str(), str->سائز_2());
    }
    حالت String_Literal_قسم:
      واپس StringRef(LHS.ptr_And_لمبائی.ptr, LHS.ptr_And_لمبائی.لمبائی_2);
    طےشدہ:
      توڑو;
    }
  }
  مقصد_سمتیہ(برآمد);
  برآمد.ڈالو_عقب(0);
  برآمد.نکالو_عقب();
  واپس StringRef(برآمد.ڈیٹا(), برآمد.سائز_2());
}

خالی Twine::print_One_بچہ(raw_ostream &OS, بچہ Ptr, گرہ_قسم قسم_3) ثابت {
  انتخاب (قسم_3) {
  حالت Twine::Null_قسم:
    توڑو;
  حالت Twine::خالی_خانے_قسم:
    توڑو;
  حالت Twine::Twine_قسم:
    Ptr.twine->print(OS);
    توڑو;
  حالت Twine::CString_قسم:
    OS << Ptr.cString;
    توڑو;
  حالت Twine::Std_String_قسم:
    OS << *Ptr.stdString;
    توڑو;
  حالت Twine::Ptr_And_لمبائی_قسم:
  حالت Twine::String_Literal_قسم:
    OS << StringRef(Ptr.ptr_And_لمبائی.ptr, Ptr.ptr_And_لمبائی.لمبائی_2);
    توڑو;
  حالت Twine::Formatv_Object_قسم:
    OS << *Ptr.formatvObject;
    توڑو;
  حالت Twine::Char_قسم:
    OS << Ptr.character;
    توڑو;
  حالت Twine::DecUIKind:
    OS << Ptr.decUI;
    توڑو;
  حالت Twine::DecIKind:
    OS << Ptr.decI;
    توڑو;
  حالت Twine::DecULKind:
    OS << Ptr.decUL;
    توڑو;
  حالت Twine::DecLKind:
    OS << Ptr.decL;
    توڑو;
  حالت Twine::DecULLKind:
    OS << Ptr.decULL;
    توڑو;
  حالت Twine::DecLLKind:
    OS << Ptr.decLL;
    توڑو;
  حالت Twine::UHex_قسم:
    OS.write_hex(Ptr.uHex);
    توڑو;
  }
}

خالی Twine::print_One_بچہ_Repr(raw_ostream &OS, بچہ Ptr, گرہ_قسم قسم_3) ثابت {
  انتخاب (قسم_3) {
  حالت Twine::Null_قسم:
    OS << "null";
    توڑو;
  حالت Twine::خالی_خانے_قسم:
    OS << "empty";
    توڑو;
  حالت Twine::Twine_قسم:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    توڑو;
  حالت Twine::CString_قسم:
    OS << "cstring:\"" << Ptr.cString << "\"";
    توڑو;
  حالت Twine::Std_String_قسم:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    توڑو;
  حالت Twine::Ptr_And_لمبائی_قسم:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_لمبائی.ptr, Ptr.ptr_And_لمبائی.لمبائی_2) << "\"";
    توڑو;
  حالت Twine::String_Literal_قسم:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_لمبائی.ptr, Ptr.ptr_And_لمبائی.لمبائی_2) << "\"";
    توڑو;
  حالت Twine::Formatv_Object_قسم:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    توڑو;
  حالت Twine::Char_قسم:
    OS << "char:\"" << Ptr.character << "\"";
    توڑو;
  حالت Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    توڑو;
  حالت Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    توڑو;
  حالت Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    توڑو;
  حالت Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    توڑو;
  حالت Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    توڑو;
  حالت Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    توڑو;
  حالت Twine::UHex_قسم:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    توڑو;
  }
}

خالی Twine::print(raw_ostream &OS) ثابت {
  print_One_بچہ(OS, LHS, getLHSKind());
  print_One_بچہ(OS, RHS, getRHSKind());
}

خالی Twine::printRepr(raw_ostream &OS) ثابت {
  OS << "(Twine ";
  print_One_بچہ_Repr(OS, LHS, getLHSKind());
  OS << " ";
  print_One_بچہ_Repr(OS, RHS, getRHSKind());
  OS << ")";
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ خالی Twine::dump() ثابت { print(dbgs()); }

LLVM_DUMP_طریقہ خالی Twine::dumpRepr() ثابت { printRepr(dbgs()); }
#اگر_ختم
