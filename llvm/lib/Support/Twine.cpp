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
उपयुज्य नामाकाश llvm;

std::string Twine::str() ध्रुव {
  // If we're storing only a std::string, just return it.
  यदि (LHSKind == Std_String_विधा && RHSKind == रिक्तानि_विधा)
    प्रत्यागम *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  यदि (LHSKind == Formatv_Object_विधा && RHSKind == रिक्तानि_विधा)
    प्रत्यागम LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  प्रत्यागम गन्तव्यम्_String_Ref(Vec).str();
}

शून्य Twine::गन्तव्यम्_सदिशम्(Small_सदिशम्_Impl<वर्ण> &बहिर्गमनम्) ध्रुव {
  raw_svector_ostream OS(बहिर्गमनम्);
  print(OS);
}

StringRef Twine::गन्तव्यम्_Null_Terminated_String_Ref(Small_सदिशम्_Impl<वर्ण> &बहिर्गमनम्) ध्रुव {
  यदि (isUnary()) {
    विकल्प (getLHSKind()) {
    पक्ष CString_विधा:
      // Already null terminated, yay!
      प्रत्यागम StringRef(LHS.cString);
    पक्ष Std_String_विधा: {
      ध्रुव std::string *str = LHS.stdString;
      प्रत्यागम StringRef(str->c_str(), str->आकार_2());
    }
    पक्ष String_Literal_विधा:
      प्रत्यागम StringRef(LHS.ptr_And_दीर्घता.ptr, LHS.ptr_And_दीर्घता.दीर्घता_2);
    स्वभाव:
      भङ्ग;
    }
  }
  गन्तव्यम्_सदिशम्(बहिर्गमनम्);
  बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(0);
  बहिर्गमनम्.निष्कासय_पृष्ठभागः();
  प्रत्यागम StringRef(बहिर्गमनम्.दत्त(), बहिर्गमनम्.आकार_2());
}

शून्य Twine::printOneChild(raw_ostream &OS, Child Ptr, ग्रन्थिः_विधा विधा_2) ध्रुव {
  विकल्प (विधा_2) {
  पक्ष Twine::Null_विधा:
    भङ्ग;
  पक्ष Twine::रिक्तानि_विधा:
    भङ्ग;
  पक्ष Twine::Twine_विधा:
    Ptr.twine->print(OS);
    भङ्ग;
  पक्ष Twine::CString_विधा:
    OS << Ptr.cString;
    भङ्ग;
  पक्ष Twine::Std_String_विधा:
    OS << *Ptr.stdString;
    भङ्ग;
  पक्ष Twine::Ptr_And_दीर्घता_विधा:
  पक्ष Twine::String_Literal_विधा:
    OS << StringRef(Ptr.ptr_And_दीर्घता.ptr, Ptr.ptr_And_दीर्घता.दीर्घता_2);
    भङ्ग;
  पक्ष Twine::Formatv_Object_विधा:
    OS << *Ptr.formatvObject;
    भङ्ग;
  पक्ष Twine::Char_विधा:
    OS << Ptr.character;
    भङ्ग;
  पक्ष Twine::DecUIKind:
    OS << Ptr.decUI;
    भङ्ग;
  पक्ष Twine::DecIKind:
    OS << Ptr.decI;
    भङ्ग;
  पक्ष Twine::DecULKind:
    OS << Ptr.decUL;
    भङ्ग;
  पक्ष Twine::DecLKind:
    OS << Ptr.decL;
    भङ्ग;
  पक्ष Twine::DecULLKind:
    OS << Ptr.decULL;
    भङ्ग;
  पक्ष Twine::DecLLKind:
    OS << Ptr.decLL;
    भङ्ग;
  पक्ष Twine::UHex_विधा:
    OS.write_hex(Ptr.uHex);
    भङ्ग;
  }
}

शून्य Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, ग्रन्थिः_विधा विधा_2) ध्रुव {
  विकल्प (विधा_2) {
  पक्ष Twine::Null_विधा:
    OS << "null";
    भङ्ग;
  पक्ष Twine::रिक्तानि_विधा:
    OS << "empty";
    भङ्ग;
  पक्ष Twine::Twine_विधा:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    भङ्ग;
  पक्ष Twine::CString_विधा:
    OS << "cstring:\"" << Ptr.cString << "\"";
    भङ्ग;
  पक्ष Twine::Std_String_विधा:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    भङ्ग;
  पक्ष Twine::Ptr_And_दीर्घता_विधा:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_दीर्घता.ptr, Ptr.ptr_And_दीर्घता.दीर्घता_2) << "\"";
    भङ्ग;
  पक्ष Twine::String_Literal_विधा:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_दीर्घता.ptr, Ptr.ptr_And_दीर्घता.दीर्घता_2) << "\"";
    भङ्ग;
  पक्ष Twine::Formatv_Object_विधा:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    भङ्ग;
  पक्ष Twine::Char_विधा:
    OS << "char:\"" << Ptr.character << "\"";
    भङ्ग;
  पक्ष Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    भङ्ग;
  पक्ष Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    भङ्ग;
  पक्ष Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    भङ्ग;
  पक्ष Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    भङ्ग;
  पक्ष Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    भङ्ग;
  पक्ष Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    भङ्ग;
  पक्ष Twine::UHex_विधा:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    भङ्ग;
  }
}

शून्य Twine::print(raw_ostream &OS) ध्रुव {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

शून्य Twine::printRepr(raw_ostream &OS) ध्रुव {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_विधानम् शून्य Twine::dump() ध्रुव { print(dbgs()); }

LLVM_DUMP_विधानम् शून्य Twine::dumpRepr() ध्रुव { printRepr(dbgs()); }
#यदि_अन्तः
