//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FormatVariadic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

std::string Twine::str() ಸ್ಥಿರಾಂಕ {
  // If we're storing only a std::string, just return it.
  ಆದರೆ (LHSKind == Std_String_ಬಗೆ && RHSKind == ಖಾಲಿಗಳು_ಬಗೆ)
    ಹಿಂತಿರುಗಿ *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  ಆದರೆ (LHSKind == Formatv_Object_ಬಗೆ && RHSKind == ಖಾಲಿಗಳು_ಬಗೆ)
    ಹಿಂತಿರುಗಿ LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  ಹಿಂತಿರುಗಿ ಗಮ್ಯ_String_Ref(Vec).str();
}

ಶೂನ್ಯ Twine::ಗಮ್ಯ_ಸದಿಶ(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಹೊರಹೋಗು) ಸ್ಥಿರಾಂಕ {
  raw_svector_ostream OS(ಹೊರಹೋಗು);
  print(OS);
}

StringRef Twine::ಗಮ್ಯ_Null_Terminated_String_Ref(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಹೊರಹೋಗು) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (isUnary()) {
    ಆಯ್ಕೆ (getLHSKind()) {
    ಸ್ಥಿತಿ CString_ಬಗೆ:
      // Already null terminated, yay!
      ಹಿಂತಿರುಗಿ StringRef(LHS.cString);
    ಸ್ಥಿತಿ Std_String_ಬಗೆ: {
      ಸ್ಥಿರಾಂಕ std::string *str = LHS.stdString;
      ಹಿಂತಿರುಗಿ StringRef(str->c_str(), str->ಗಾತ್ರ_3());
    }
    ಸ್ಥಿತಿ String_Literal_ಬಗೆ:
      ಹಿಂತಿರುಗಿ StringRef(LHS.ptr_And_ಉದ್ದಳತೆ.ptr, LHS.ptr_And_ಉದ್ದಳತೆ.ಉದ್ದಳತೆ_2);
    ಪೂರ್ವನಿಯೋಜಿತ:
      ನಿಲ್ಲಿಸು;
    }
  }
  ಗಮ್ಯ_ಸದಿಶ(ಹೊರಹೋಗು);
  ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(0);
  ಹೊರಹೋಗು.ತೆಗೆ_ಹಿಂಭಾಗ();
  ಹಿಂತಿರುಗಿ StringRef(ಹೊರಹೋಗು.ದತ್ತಾಂಶ(), ಹೊರಹೋಗು.ಗಾತ್ರ_3());
}

ಶೂನ್ಯ Twine::print_One_ಮಗು(raw_ostream &OS, ಮಗು Ptr, ಗಂಟು_ಬಗೆ ಬಗೆ_2) ಸ್ಥಿರಾಂಕ {
  ಆಯ್ಕೆ (ಬಗೆ_2) {
  ಸ್ಥಿತಿ Twine::Null_ಬಗೆ:
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::ಖಾಲಿಗಳು_ಬಗೆ:
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Twine_ಬಗೆ:
    Ptr.twine->print(OS);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::CString_ಬಗೆ:
    OS << Ptr.cString;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Std_String_ಬಗೆ:
    OS << *Ptr.stdString;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Ptr_And_ಉದ್ದಳತೆ_ಬಗೆ:
  ಸ್ಥಿತಿ Twine::String_Literal_ಬಗೆ:
    OS << StringRef(Ptr.ptr_And_ಉದ್ದಳತೆ.ptr, Ptr.ptr_And_ಉದ್ದಳತೆ.ಉದ್ದಳತೆ_2);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Formatv_Object_ಬಗೆ:
    OS << *Ptr.formatvObject;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Char_ಬಗೆ:
    OS << Ptr.character;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecUIKind:
    OS << Ptr.decUI;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecIKind:
    OS << Ptr.decI;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecULKind:
    OS << Ptr.decUL;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecLKind:
    OS << Ptr.decL;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecULLKind:
    OS << Ptr.decULL;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecLLKind:
    OS << Ptr.decLL;
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::UHex_ಬಗೆ:
    OS.write_hex(Ptr.uHex);
    ನಿಲ್ಲಿಸು;
  }
}

ಶೂನ್ಯ Twine::print_One_ಮಗು_Repr(raw_ostream &OS, ಮಗು Ptr, ಗಂಟು_ಬಗೆ ಬಗೆ_2) ಸ್ಥಿರಾಂಕ {
  ಆಯ್ಕೆ (ಬಗೆ_2) {
  ಸ್ಥಿತಿ Twine::Null_ಬಗೆ:
    OS << "null";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::ಖಾಲಿಗಳು_ಬಗೆ:
    OS << "empty";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Twine_ಬಗೆ:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::CString_ಬಗೆ:
    OS << "cstring:\"" << Ptr.cString << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Std_String_ಬಗೆ:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Ptr_And_ಉದ್ದಳತೆ_ಬಗೆ:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_ಉದ್ದಳತೆ.ptr, Ptr.ptr_And_ಉದ್ದಳತೆ.ಉದ್ದಳತೆ_2) << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::String_Literal_ಬಗೆ:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_ಉದ್ದಳತೆ.ptr, Ptr.ptr_And_ಉದ್ದಳತೆ.ಉದ್ದಳತೆ_2) << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Formatv_Object_ಬಗೆ:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::Char_ಬಗೆ:
    OS << "char:\"" << Ptr.character << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ Twine::UHex_ಬಗೆ:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    ನಿಲ್ಲಿಸು;
  }
}

ಶೂನ್ಯ Twine::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  print_One_ಮಗು(OS, LHS, getLHSKind());
  print_One_ಮಗು(OS, RHS, getRHSKind());
}

ಶೂನ್ಯ Twine::printRepr(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OS << "(Twine ";
  print_One_ಮಗು_Repr(OS, LHS, getLHSKind());
  OS << " ";
  print_One_ಮಗು_Repr(OS, RHS, getRHSKind());
  OS << ")";
}

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ Twine::dump() ಸ್ಥಿರಾಂಕ { print(dbgs()); }

LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ Twine::dumpRepr() ಸ್ಥಿರಾಂಕ { printRepr(dbgs()); }
#ಆದರೆ_ಅಂತ್ಯ
