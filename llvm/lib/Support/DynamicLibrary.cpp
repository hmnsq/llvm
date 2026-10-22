//===-- DynamicLibrary.cpp - Runtime link/load libraries --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the operating system DynamicLibrary concept.
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/DynamicLibrary.h"
#अन्तर्भाव "llvm-c/Support.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Mutex.h"
#अन्तर्भाव <vector>

using नामाकाश llvm;
using नामाकाश llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
वर्ग DynamicLibrary::HandleSet {
  using HandleList = std::vector<शून्य *>;
  HandleList Handles;
  शून्य *Process = &Invalid;

सार्वजनिक:
  स्थिर शून्य *DLOpen(ध्रुव वर्ण *Filename, std::string *Err);
  स्थिर शून्य DLClose(शून्य *Handle);
  स्थिर शून्य *DLSym(शून्य *Handle, ध्रुव वर्ण *चिह्नम्);

  HandleSet() = स्वभाव;
  ~HandleSet();

  HandleList::iterator Find(शून्य *Handle) { प्रत्यागम find(Handles, Handle); }

  तार्किक Contains(शून्य *Handle) {
    प्रत्यागम Handle == Process || Find(Handle) != Handles.end();
  }

  तार्किक AddLibrary(शून्य *Handle, तार्किक IsProcess = मिच्छ, तार्किक CanClose = सच्च,
                  तार्किक AllowDuplicates = मिच्छ) {
#ifdef _WIN32
    प्रतिज्ञा((Handle == एतत् ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    प्रतिज्ञा((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    जइ (LLVM_LIKELY(!IsProcess)) {
      जइ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        जइ (CanClose)
          DLClose(Handle);
        प्रत्यागम मिच्छ;
      }
      Handles.प्रक्षिप_back(Handle);
    } अण्णहा {
#ifndef _WIN32
      जइ (Process != &Invalid) {
        जइ (CanClose)
          DLClose(Process);
        जइ (Process == Handle)
          प्रत्यागम मिच्छ;
      }
#endif
      Process = Handle;
    }
    प्रत्यागम सच्च;
  }

  शून्य CloseLibrary(शून्य *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    जइ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  शून्य *LibLookup(ध्रुव वर्ण *चिह्नम्, DynamicLibrary::अन्वेषणम्_Ordering Order) {
    जइ (Order & SO_LoadOrder) {
      प्रति (शून्य *Handle : Handles) {
        जइ (शून्य *Ptr = DLSym(Handle, चिह्नम्))
          प्रत्यागम Ptr;
      }
    } अण्णहा {
      प्रति (शून्य *Handle : llvm::reverse(Handles)) {
        जइ (शून्य *Ptr = DLSym(Handle, चिह्नम्))
          प्रत्यागम Ptr;
      }
    }
    प्रत्यागम nullptr;
  }

  शून्य *Lookup(ध्रुव वर्ण *चिह्नम्, DynamicLibrary::अन्वेषणम्_Ordering Order) {
    प्रतिज्ञा(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    जइ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      जइ (शून्य *Ptr = LibLookup(चिह्नम्, Order))
        प्रत्यागम Ptr;
    }
    जइ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      जइ (शून्य *Ptr = DLSym(Process, चिह्नम्))
        प्रत्यागम Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      जइ (Order & SO_LoadedLast) {
        जइ (शून्य *Ptr = LibLookup(चिह्नम्, Order))
          प्रत्यागम Ptr;
      }
    }
    प्रत्यागम nullptr;
  }
};

नामाकाश {

संरचना Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<शून्य *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<सच्च> SymbolsMutex;
};

Globals &getGlobals() {
  स्थिर Globals G;
  प्रत्यागम G;
}

} // namespace

#ifdef _WIN32

#अन्तर्भाव "Windows/DynamicLibrary.inc"

#अण्णहा

#अन्तर्भाव "Unix/DynamicLibrary.inc"

#endif

वर्ण DynamicLibrary::Invalid;
DynamicLibrary::अन्वेषणम्_Ordering DynamicLibrary::अन्वेषणम्_Order =
    DynamicLibrary::SO_Linker;

नामाकाश llvm {
शून्य *अन्वेषणम्_For_Address_Of_Special_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name) {
  प्रत्यागम Do_अन्वेषणम्(चिह्नम्_Name); // DynamicLibrary.inc
}
} // namespace llvm

शून्य DynamicLibrary::Add_चिह्नम्(StringRef चिह्नम्_Name, शून्य *चिह्नम्_मानम्) {
  auto &G = getGlobals();
  SmartScopedLock<सच्च> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[चिह्नम्_Name] = चिह्नम्_मानम्;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ध्रुव वर्ण *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  शून्य *Handle = HandleSet::DLOpen(FileName, Err);
  जइ (Handle != &Invalid) {
    SmartScopedLock<सच्च> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  प्रत्यागम DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(शून्य *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<सच्च> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  जइ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ मिच्छ,
                                  /*CanClose*/ मिच्छ))
    *Err = "Library already loaded";

  प्रत्यागम DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ध्रुव वर्ण *FileName,
                                          std::string *Err) {
  प्रतिज्ञा(FileName && "Use getPermanentLibrary() for opening process handle");
  शून्य *Handle = HandleSet::DLOpen(FileName, Err);
  जइ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<सच्च> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ मिच्छ,
                                        /*CanClose*/ मिच्छ,
                                        /*AllowDuplicates*/ सच्च);
  }
  प्रत्यागम DynamicLibrary(Handle);
}

शून्य DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<सच्च> Lock(G.SymbolsMutex);
  जइ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

शून्य *DynamicLibrary::get_Address_Of_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name) {
  जइ (!isValid())
    प्रत्यागम nullptr;
  प्रत्यागम HandleSet::DLSym(Data, चिह्नम्_Name);
}

शून्य *DynamicLibrary::अन्वेषणम्_For_Address_Of_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<सच्च> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<शून्य *>::iterator i = G.ExplicitSymbols.find(चिह्नम्_Name);

    जइ (i != G.ExplicitSymbols.end())
      प्रत्यागम i->second;

    // Now search the libraries.
    जइ (शून्य *Ptr = G.OpenedHandles.Lookup(चिह्नम्_Name, अन्वेषणम्_Order))
      प्रत्यागम Ptr;
    जइ (शून्य *Ptr = G.OpenedTemporaryHandles.Lookup(चिह्नम्_Name, अन्वेषणम्_Order))
      प्रत्यागम Ptr;
  }

  प्रत्यागम llvm::अन्वेषणम्_For_Address_Of_Special_चिह्नम्(चिह्नम्_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ध्रुव वर्ण *Filename) {
  प्रत्यागम llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

शून्य *LLVMSearch_For_Address_Of_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name_2) {
  प्रत्यागम llvm::sys::DynamicLibrary::अन्वेषणम्_For_Address_Of_चिह्नम्(चिह्नम्_Name_2);
}

शून्य LLVMAdd_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name_2, शून्य *चिह्नम्_मानम्_2) {
  प्रत्यागम llvm::sys::DynamicLibrary::Add_चिह्नम्(चिह्नम्_Name_2, चिह्नम्_मानम्_2);
}
