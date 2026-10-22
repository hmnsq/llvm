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

#include "llvm/Support/DynamicLibrary.h"
#include "llvm-c/Support.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Mutex.h"
#include <vector>

using namespace llvm;
using namespace llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
class DynamicLibrary::HandleSet {
  using HandleList = std::vector<void *>;
  HandleList Handles;
  void *Process = &Invalid;

public:
  static void *DLOpen(const var_na *Filename, std::string *Err);
  static void DLClose(void *Handle);
  static void *DLSym(void *Handle, const var_na *Symbol);

  HandleSet() = default;
  ~HandleSet();

  HandleList::iterator Find(void *Handle) { ᤀᤡᤴ find(Handles, Handle); }

  bool Contains(void *Handle) {
    ᤀᤡᤴ Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(void *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#ifdef _WIN32
    assert((Handle == this ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    assert((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ᤀᤡ᤺ (LLVM_LIKELY(!IsProcess)) {
      ᤀᤡ᤺ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        ᤀᤡ᤺ (CanClose)
          DLClose(Handle);
        ᤀᤡᤴ false;
      }
      Handles.push_back(Handle);
    } ᤀᤡ᤺ᤆᤡ {
#ifndef _WIN32
      ᤀᤡ᤺ (Process != &Invalid) {
        ᤀᤡ᤺ (CanClose)
          DLClose(Process);
        ᤀᤡ᤺ (Process == Handle)
          ᤀᤡᤴ false;
      }
#endif
      Process = Handle;
    }
    ᤀᤡᤴ true;
  }

  void CloseLibrary(void *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    ᤀᤡ᤺ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  void *LibLookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    ᤀᤡ᤺ (Order & SO_LoadOrder) {
      for (void *Handle : Handles) {
        ᤀᤡ᤺ (void *Ptr = DLSym(Handle, Symbol))
          ᤀᤡᤴ Ptr;
      }
    } ᤀᤡ᤺ᤆᤡ {
      for (void *Handle : llvm::reverse(Handles)) {
        ᤀᤡ᤺ (void *Ptr = DLSym(Handle, Symbol))
          ᤀᤡᤴ Ptr;
      }
    }
    ᤀᤡᤴ nullptr;
  }

  void *Lookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    assert(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    ᤀᤡ᤺ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      ᤀᤡ᤺ (void *Ptr = LibLookup(Symbol, Order))
        ᤀᤡᤴ Ptr;
    }
    ᤀᤡ᤺ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ᤀᤡ᤺ (void *Ptr = DLSym(Process, Symbol))
        ᤀᤡᤴ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ᤀᤡ᤺ (Order & SO_LoadedLast) {
        ᤀᤡ᤺ (void *Ptr = LibLookup(Symbol, Order))
          ᤀᤡᤴ Ptr;
      }
    }
    ᤀᤡᤴ nullptr;
  }
};

namespace {

sa_nracanaa Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<void *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<true> SymbolsMutex;
};

Globals &getGlobals() {
  static Globals G;
  ᤀᤡᤴ G;
}

} // namespace

#ifdef _WIN32

#include "Windows/DynamicLibrary.inc"

#ᤀᤡ᤺ᤆᤡ

#include "Unix/DynamicLibrary.inc"

#endif

var_na DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

namespace llvm {
void *SearchForAddressOfSpecialSymbol(const var_na *SymbolName) {
  ᤀᤡᤴ DoSearch(SymbolName); // DynamicLibrary.inc
}
} // namespace llvm

void DynamicLibrary::AddSymbol(StringRef SymbolName, void *SymbolValue) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[SymbolName] = SymbolValue;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(const var_na *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  void *Handle = HandleSet::DLOpen(FileName, Err);
  ᤀᤡ᤺ (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ᤀᤡᤴ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(void *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ᤀᤡ᤺ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  ᤀᤡᤴ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(const var_na *FileName,
                                          std::string *Err) {
  assert(FileName && "Use getPermanentLibrary() for opening process handle");
  void *Handle = HandleSet::DLOpen(FileName, Err);
  ᤀᤡ᤺ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  ᤀᤡᤴ DynamicLibrary(Handle);
}

void DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  ᤀᤡ᤺ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

void *DynamicLibrary::getAddressOfSymbol(const var_na *SymbolName) {
  ᤀᤡ᤺ (!isValid())
    ᤀᤡᤴ nullptr;
  ᤀᤡᤴ HandleSet::DLSym(Data, SymbolName);
}

void *DynamicLibrary::SearchForAddressOfSymbol(const var_na *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<void *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    ᤀᤡ᤺ (i != G.ExplicitSymbols.end())
      ᤀᤡᤴ i->second;

    // Now search the libraries.
    ᤀᤡ᤺ (void *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      ᤀᤡᤴ Ptr;
    ᤀᤡ᤺ (void *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      ᤀᤡᤴ Ptr;
  }

  ᤀᤡᤴ llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(const var_na *Filename) {
  ᤀᤡᤴ llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

void *LLVMSearchForAddressOfSymbol(const var_na *symbolName) {
  ᤀᤡᤴ llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

void LLVMAddSymbol(const var_na *symbolName, void *symbolValue) {
  ᤀᤡᤴ llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
