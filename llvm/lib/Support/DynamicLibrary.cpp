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
वग्ग DynamicLibrary::HandleSet {
  using HandleList = std::vector<शून्य *>;
  HandleList Handles;
  शून्य *Process = &Invalid;

सार्वजनिक:
  स्थिर शून्य *DLOpen(ध्रुव वर्ण *Filename, std::string *Err);
  स्थिर शून्य DLClose(शून्य *Handle);
  स्थिर शून्य *DLSym(शून्य *Handle, ध्रुव वर्ण *चिह्नम्);

  HandleSet() = स्वभाव;
  ~HandleSet();

  HandleList::iterator Find(शून्य *Handle) { पटिनिवत्त find(Handles, Handle); }

  तार्किक Contains(शून्य *Handle) {
    पटिनिवत्त Handle == Process || Find(Handle) != Handles.end();
  }

  तार्किक AddLibrary(शून्य *Handle, तार्किक IsProcess = मिच्छा, तार्किक CanClose = सच्च,
                  तार्किक AllowDuplicates = मिच्छा) {
#ifdef _WIN32
    प्रतिज्ञा((Handle == एतत् ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    प्रतिज्ञा((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    सचे (LLVM_LIKELY(!IsProcess)) {
      सचे (!AllowDuplicates && Find(Handle) != Handles.end()) {
        सचे (CanClose)
          DLClose(Handle);
        पटिनिवत्त मिच्छा;
      }
      Handles.प्रक्षिप_back(Handle);
    } अञ्ञथा {
#ifndef _WIN32
      सचे (Process != &Invalid) {
        सचे (CanClose)
          DLClose(Process);
        सचे (Process == Handle)
          पटिनिवत्त मिच्छा;
      }
#endif
      Process = Handle;
    }
    पटिनिवत्त सच्च;
  }

  शून्य CloseLibrary(शून्य *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    सचे (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  शून्य *LibLookup(ध्रुव वर्ण *चिह्नम्, DynamicLibrary::अन्वेषणम्_Ordering Order) {
    सचे (Order & SO_LoadOrder) {
      अत्थाय (शून्य *Handle : Handles) {
        सचे (शून्य *Ptr = DLSym(Handle, चिह्नम्))
          पटिनिवत्त Ptr;
      }
    } अञ्ञथा {
      अत्थाय (शून्य *Handle : llvm::reverse(Handles)) {
        सचे (शून्य *Ptr = DLSym(Handle, चिह्नम्))
          पटिनिवत्त Ptr;
      }
    }
    पटिनिवत्त nullptr;
  }

  शून्य *Lookup(ध्रुव वर्ण *चिह्नम्, DynamicLibrary::अन्वेषणम्_Ordering Order) {
    प्रतिज्ञा(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    सचे (Process == &Invalid || (Order & SO_LoadedFirst)) {
      सचे (शून्य *Ptr = LibLookup(चिह्नम्, Order))
        पटिनिवत्त Ptr;
    }
    सचे (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      सचे (शून्य *Ptr = DLSym(Process, चिह्नम्))
        पटिनिवत्त Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      सचे (Order & SO_LoadedLast) {
        सचे (शून्य *Ptr = LibLookup(चिह्नम्, Order))
          पटिनिवत्त Ptr;
      }
    }
    पटिनिवत्त nullptr;
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
  पटिनिवत्त G;
}

} // namespace

#ifdef _WIN32

#अन्तर्भाव "Windows/DynamicLibrary.inc"

#अञ्ञथा

#अन्तर्भाव "Unix/DynamicLibrary.inc"

#endif

वर्ण DynamicLibrary::Invalid;
DynamicLibrary::अन्वेषणम्_Ordering DynamicLibrary::अन्वेषणम्_Order =
    DynamicLibrary::SO_Linker;

नामाकाश llvm {
शून्य *अन्वेषणम्_For_Address_Of_Special_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name) {
  पटिनिवत्त Do_अन्वेषणम्(चिह्नम्_Name); // DynamicLibrary.inc
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
  सचे (Handle != &Invalid) {
    SmartScopedLock<सच्च> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  पटिनिवत्त DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(शून्य *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<सच्च> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  सचे (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ मिच्छा,
                                  /*CanClose*/ मिच्छा))
    *Err = "Library already loaded";

  पटिनिवत्त DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ध्रुव वर्ण *FileName,
                                          std::string *Err) {
  प्रतिज्ञा(FileName && "Use getPermanentLibrary() for opening process handle");
  शून्य *Handle = HandleSet::DLOpen(FileName, Err);
  सचे (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<सच्च> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ मिच्छा,
                                        /*CanClose*/ मिच्छा,
                                        /*AllowDuplicates*/ सच्च);
  }
  पटिनिवत्त DynamicLibrary(Handle);
}

शून्य DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<सच्च> Lock(G.SymbolsMutex);
  सचे (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

शून्य *DynamicLibrary::get_Address_Of_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name) {
  सचे (!isValid())
    पटिनिवत्त nullptr;
  पटिनिवत्त HandleSet::DLSym(Data, चिह्नम्_Name);
}

शून्य *DynamicLibrary::अन्वेषणम्_For_Address_Of_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<सच्च> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<शून्य *>::iterator i = G.ExplicitSymbols.find(चिह्नम्_Name);

    सचे (i != G.ExplicitSymbols.end())
      पटिनिवत्त i->second;

    // Now search the libraries.
    सचे (शून्य *Ptr = G.OpenedHandles.Lookup(चिह्नम्_Name, अन्वेषणम्_Order))
      पटिनिवत्त Ptr;
    सचे (शून्य *Ptr = G.OpenedTemporaryHandles.Lookup(चिह्नम्_Name, अन्वेषणम्_Order))
      पटिनिवत्त Ptr;
  }

  पटिनिवत्त llvm::अन्वेषणम्_For_Address_Of_Special_चिह्नम्(चिह्नम्_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ध्रुव वर्ण *Filename) {
  पटिनिवत्त llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

शून्य *LLVMSearch_For_Address_Of_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name_2) {
  पटिनिवत्त llvm::sys::DynamicLibrary::अन्वेषणम्_For_Address_Of_चिह्नम्(चिह्नम्_Name_2);
}

शून्य LLVMAdd_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name_2, शून्य *चिह्नम्_मानम्_2) {
  पटिनिवत्त llvm::sys::DynamicLibrary::Add_चिह्नम्(चिह्नम्_Name_2, चिह्नम्_मानम्_2);
}
