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

#ಅನ್ತರ್ಭಾವ "llvm/Support/DynamicLibrary.h"
#ಅನ್ತರ್ಭಾವ "llvm-c/Support.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Mutex.h"
#ಅನ್ತರ್ಭಾವ <vector>

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
ವರ್ಗ DynamicLibrary::HandleSet {
  using HandleList = std::vector<ಶೂನ್ಯ *>;
  HandleList Handles;
  ಶೂನ್ಯ *Process = &Invalid;

ಸಾರ್ವಜನಿಕ:
  ಸ್ಥಿರ ಶೂನ್ಯ *DLOpen(ಧ್ರುವ ವರ್ಣ *Filename, std::string *Err);
  ಸ್ಥಿರ ಶೂನ್ಯ DLClose(ಶೂನ್ಯ *Handle);
  ಸ್ಥಿರ ಶೂನ್ಯ *DLSym(ಶೂನ್ಯ *Handle, ಧ್ರುವ ವರ್ಣ *ಚಿಹ್ನೆ);

  HandleSet() = ಸ್ವಭಾವ;
  ~HandleSet();

  HandleList::iterator Find(ಶೂನ್ಯ *Handle) { ಪ್ರತ್ಯಾಗಮ find(Handles, Handle); }

  ತಾರ್ಕಿಕ Contains(ಶೂನ್ಯ *Handle) {
    ಪ್ರತ್ಯಾಗಮ Handle == Process || Find(Handle) != Handles.end();
  }

  ತಾರ್ಕಿಕ AddLibrary(ಶೂನ್ಯ *Handle, ತಾರ್ಕಿಕ IsProcess = ಅಸತ್ಯ, ತಾರ್ಕಿಕ CanClose = ಸತ್ಯ,
                  ತಾರ್ಕಿಕ AllowDuplicates = ಅಸತ್ಯ) {
#ifdef _WIN32
    ಪ್ರತಿಜ್ಞಾ((Handle == ಏತತ್ ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    ಪ್ರತಿಜ್ಞಾ((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ಯದಿ (LLVM_LIKELY(!IsProcess)) {
      ಯದಿ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        ಯದಿ (CanClose)
          DLClose(Handle);
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      }
      Handles.ತಳ್ಳು_back(Handle);
    } ಅನ್ಯಥಾ {
#ifndef _WIN32
      ಯದಿ (Process != &Invalid) {
        ಯದಿ (CanClose)
          DLClose(Process);
        ಯದಿ (Process == Handle)
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      }
#endif
      Process = Handle;
    }
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  ಶೂನ್ಯ CloseLibrary(ಶೂನ್ಯ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    ಯದಿ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  ಶೂನ್ಯ *LibLookup(ಧ್ರುವ ವರ್ಣ *ಚಿಹ್ನೆ, DynamicLibrary::ಹುಡುಕು_Ordering Order) {
    ಯದಿ (Order & SO_LoadOrder) {
      ಪ್ರತಿ (ಶೂನ್ಯ *Handle : Handles) {
        ಯದಿ (ಶೂನ್ಯ *Ptr = DLSym(Handle, ಚಿಹ್ನೆ))
          ಪ್ರತ್ಯಾಗಮ Ptr;
      }
    } ಅನ್ಯಥಾ {
      ಪ್ರತಿ (ಶೂನ್ಯ *Handle : llvm::reverse(Handles)) {
        ಯದಿ (ಶೂನ್ಯ *Ptr = DLSym(Handle, ಚಿಹ್ನೆ))
          ಪ್ರತ್ಯಾಗಮ Ptr;
      }
    }
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }

  ಶೂನ್ಯ *Lookup(ಧ್ರುವ ವರ್ಣ *ಚಿಹ್ನೆ, DynamicLibrary::ಹುಡುಕು_Ordering Order) {
    ಪ್ರತಿಜ್ಞಾ(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    ಯದಿ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      ಯದಿ (ಶೂನ್ಯ *Ptr = LibLookup(ಚಿಹ್ನೆ, Order))
        ಪ್ರತ್ಯಾಗಮ Ptr;
    }
    ಯದಿ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ಯದಿ (ಶೂನ್ಯ *Ptr = DLSym(Process, ಚಿಹ್ನೆ))
        ಪ್ರತ್ಯಾಗಮ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ಯದಿ (Order & SO_LoadedLast) {
        ಯದಿ (ಶೂನ್ಯ *Ptr = LibLookup(ಚಿಹ್ನೆ, Order))
          ಪ್ರತ್ಯಾಗಮ Ptr;
      }
    }
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }
};

ನಾಮಾಕಾಶ {

ಸಂರಚನಾ Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<ಶೂನ್ಯ *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<ಸತ್ಯ> SymbolsMutex;
};

Globals &getGlobals() {
  ಸ್ಥಿರ Globals G;
  ಪ್ರತ್ಯಾಗಮ G;
}

} // namespace

#ifdef _WIN32

#ಅನ್ತರ್ಭಾವ "Windows/DynamicLibrary.inc"

#ಅನ್ಯಥಾ

#ಅನ್ತರ್ಭಾವ "Unix/DynamicLibrary.inc"

#endif

ವರ್ಣ DynamicLibrary::Invalid;
DynamicLibrary::ಹುಡುಕು_Ordering DynamicLibrary::ಹುಡುಕು_Order =
    DynamicLibrary::SO_Linker;

ನಾಮಾಕಾಶ llvm {
ಶೂನ್ಯ *ಹುಡುಕು_For_Address_Of_Special_ಚಿಹ್ನೆ(ಧ್ರುವ ವರ್ಣ *ಚಿಹ್ನೆ_Name) {
  ಪ್ರತ್ಯಾಗಮ Do_ಹುಡುಕು(ಚಿಹ್ನೆ_Name); // DynamicLibrary.inc
}
} // namespace llvm

ಶೂನ್ಯ DynamicLibrary::Add_ಚಿಹ್ನೆ(StringRef ಚಿಹ್ನೆ_Name, ಶೂನ್ಯ *ಚಿಹ್ನೆ_ಮೌಲ್ಯ) {
  auto &G = getGlobals();
  SmartScopedLock<ಸತ್ಯ> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[ಚಿಹ್ನೆ_Name] = ಚಿಹ್ನೆ_ಮೌಲ್ಯ;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ಧ್ರುವ ವರ್ಣ *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  ಶೂನ್ಯ *Handle = HandleSet::DLOpen(FileName, Err);
  ಯದಿ (Handle != &Invalid) {
    SmartScopedLock<ಸತ್ಯ> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ಪ್ರತ್ಯಾಗಮ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(ಶೂನ್ಯ *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<ಸತ್ಯ> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ಯದಿ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ ಅಸತ್ಯ,
                                  /*CanClose*/ ಅಸತ್ಯ))
    *Err = "Library already loaded";

  ಪ್ರತ್ಯಾಗಮ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ಧ್ರುವ ವರ್ಣ *FileName,
                                          std::string *Err) {
  ಪ್ರತಿಜ್ಞಾ(FileName && "Use getPermanentLibrary() for opening process handle");
  ಶೂನ್ಯ *Handle = HandleSet::DLOpen(FileName, Err);
  ಯದಿ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<ಸತ್ಯ> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ ಅಸತ್ಯ,
                                        /*CanClose*/ ಅಸತ್ಯ,
                                        /*AllowDuplicates*/ ಸತ್ಯ);
  }
  ಪ್ರತ್ಯಾಗಮ DynamicLibrary(Handle);
}

ಶೂನ್ಯ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<ಸತ್ಯ> Lock(G.SymbolsMutex);
  ಯದಿ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

ಶೂನ್ಯ *DynamicLibrary::get_Address_Of_ಚಿಹ್ನೆ(ಧ್ರುವ ವರ್ಣ *ಚಿಹ್ನೆ_Name) {
  ಯದಿ (!isValid())
    ಪ್ರತ್ಯಾಗಮ nullptr;
  ಪ್ರತ್ಯಾಗಮ HandleSet::DLSym(Data, ಚಿಹ್ನೆ_Name);
}

ಶೂನ್ಯ *DynamicLibrary::ಹುಡುಕು_For_Address_Of_ಚಿಹ್ನೆ(ಧ್ರುವ ವರ್ಣ *ಚಿಹ್ನೆ_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<ಸತ್ಯ> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<ಶೂನ್ಯ *>::iterator i = G.ExplicitSymbols.find(ಚಿಹ್ನೆ_Name);

    ಯದಿ (i != G.ExplicitSymbols.end())
      ಪ್ರತ್ಯಾಗಮ i->second;

    // Now search the libraries.
    ಯದಿ (ಶೂನ್ಯ *Ptr = G.OpenedHandles.Lookup(ಚಿಹ್ನೆ_Name, ಹುಡುಕು_Order))
      ಪ್ರತ್ಯಾಗಮ Ptr;
    ಯದಿ (ಶೂನ್ಯ *Ptr = G.OpenedTemporaryHandles.Lookup(ಚಿಹ್ನೆ_Name, ಹುಡುಕು_Order))
      ಪ್ರತ್ಯಾಗಮ Ptr;
  }

  ಪ್ರತ್ಯಾಗಮ llvm::ಹುಡುಕು_For_Address_Of_Special_ಚಿಹ್ನೆ(ಚಿಹ್ನೆ_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ಧ್ರುವ ವರ್ಣ *Filename) {
  ಪ್ರತ್ಯಾಗಮ llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

ಶೂನ್ಯ *LLVMSearch_For_Address_Of_ಚಿಹ್ನೆ(ಧ್ರುವ ವರ್ಣ *ಚಿಹ್ನೆ_Name_2) {
  ಪ್ರತ್ಯಾಗಮ llvm::sys::DynamicLibrary::ಹುಡುಕು_For_Address_Of_ಚಿಹ್ನೆ(ಚಿಹ್ನೆ_Name_2);
}

ಶೂನ್ಯ LLVMAdd_ಚಿಹ್ನೆ(ಧ್ರುವ ವರ್ಣ *ಚಿಹ್ನೆ_Name_2, ಶೂನ್ಯ *ಚಿಹ್ನೆ_ಮೌಲ್ಯ_2) {
  ಪ್ರತ್ಯಾಗಮ llvm::sys::DynamicLibrary::Add_ಚಿಹ್ನೆ(ಚಿಹ್ನೆ_Name_2, ಚಿಹ್ನೆ_ಮೌಲ್ಯ_2);
}
