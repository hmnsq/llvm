//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FormatVariadic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

std::string Twine::str() ਅਚਲ {
  // If we're storing only a std::string, just return it.
  ਜੇ (LHSKind == Std_String_ਕਿਸਮ && RHSKind == ਖਾਲੀ_ਥਾਵਾਂ_ਕਿਸਮ)
    ਵਾਪਸ *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  ਜੇ (LHSKind == Formatv_Object_ਕਿਸਮ && RHSKind == ਖਾਲੀ_ਥਾਵਾਂ_ਕਿਸਮ)
    ਵਾਪਸ LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  ਵਾਪਸ ਮੰਜ਼ਿਲ_String_Ref(Vec).str();
}

ਖਾਲੀ Twine::ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਬਾਹਰਲਾ) ਅਚਲ {
  raw_svector_ostream OS(ਬਾਹਰਲਾ);
  print(OS);
}

StringRef Twine::ਮੰਜ਼ਿਲ_Null_Terminated_String_Ref(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਬਾਹਰਲਾ) ਅਚਲ {
  ਜੇ (isUnary()) {
    ਚੋਣ (getLHSKind()) {
    ਹਾਲਤ CString_ਕਿਸਮ:
      // Already null terminated, yay!
      ਵਾਪਸ StringRef(LHS.cString);
    ਹਾਲਤ Std_String_ਕਿਸਮ: {
      ਅਚਲ std::string *str = LHS.stdString;
      ਵਾਪਸ StringRef(str->c_str(), str->ਮਾਪ_2());
    }
    ਹਾਲਤ String_Literal_ਕਿਸਮ:
      ਵਾਪਸ StringRef(LHS.ptr_And_ਲੰਮਾਈ.ptr, LHS.ptr_And_ਲੰਮਾਈ.ਲੰਮਾਈ_2);
    ਮੂਲ:
      ਰੋਕੋ;
    }
  }
  ਮੰਜ਼ਿਲ_ਸਦਿਸ਼(ਬਾਹਰਲਾ);
  ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(0);
  ਬਾਹਰਲਾ.ਕੱਢੋ_ਪਿੱਠ();
  ਵਾਪਸ StringRef(ਬਾਹਰਲਾ.ਅੰਕੜੇ(), ਬਾਹਰਲਾ.ਮਾਪ_2());
}

ਖਾਲੀ Twine::print_One_ਬੱਚਾ(raw_ostream &OS, ਬੱਚਾ Ptr, ਗੰਢ_ਕਿਸਮ ਕਿਸਮ_3) ਅਚਲ {
  ਚੋਣ (ਕਿਸਮ_3) {
  ਹਾਲਤ Twine::Null_ਕਿਸਮ:
    ਰੋਕੋ;
  ਹਾਲਤ Twine::ਖਾਲੀ_ਥਾਵਾਂ_ਕਿਸਮ:
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Twine_ਕਿਸਮ:
    Ptr.twine->print(OS);
    ਰੋਕੋ;
  ਹਾਲਤ Twine::CString_ਕਿਸਮ:
    OS << Ptr.cString;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Std_String_ਕਿਸਮ:
    OS << *Ptr.stdString;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Ptr_And_ਲੰਮਾਈ_ਕਿਸਮ:
  ਹਾਲਤ Twine::String_Literal_ਕਿਸਮ:
    OS << StringRef(Ptr.ptr_And_ਲੰਮਾਈ.ptr, Ptr.ptr_And_ਲੰਮਾਈ.ਲੰਮਾਈ_2);
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Formatv_Object_ਕਿਸਮ:
    OS << *Ptr.formatvObject;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Char_ਕਿਸਮ:
    OS << Ptr.character;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecUIKind:
    OS << Ptr.decUI;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecIKind:
    OS << Ptr.decI;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecULKind:
    OS << Ptr.decUL;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecLKind:
    OS << Ptr.decL;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecULLKind:
    OS << Ptr.decULL;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecLLKind:
    OS << Ptr.decLL;
    ਰੋਕੋ;
  ਹਾਲਤ Twine::UHex_ਕਿਸਮ:
    OS.write_hex(Ptr.uHex);
    ਰੋਕੋ;
  }
}

ਖਾਲੀ Twine::print_One_ਬੱਚਾ_Repr(raw_ostream &OS, ਬੱਚਾ Ptr, ਗੰਢ_ਕਿਸਮ ਕਿਸਮ_3) ਅਚਲ {
  ਚੋਣ (ਕਿਸਮ_3) {
  ਹਾਲਤ Twine::Null_ਕਿਸਮ:
    OS << "null";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::ਖਾਲੀ_ਥਾਵਾਂ_ਕਿਸਮ:
    OS << "empty";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Twine_ਕਿਸਮ:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    ਰੋਕੋ;
  ਹਾਲਤ Twine::CString_ਕਿਸਮ:
    OS << "cstring:\"" << Ptr.cString << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Std_String_ਕਿਸਮ:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Ptr_And_ਲੰਮਾਈ_ਕਿਸਮ:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_ਲੰਮਾਈ.ptr, Ptr.ptr_And_ਲੰਮਾਈ.ਲੰਮਾਈ_2) << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::String_Literal_ਕਿਸਮ:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_ਲੰਮਾਈ.ptr, Ptr.ptr_And_ਲੰਮਾਈ.ਲੰਮਾਈ_2) << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Formatv_Object_ਕਿਸਮ:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::Char_ਕਿਸਮ:
    OS << "char:\"" << Ptr.character << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    ਰੋਕੋ;
  ਹਾਲਤ Twine::UHex_ਕਿਸਮ:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    ਰੋਕੋ;
  }
}

ਖਾਲੀ Twine::print(raw_ostream &OS) ਅਚਲ {
  print_One_ਬੱਚਾ(OS, LHS, getLHSKind());
  print_One_ਬੱਚਾ(OS, RHS, getRHSKind());
}

ਖਾਲੀ Twine::printRepr(raw_ostream &OS) ਅਚਲ {
  OS << "(Twine ";
  print_One_ਬੱਚਾ_Repr(OS, LHS, getLHSKind());
  OS << " ";
  print_One_ਬੱਚਾ_Repr(OS, RHS, getRHSKind());
  OS << ")";
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ Twine::dump() ਅਚਲ { print(dbgs()); }

LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ Twine::dumpRepr() ਅਚਲ { printRepr(dbgs()); }
#ਜੇ_ਅੰਤ
