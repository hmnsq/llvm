//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/Twine.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/FormatVariadic.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
ఉపయోగించి నామాకాశ llvm;

std::string Twine::str() స్థిరరాశి {
  // If we're storing only a std::string, just return it.
  ఒకవేళ (LHSKind == Std_String_రకం && RHSKind == ఖాళీలు_రకం)
    తిరిగి *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  ఒకవేళ (LHSKind == Formatv_Object_రకం && RHSKind == ఖాళీలు_రకం)
    తిరిగి LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  తిరిగి గమ్యం_String_Ref(Vec).str();
}

శూన్య Twine::గమ్యం_సదిశ(Small_సదిశ_Impl<అక్షరం> &బహిర్గతం) స్థిరరాశి {
  raw_svector_ostream OS(బహిర్గతం);
  print(OS);
}

StringRef Twine::గమ్యం_Null_Terminated_String_Ref(Small_సదిశ_Impl<అక్షరం> &బహిర్గతం) స్థిరరాశి {
  ఒకవేళ (isUnary()) {
    ఎంపిక (getLHSKind()) {
    స్థితి CString_రకం:
      // Already null terminated, yay!
      తిరిగి StringRef(LHS.cString);
    స్థితి Std_String_రకం: {
      స్థిరరాశి std::string *str = LHS.stdString;
      తిరిగి StringRef(str->c_str(), str->కొలత_2());
    }
    స్థితి String_Literal_రకం:
      తిరిగి StringRef(LHS.ptr_And_నిడివి.ptr, LHS.ptr_And_నిడివి.నిడివి_2);
    అప్రమేయ:
      ఆపు;
    }
  }
  గమ్యం_సదిశ(బహిర్గతం);
  బహిర్గతం.నెట్టు_వెనుక(0);
  బహిర్గతం.తీయి_వెనుక();
  తిరిగి StringRef(బహిర్గతం.దత్తాంశం(), బహిర్గతం.కొలత_2());
}

శూన్య Twine::print_One_బిడ్డ(raw_ostream &OS, బిడ్డ Ptr, కణుపు_రకం రకం_3) స్థిరరాశి {
  ఎంపిక (రకం_3) {
  స్థితి Twine::Null_రకం:
    ఆపు;
  స్థితి Twine::ఖాళీలు_రకం:
    ఆపు;
  స్థితి Twine::Twine_రకం:
    Ptr.twine->print(OS);
    ఆపు;
  స్థితి Twine::CString_రకం:
    OS << Ptr.cString;
    ఆపు;
  స్థితి Twine::Std_String_రకం:
    OS << *Ptr.stdString;
    ఆపు;
  స్థితి Twine::Ptr_And_నిడివి_రకం:
  స్థితి Twine::String_Literal_రకం:
    OS << StringRef(Ptr.ptr_And_నిడివి.ptr, Ptr.ptr_And_నిడివి.నిడివి_2);
    ఆపు;
  స్థితి Twine::Formatv_Object_రకం:
    OS << *Ptr.formatvObject;
    ఆపు;
  స్థితి Twine::Char_రకం:
    OS << Ptr.character;
    ఆపు;
  స్థితి Twine::DecUIKind:
    OS << Ptr.decUI;
    ఆపు;
  స్థితి Twine::DecIKind:
    OS << Ptr.decI;
    ఆపు;
  స్థితి Twine::DecULKind:
    OS << Ptr.decUL;
    ఆపు;
  స్థితి Twine::DecLKind:
    OS << Ptr.decL;
    ఆపు;
  స్థితి Twine::DecULLKind:
    OS << Ptr.decULL;
    ఆపు;
  స్థితి Twine::DecLLKind:
    OS << Ptr.decLL;
    ఆపు;
  స్థితి Twine::UHex_రకం:
    OS.write_hex(Ptr.uHex);
    ఆపు;
  }
}

శూన్య Twine::print_One_బిడ్డ_Repr(raw_ostream &OS, బిడ్డ Ptr, కణుపు_రకం రకం_3) స్థిరరాశి {
  ఎంపిక (రకం_3) {
  స్థితి Twine::Null_రకం:
    OS << "null";
    ఆపు;
  స్థితి Twine::ఖాళీలు_రకం:
    OS << "empty";
    ఆపు;
  స్థితి Twine::Twine_రకం:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    ఆపు;
  స్థితి Twine::CString_రకం:
    OS << "cstring:\"" << Ptr.cString << "\"";
    ఆపు;
  స్థితి Twine::Std_String_రకం:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    ఆపు;
  స్థితి Twine::Ptr_And_నిడివి_రకం:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptr_And_నిడివి.ptr, Ptr.ptr_And_నిడివి.నిడివి_2) << "\"";
    ఆపు;
  స్థితి Twine::String_Literal_రకం:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptr_And_నిడివి.ptr, Ptr.ptr_And_నిడివి.నిడివి_2) << "\"";
    ఆపు;
  స్థితి Twine::Formatv_Object_రకం:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    ఆపు;
  స్థితి Twine::Char_రకం:
    OS << "char:\"" << Ptr.character << "\"";
    ఆపు;
  స్థితి Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    ఆపు;
  స్థితి Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    ఆపు;
  స్థితి Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    ఆపు;
  స్థితి Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    ఆపు;
  స్థితి Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    ఆపు;
  స్థితి Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    ఆపు;
  స్థితి Twine::UHex_రకం:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    ఆపు;
  }
}

శూన్య Twine::print(raw_ostream &OS) స్థిరరాశి {
  print_One_బిడ్డ(OS, LHS, getLHSKind());
  print_One_బిడ్డ(OS, RHS, getRHSKind());
}

శూన్య Twine::printRepr(raw_ostream &OS) స్థిరరాశి {
  OS << "(Twine ";
  print_One_బిడ్డ_Repr(OS, LHS, getLHSKind());
  OS << " ";
  print_One_బిడ్డ_Repr(OS, RHS, getRHSKind());
  OS << ")";
}

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_పద్ధతి శూన్య Twine::dump() స్థిరరాశి { print(dbgs()); }

LLVM_DUMP_పద్ధతి శూన్య Twine::dumpRepr() స్థిరరాశి { printRepr(dbgs()); }
#అయితే_ముగింపు
