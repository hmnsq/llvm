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
صنف DynamicLibrary::HandleSet {
  using HandleList = std::vector<فراغ *>;
  HandleList Handles;
  فراغ *Process = &Invalid;

عام:
  ساكن فراغ *DLOpen(ثابت حرف *Filename, std::string *Err);
  ساكن فراغ DLClose(فراغ *Handle);
  ساكن فراغ *DLSym(فراغ *Handle, ثابت حرف *Symbol);

  HandleSet() = افتراضي;
  ~HandleSet();

  HandleList::iterator Find(فراغ *Handle) { ܦܢܐ find(Handles, Handle); }

  منطقي Contains(فراغ *Handle) {
    ܦܢܐ Handle == Process || Find(Handle) != Handles.end();
  }

  منطقي AddLibrary(فراغ *Handle, منطقي IsProcess = ܕܓܠܐ, منطقي CanClose = ܫܪܪܐ,
                  منطقي AllowDuplicates = ܕܓܠܐ) {
#ifdef _WIN32
    أكد((Handle == هذا ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ܐܢ (LLVM_LIKELY(!IsProcess)) {
      ܐܢ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        ܐܢ (CanClose)
          DLClose(Handle);
        ܦܢܐ ܕܓܠܐ;
      }
      Handles.push_back(Handle);
    } ܐܚܪܢܐ {
#ifndef _WIN32
      ܐܢ (Process != &Invalid) {
        ܐܢ (CanClose)
          DLClose(Process);
        ܐܢ (Process == Handle)
          ܦܢܐ ܕܓܠܐ;
      }
#endif
      Process = Handle;
    }
    ܦܢܐ ܫܪܪܐ;
  }

  فراغ CloseLibrary(فراغ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    ܐܢ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  فراغ *LibLookup(ثابت حرف *Symbol, DynamicLibrary::SearchOrdering Order) {
    ܐܢ (Order & SO_LoadOrder) {
      لكل (فراغ *Handle : Handles) {
        ܐܢ (فراغ *Ptr = DLSym(Handle, Symbol))
          ܦܢܐ Ptr;
      }
    } ܐܚܪܢܐ {
      لكل (فراغ *Handle : llvm::reverse(Handles)) {
        ܐܢ (فراغ *Ptr = DLSym(Handle, Symbol))
          ܦܢܐ Ptr;
      }
    }
    ܦܢܐ nullptr;
  }

  فراغ *Lookup(ثابت حرف *Symbol, DynamicLibrary::SearchOrdering Order) {
    أكد(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    ܐܢ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      ܐܢ (فراغ *Ptr = LibLookup(Symbol, Order))
        ܦܢܐ Ptr;
    }
    ܐܢ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ܐܢ (فراغ *Ptr = DLSym(Process, Symbol))
        ܦܢܐ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ܐܢ (Order & SO_LoadedLast) {
        ܐܢ (فراغ *Ptr = LibLookup(Symbol, Order))
          ܦܢܐ Ptr;
      }
    }
    ܦܢܐ nullptr;
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
  llvm::sys::SmartMutex<ܫܪܪܐ> SymbolsMutex;
};

Globals &getGlobals() {
  ساكن Globals G;
  ܦܢܐ G;
}

} // namespace

#ifdef _WIN32

#ضمن "Windows/DynamicLibrary.inc"

#ܐܚܪܢܐ

#ضمن "Unix/DynamicLibrary.inc"

#endif

حرف DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *SearchForAddressOfSpecialSymbol(ثابت حرف *SymbolName) {
  ܦܢܐ DoSearch(SymbolName); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::AddSymbol(StringRef SymbolName, فراغ *SymbolValue) {
  auto &G = getGlobals();
  SmartScopedLock<ܫܪܪܐ> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[SymbolName] = SymbolValue;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  فراغ *Handle = HandleSet::DLOpen(FileName, Err);
  ܐܢ (Handle != &Invalid) {
    SmartScopedLock<ܫܪܪܐ> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ܦܢܐ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(فراغ *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<ܫܪܪܐ> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ܐܢ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ ܕܓܠܐ,
                                  /*CanClose*/ ܕܓܠܐ))
    *Err = "Library already loaded";

  ܦܢܐ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت حرف *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  فراغ *Handle = HandleSet::DLOpen(FileName, Err);
  ܐܢ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<ܫܪܪܐ> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ ܕܓܠܐ,
                                        /*CanClose*/ ܕܓܠܐ,
                                        /*AllowDuplicates*/ ܫܪܪܐ);
  }
  ܦܢܐ DynamicLibrary(Handle);
}

فراغ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<ܫܪܪܐ> Lock(G.SymbolsMutex);
  ܐܢ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

فراغ *DynamicLibrary::getAddressOfSymbol(ثابت حرف *SymbolName) {
  ܐܢ (!isValid())
    ܦܢܐ nullptr;
  ܦܢܐ HandleSet::DLSym(Data, SymbolName);
}

فراغ *DynamicLibrary::SearchForAddressOfSymbol(ثابت حرف *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<ܫܪܪܐ> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    ܐܢ (i != G.ExplicitSymbols.end())
      ܦܢܐ i->second;

    // Now search the libraries.
    ܐܢ (فراغ *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      ܦܢܐ Ptr;
    ܐܢ (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      ܦܢܐ Ptr;
  }

  ܦܢܐ llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  ܦܢܐ llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

فراغ *LLVMSearchForAddressOfSymbol(ثابت حرف *symbolName) {
  ܦܢܐ llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

فراغ LLVMAddSymbol(ثابت حرف *symbolName, فراغ *symbolValue) {
  ܦܢܐ llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
