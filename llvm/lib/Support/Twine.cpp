//===-- Twine.cpp - Fast Temporary String Concatenation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/ADT/Twine.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/FormatVariadic.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
using நாமாகாஶ llvm;

std::string Twine::str() மாறிலி {
  // If we're storing only a std::string, just return it.
  எனில் (LHSKind == StdStringKind && RHSKind == EmptyKind)
    திரும்பு *LHS.stdString;

  // If we're storing a formatv_object, we can avoid an extra copy by formatting
  // it immediately and returning the result.
  எனில் (LHSKind == FormatvObjectKind && RHSKind == EmptyKind)
    திரும்பு LHS.formatvObject->str();

  // Otherwise, flatten and copy the contents first.
  SmallString<256> Vec;
  திரும்பு toStringRef(Vec).str();
}

வெறுமை Twine::toVector(SmallVectorImpl<எழுத்து> &Out) மாறிலி {
  raw_svector_ostream OS(Out);
  print(OS);
}

StringRef Twine::toNullTerminatedStringRef(SmallVectorImpl<எழுத்து> &Out) மாறிலி {
  எனில் (isUnary()) {
    தேர்வு (getLHSKind()) {
    நிலை CStringKind:
      // Already null terminated, yay!
      திரும்பு StringRef(LHS.cString);
    நிலை StdStringKind: {
      மாறிலி std::string *str = LHS.stdString;
      திரும்பு StringRef(str->c_str(), str->size());
    }
    நிலை StringLiteralKind:
      திரும்பு StringRef(LHS.ptrAndLength.ptr, LHS.ptrAndLength.length);
    இயல்பு:
      நிறுத்து;
    }
  }
  toVector(Out);
  Out.தள்ளு_back(0);
  Out.எடு_back();
  திரும்பு StringRef(Out.data(), Out.size());
}

வெறுமை Twine::printOneChild(raw_ostream &OS, Child Ptr, முனை_Kind Kind) மாறிலி {
  தேர்வு (Kind) {
  நிலை Twine::NullKind:
    நிறுத்து;
  நிலை Twine::EmptyKind:
    நிறுத்து;
  நிலை Twine::TwineKind:
    Ptr.twine->print(OS);
    நிறுத்து;
  நிலை Twine::CStringKind:
    OS << Ptr.cString;
    நிறுத்து;
  நிலை Twine::StdStringKind:
    OS << *Ptr.stdString;
    நிறுத்து;
  நிலை Twine::PtrAndLengthKind:
  நிலை Twine::StringLiteralKind:
    OS << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length);
    நிறுத்து;
  நிலை Twine::FormatvObjectKind:
    OS << *Ptr.formatvObject;
    நிறுத்து;
  நிலை Twine::CharKind:
    OS << Ptr.character;
    நிறுத்து;
  நிலை Twine::DecUIKind:
    OS << Ptr.decUI;
    நிறுத்து;
  நிலை Twine::DecIKind:
    OS << Ptr.decI;
    நிறுத்து;
  நிலை Twine::DecULKind:
    OS << Ptr.decUL;
    நிறுத்து;
  நிலை Twine::DecLKind:
    OS << Ptr.decL;
    நிறுத்து;
  நிலை Twine::DecULLKind:
    OS << Ptr.decULL;
    நிறுத்து;
  நிலை Twine::DecLLKind:
    OS << Ptr.decLL;
    நிறுத்து;
  நிலை Twine::UHexKind:
    OS.write_hex(Ptr.uHex);
    நிறுத்து;
  }
}

வெறுமை Twine::printOneChildRepr(raw_ostream &OS, Child Ptr, முனை_Kind Kind) மாறிலி {
  தேர்வு (Kind) {
  நிலை Twine::NullKind:
    OS << "null";
    நிறுத்து;
  நிலை Twine::EmptyKind:
    OS << "empty";
    நிறுத்து;
  நிலை Twine::TwineKind:
    OS << "rope:";
    Ptr.twine->printRepr(OS);
    நிறுத்து;
  நிலை Twine::CStringKind:
    OS << "cstring:\"" << Ptr.cString << "\"";
    நிறுத்து;
  நிலை Twine::StdStringKind:
    OS << "std::string:\"" << Ptr.stdString << "\"";
    நிறுத்து;
  நிலை Twine::PtrAndLengthKind:
    OS << "ptrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    நிறுத்து;
  நிலை Twine::StringLiteralKind:
    OS << "constexprPtrAndLength:\""
       << StringRef(Ptr.ptrAndLength.ptr, Ptr.ptrAndLength.length) << "\"";
    நிறுத்து;
  நிலை Twine::FormatvObjectKind:
    OS << "formatv:\"" << *Ptr.formatvObject << "\"";
    நிறுத்து;
  நிலை Twine::CharKind:
    OS << "char:\"" << Ptr.character << "\"";
    நிறுத்து;
  நிலை Twine::DecUIKind:
    OS << "decUI:\"" << Ptr.decUI << "\"";
    நிறுத்து;
  நிலை Twine::DecIKind:
    OS << "decI:\"" << Ptr.decI << "\"";
    நிறுத்து;
  நிலை Twine::DecULKind:
    OS << "decUL:\"" << Ptr.decUL << "\"";
    நிறுத்து;
  நிலை Twine::DecLKind:
    OS << "decL:\"" << Ptr.decL << "\"";
    நிறுத்து;
  நிலை Twine::DecULLKind:
    OS << "decULL:\"" << Ptr.decULL << "\"";
    நிறுத்து;
  நிலை Twine::DecLLKind:
    OS << "decLL:\"" << Ptr.decLL << "\"";
    நிறுத்து;
  நிலை Twine::UHexKind:
    OS << "uhex:\"" << Ptr.uHex << "\"";
    நிறுத்து;
  }
}

வெறுமை Twine::print(raw_ostream &OS) மாறிலி {
  printOneChild(OS, LHS, getLHSKind());
  printOneChild(OS, RHS, getRHSKind());
}

வெறுமை Twine::printRepr(raw_ostream &OS) மாறிலி {
  OS << "(Twine ";
  printOneChildRepr(OS, LHS, getLHSKind());
  OS << " ";
  printOneChildRepr(OS, RHS, getRHSKind());
  OS << ")";
}

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை Twine::dump() மாறிலி { print(dbgs()); }

LLVM_DUMP_METHOD வெறுமை Twine::dumpRepr() மாறிலி { printRepr(dbgs()); }
#endif
