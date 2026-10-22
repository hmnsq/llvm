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

#ضمن "llvm/Support/DynamicLibrary.h"
#ضمن "llvm-c/Support.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Mutex.h"
#ضمن <vector>

using نطاق llvm;
using نطاق llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
מחלקה DynamicLibrary::HandleSet {
  using HandleList = std::vector<فراغ *>;
  HandleList Handles;
  فراغ *Process = &Invalid;

عام:
  ساكن فراغ *DLOpen(ثابت حرف *Filename, std::string *Err);
  ساكن فراغ DLClose(فراغ *Handle);
  ساكن فراغ *DLSym(فراغ *Handle, ثابت حرف *Symbol);

  HandleSet() = افتراضي;
  ~HandleSet();

  HandleList::iterator Find(فراغ *Handle) { החזר find(Handles, Handle); }

  منطقي Contains(فراغ *Handle) {
    החזר Handle == Process || Find(Handle) != Handles.end();
  }

  منطقي AddLibrary(فراغ *Handle, منطقي IsProcess = שקר, منطقي CanClose = אמת,
                  منطقي AllowDuplicates = שקר) {
#ifdef _WIN32
    أكد((Handle == هذا ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    אם (LLVM_LIKELY(!IsProcess)) {
      אם (!AllowDuplicates && Find(Handle) != Handles.end()) {
        אם (CanClose)
          DLClose(Handle);
        החזר שקר;
      }
      Handles.push_back(Handle);
    } אחרת {
#ifndef _WIN32
      אם (Process != &Invalid) {
        אם (CanClose)
          DLClose(Process);
        אם (Process == Handle)
          החזר שקר;
      }
#endif
      Process = Handle;
    }
    החזר אמת;
  }

  فراغ CloseLibrary(فراغ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    אם (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  فراغ *LibLookup(ثابت حرف *Symbol, DynamicLibrary::SearchOrdering Order) {
    אם (Order & SO_LoadOrder) {
      עבור (فراغ *Handle : Handles) {
        אם (فراغ *Ptr = DLSym(Handle, Symbol))
          החזר Ptr;
      }
    } אחרת {
      עבור (فراغ *Handle : llvm::reverse(Handles)) {
        אם (فراغ *Ptr = DLSym(Handle, Symbol))
          החזר Ptr;
      }
    }
    החזר nullptr;
  }

  فراغ *Lookup(ثابت حرف *Symbol, DynamicLibrary::SearchOrdering Order) {
    أكد(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    אם (Process == &Invalid || (Order & SO_LoadedFirst)) {
      אם (فراغ *Ptr = LibLookup(Symbol, Order))
        החזר Ptr;
    }
    אם (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      אם (فراغ *Ptr = DLSym(Process, Symbol))
        החזר Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      אם (Order & SO_LoadedLast) {
        אם (فراغ *Ptr = LibLookup(Symbol, Order))
          החזר Ptr;
      }
    }
    החזר nullptr;
  }
};

نطاق {

بنية Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<فراغ *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<אמת> SymbolsMutex;
};

Globals &getGlobals() {
  ساكن Globals G;
  החזר G;
}

} // namespace

#ifdef _WIN32

#ضمن "Windows/DynamicLibrary.inc"

#אחרת

#ضمن "Unix/DynamicLibrary.inc"

#endif

حرف DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *SearchForAddressOfSpecialSymbol(ثابت حرف *SymbolName) {
  החזר DoSearch(SymbolName); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::AddSymbol(StringRef SymbolName, فراغ *SymbolValue) {
  auto &G = getGlobals();
  SmartScopedLock<אמת> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[SymbolName] = SymbolValue;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  فراغ *Handle = HandleSet::DLOpen(FileName, Err);
  אם (Handle != &Invalid) {
    SmartScopedLock<אמת> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  החזר DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(فراغ *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<אמת> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  אם (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ שקר,
                                  /*CanClose*/ שקר))
    *Err = "Library already loaded";

  החזר DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت حرف *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  فراغ *Handle = HandleSet::DLOpen(FileName, Err);
  אם (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<אמת> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ שקר,
                                        /*CanClose*/ שקר,
                                        /*AllowDuplicates*/ אמת);
  }
  החזר DynamicLibrary(Handle);
}

فراغ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<אמת> Lock(G.SymbolsMutex);
  אם (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

فراغ *DynamicLibrary::getAddressOfSymbol(ثابت حرف *SymbolName) {
  אם (!isValid())
    החזר nullptr;
  החזר HandleSet::DLSym(Data, SymbolName);
}

فراغ *DynamicLibrary::SearchForAddressOfSymbol(ثابت حرف *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<אמת> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    אם (i != G.ExplicitSymbols.end())
      החזר i->second;

    // Now search the libraries.
    אם (فراغ *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      החזר Ptr;
    אם (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      החזר Ptr;
  }

  החזר llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  החזר llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

فراغ *LLVMSearchForAddressOfSymbol(ثابت حرف *symbolName) {
  החזר llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

فراغ LLVMAddSymbol(ثابت حرف *symbolName, فراغ *symbolValue) {
  החזר llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
