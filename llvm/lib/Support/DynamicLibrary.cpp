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
  स्थिर शून्य *DLOpen(स्थिरांक अक्षर *Filename, std::string *Err);
  स्थिर शून्य DLClose(शून्य *Handle);
  स्थिर शून्य *DLSym(शून्य *Handle, स्थिरांक अक्षर *चिन्ह);

  HandleSet() = पूर्वनिर्धारित;
  ~HandleSet();

  HandleList::iterator Find(शून्य *Handle) { परत find(Handles, Handle); }

  तार्किक Contains(शून्य *Handle) {
    परत Handle == Process || Find(Handle) != Handles.end();
  }

  तार्किक AddLibrary(शून्य *Handle, तार्किक IsProcess = असत्य, तार्किक CanClose = सत्य,
                  तार्किक AllowDuplicates = असत्य) {
#ifdef _WIN32
    प्रतिज्ञा((Handle == हे ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    प्रतिज्ञा((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    जर (LLVM_LIKELY(!IsProcess)) {
      जर (!AllowDuplicates && Find(Handle) != Handles.end()) {
        जर (CanClose)
          DLClose(Handle);
        परत असत्य;
      }
      Handles.ढकला_back(Handle);
    } अन्यथा {
#ifndef _WIN32
      जर (Process != &Invalid) {
        जर (CanClose)
          DLClose(Process);
        जर (Process == Handle)
          परत असत्य;
      }
#endif
      Process = Handle;
    }
    परत सत्य;
  }

  शून्य CloseLibrary(शून्य *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    जर (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  शून्य *LibLookup(स्थिरांक अक्षर *चिन्ह, DynamicLibrary::शोध_Ordering Order) {
    जर (Order & SO_LoadOrder) {
      साठी (शून्य *Handle : Handles) {
        जर (शून्य *Ptr = DLSym(Handle, चिन्ह))
          परत Ptr;
      }
    } अन्यथा {
      साठी (शून्य *Handle : llvm::reverse(Handles)) {
        जर (शून्य *Ptr = DLSym(Handle, चिन्ह))
          परत Ptr;
      }
    }
    परत nullptr;
  }

  शून्य *Lookup(स्थिरांक अक्षर *चिन्ह, DynamicLibrary::शोध_Ordering Order) {
    प्रतिज्ञा(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    जर (Process == &Invalid || (Order & SO_LoadedFirst)) {
      जर (शून्य *Ptr = LibLookup(चिन्ह, Order))
        परत Ptr;
    }
    जर (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      जर (शून्य *Ptr = DLSym(Process, चिन्ह))
        परत Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      जर (Order & SO_LoadedLast) {
        जर (शून्य *Ptr = LibLookup(चिन्ह, Order))
          परत Ptr;
      }
    }
    परत nullptr;
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
  llvm::sys::SmartMutex<सत्य> SymbolsMutex;
};

Globals &getGlobals() {
  स्थिर Globals G;
  परत G;
}

} // namespace

#ifdef _WIN32

#अन्तर्भाव "Windows/DynamicLibrary.inc"

#अन्यथा

#अन्तर्भाव "Unix/DynamicLibrary.inc"

#endif

अक्षर DynamicLibrary::Invalid;
DynamicLibrary::शोध_Ordering DynamicLibrary::शोध_Order =
    DynamicLibrary::SO_Linker;

नामाकाश llvm {
शून्य *शोध_For_Address_Of_Special_चिन्ह(स्थिरांक अक्षर *चिन्ह_Name) {
  परत Do_शोध(चिन्ह_Name); // DynamicLibrary.inc
}
} // namespace llvm

शून्य DynamicLibrary::Add_चिन्ह(StringRef चिन्ह_Name, शून्य *चिन्ह_मूल्य) {
  स्वतः &G = getGlobals();
  SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[चिन्ह_Name] = चिन्ह_मूल्य;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(स्थिरांक अक्षर *FileName,
                                                   std::string *Err) {
  स्वतः &G = getGlobals();
  शून्य *Handle = HandleSet::DLOpen(FileName, Err);
  जर (Handle != &Invalid) {
    SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  परत DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(शून्य *Handle,
                                                   std::string *Err) {
  स्वतः &G = getGlobals();
  SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  जर (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ असत्य,
                                  /*CanClose*/ असत्य))
    *Err = "Library already loaded";

  परत DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(स्थिरांक अक्षर *FileName,
                                          std::string *Err) {
  प्रतिज्ञा(FileName && "Use getPermanentLibrary() for opening process handle");
  शून्य *Handle = HandleSet::DLOpen(FileName, Err);
  जर (Handle != &Invalid) {
    स्वतः &G = getGlobals();
    SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ असत्य,
                                        /*CanClose*/ असत्य,
                                        /*AllowDuplicates*/ सत्य);
  }
  परत DynamicLibrary(Handle);
}

शून्य DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  स्वतः &G = getGlobals();
  SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
  जर (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

शून्य *DynamicLibrary::get_Address_Of_चिन्ह(स्थिरांक अक्षर *चिन्ह_Name) {
  जर (!isValid())
    परत nullptr;
  परत HandleSet::DLSym(Data, चिन्ह_Name);
}

शून्य *DynamicLibrary::शोध_For_Address_Of_चिन्ह(स्थिरांक अक्षर *चिन्ह_Name) {
  {
    स्वतः &G = getGlobals();
    SmartScopedLock<सत्य> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<शून्य *>::iterator i = G.ExplicitSymbols.find(चिन्ह_Name);

    जर (i != G.ExplicitSymbols.end())
      परत i->second;

    // Now search the libraries.
    जर (शून्य *Ptr = G.OpenedHandles.Lookup(चिन्ह_Name, शोध_Order))
      परत Ptr;
    जर (शून्य *Ptr = G.OpenedTemporaryHandles.Lookup(चिन्ह_Name, शोध_Order))
      परत Ptr;
  }

  परत llvm::शोध_For_Address_Of_Special_चिन्ह(चिन्ह_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(स्थिरांक अक्षर *Filename) {
  परत llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

शून्य *LLVMSearch_For_Address_Of_चिन्ह(स्थिरांक अक्षर *चिन्ह_Name_2) {
  परत llvm::sys::DynamicLibrary::शोध_For_Address_Of_चिन्ह(चिन्ह_Name_2);
}

शून्य LLVMAdd_चिन्ह(स्थिरांक अक्षर *चिन्ह_Name_2, शून्य *चिन्ह_मूल्य_2) {
  परत llvm::sys::DynamicLibrary::Add_चिन्ह(चिन्ह_Name_2, चिन्ह_मूल्य_2);
}
