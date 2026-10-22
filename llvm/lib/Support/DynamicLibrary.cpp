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

#समावेश "llvm/Support/DynamicLibrary.h"
#समावेश "llvm-c/Support.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/StringMap.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Mutex.h"
#समावेश <vector>

using namespace llvm;
using namespace llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
श्रेणी DynamicLibrary::HandleSet {
  using HandleList = std::vector<व्योम *>;
  HandleList Handles;
  व्योम *Process = &Invalid;

खुला:
  अटल व्योम *DLOpen(स्थिर अक्षर *Filename, std::string *Err);
  अटल व्योम DLClose(व्योम *Handle);
  अटल व्योम *DLSym(व्योम *Handle, स्थिर अक्षर *प्रतीक);

  HandleSet() = शेष;
  ~HandleSet();

  HandleList::iterator Find(व्योम *Handle) { वापस find(Handles, Handle); }

  bool Contains(व्योम *Handle) {
    वापस Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(व्योम *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#अगर_घोषित _WIN32
    निश्चित((Handle == यह ? IsProcess : !IsProcess) && "Bad Handle.");
#पूर्ण_अगर
    निश्चित((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    अगर (LLVM_LIKELY(!IsProcess)) {
      अगर (!AllowDuplicates && Find(Handle) != Handles.end()) {
        अगर (CanClose)
          DLClose(Handle);
        वापस false;
      }
      Handles.धकेलो_back(Handle);
    } अन्यथा {
#अगर_अघोषित _WIN32
      अगर (Process != &Invalid) {
        अगर (CanClose)
          DLClose(Process);
        अगर (Process == Handle)
          वापस false;
      }
#पूर्ण_अगर
      Process = Handle;
    }
    वापस true;
  }

  व्योम CloseLibrary(व्योम *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    अगर (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  व्योम *LibLookup(स्थिर अक्षर *प्रतीक, DynamicLibrary::खोज_Ordering Order) {
    अगर (Order & SO_LoadOrder) {
      क्रम (व्योम *Handle : Handles) {
        अगर (व्योम *Ptr = DLSym(Handle, प्रतीक))
          वापस Ptr;
      }
    } अन्यथा {
      क्रम (व्योम *Handle : llvm::reverse(Handles)) {
        अगर (व्योम *Ptr = DLSym(Handle, प्रतीक))
          वापस Ptr;
      }
    }
    वापस nullptr;
  }

  व्योम *Lookup(स्थिर अक्षर *प्रतीक, DynamicLibrary::खोज_Ordering Order) {
    निश्चित(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    अगर (Process == &Invalid || (Order & SO_LoadedFirst)) {
      अगर (व्योम *Ptr = LibLookup(प्रतीक, Order))
        वापस Ptr;
    }
    अगर (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      अगर (व्योम *Ptr = DLSym(Process, प्रतीक))
        वापस Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      अगर (Order & SO_LoadedLast) {
        अगर (व्योम *Ptr = LibLookup(प्रतीक, Order))
          वापस Ptr;
      }
    }
    वापस nullptr;
  }
};

namespace {

काष्ठा Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<व्योम *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<true> SymbolsMutex;
};

Globals &getGlobals() {
  अटल Globals G;
  वापस G;
}

} // namespace

#अगर_घोषित _WIN32

#समावेश "Windows/DynamicLibrary.inc"

#अन्यथा

#समावेश "Unix/DynamicLibrary.inc"

#पूर्ण_अगर

अक्षर DynamicLibrary::Invalid;
DynamicLibrary::खोज_Ordering DynamicLibrary::खोज_Order =
    DynamicLibrary::SO_Linker;

namespace llvm {
व्योम *खोज_For_Address_Of_Special_प्रतीक(स्थिर अक्षर *प्रतीक_Name) {
  वापस Do_खोज(प्रतीक_Name); // DynamicLibrary.inc
}
} // namespace llvm

व्योम DynamicLibrary::Add_प्रतीक(StringRef प्रतीक_Name, व्योम *प्रतीक_मूल्य) {
  स्वतः &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[प्रतीक_Name] = प्रतीक_मूल्य;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(स्थिर अक्षर *FileName,
                                                   std::string *Err) {
  स्वतः &G = getGlobals();
  व्योम *Handle = HandleSet::DLOpen(FileName, Err);
  अगर (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  वापस DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(व्योम *Handle,
                                                   std::string *Err) {
  स्वतः &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  अगर (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  वापस DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(स्थिर अक्षर *FileName,
                                          std::string *Err) {
  निश्चित(FileName && "Use getPermanentLibrary() for opening process handle");
  व्योम *Handle = HandleSet::DLOpen(FileName, Err);
  अगर (Handle != &Invalid) {
    स्वतः &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  वापस DynamicLibrary(Handle);
}

व्योम DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  स्वतः &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  अगर (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.तथ्य_2);
    Lib.तथ्य_2 = &Invalid;
  }
}

व्योम *DynamicLibrary::get_Address_Of_प्रतीक(स्थिर अक्षर *प्रतीक_Name) {
  अगर (!isValid())
    वापस nullptr;
  वापस HandleSet::DLSym(तथ्य_2, प्रतीक_Name);
}

व्योम *DynamicLibrary::खोज_For_Address_Of_प्रतीक(स्थिर अक्षर *प्रतीक_Name) {
  {
    स्वतः &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<व्योम *>::iterator i = G.ExplicitSymbols.find(प्रतीक_Name);

    अगर (i != G.ExplicitSymbols.end())
      वापस i->second;

    // Now search the libraries.
    अगर (व्योम *Ptr = G.OpenedHandles.Lookup(प्रतीक_Name, खोज_Order))
      वापस Ptr;
    अगर (व्योम *Ptr = G.OpenedTemporaryHandles.Lookup(प्रतीक_Name, खोज_Order))
      वापस Ptr;
  }

  वापस llvm::खोज_For_Address_Of_Special_प्रतीक(प्रतीक_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(स्थिर अक्षर *Filename) {
  वापस llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

व्योम *LLVMSearch_For_Address_Of_प्रतीक(स्थिर अक्षर *प्रतीक_Name_2) {
  वापस llvm::sys::DynamicLibrary::खोज_For_Address_Of_प्रतीक(प्रतीक_Name_2);
}

व्योम LLVMAdd_प्रतीक(स्थिर अक्षर *प्रतीक_Name_2, व्योम *प्रतीक_मूल्य_2) {
  वापस llvm::sys::DynamicLibrary::Add_प्रतीक(प्रतीक_Name_2, प्रतीक_मूल्य_2);
}
