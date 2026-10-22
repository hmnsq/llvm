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

  HandleList::iterator Find(void *Handle) { ត្រឡប់ find(Handles, Handle); }

  bool Contains(void *Handle) {
    ត្រឡប់ Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(void *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#ifdef _WIN32
    assert((Handle == this ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    assert((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ប្រសិនបើ (LLVM_LIKELY(!IsProcess)) {
      ប្រសិនបើ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        ប្រសិនបើ (CanClose)
          DLClose(Handle);
        ត្រឡប់ false;
      }
      Handles.push_back(Handle);
    } បើមិនដូច្នេះ {
#ifndef _WIN32
      ប្រសិនបើ (Process != &Invalid) {
        ប្រសិនបើ (CanClose)
          DLClose(Process);
        ប្រសិនបើ (Process == Handle)
          ត្រឡប់ false;
      }
#endif
      Process = Handle;
    }
    ត្រឡប់ true;
  }

  void CloseLibrary(void *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    ប្រសិនបើ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  void *LibLookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    ប្រសិនបើ (Order & SO_LoadOrder) {
      សម្រាប់ (void *Handle : Handles) {
        ប្រសិនបើ (void *Ptr = DLSym(Handle, Symbol))
          ត្រឡប់ Ptr;
      }
    } បើមិនដូច្នេះ {
      សម្រាប់ (void *Handle : llvm::reverse(Handles)) {
        ប្រសិនបើ (void *Ptr = DLSym(Handle, Symbol))
          ត្រឡប់ Ptr;
      }
    }
    ត្រឡប់ nullptr;
  }

  void *Lookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    assert(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    ប្រសិនបើ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      ប្រសិនបើ (void *Ptr = LibLookup(Symbol, Order))
        ត្រឡប់ Ptr;
    }
    ប្រសិនបើ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ប្រសិនបើ (void *Ptr = DLSym(Process, Symbol))
        ត្រឡប់ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ប្រសិនបើ (Order & SO_LoadedLast) {
        ប្រសិនបើ (void *Ptr = LibLookup(Symbol, Order))
          ត្រឡប់ Ptr;
      }
    }
    ត្រឡប់ nullptr;
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
  ត្រឡប់ G;
}

} // namespace

#ifdef _WIN32

#include "Windows/DynamicLibrary.inc"

#បើមិនដូច្នេះ

#include "Unix/DynamicLibrary.inc"

#endif

var_na DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

namespace llvm {
void *SearchForAddressOfSpecialSymbol(const var_na *SymbolName) {
  ត្រឡប់ DoSearch(SymbolName); // DynamicLibrary.inc
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
  ប្រសិនបើ (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ត្រឡប់ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(void *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ប្រសិនបើ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  ត្រឡប់ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(const var_na *FileName,
                                          std::string *Err) {
  assert(FileName && "Use getPermanentLibrary() for opening process handle");
  void *Handle = HandleSet::DLOpen(FileName, Err);
  ប្រសិនបើ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  ត្រឡប់ DynamicLibrary(Handle);
}

void DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  ប្រសិនបើ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

void *DynamicLibrary::getAddressOfSymbol(const var_na *SymbolName) {
  ប្រសិនបើ (!isValid())
    ត្រឡប់ nullptr;
  ត្រឡប់ HandleSet::DLSym(Data, SymbolName);
}

void *DynamicLibrary::SearchForAddressOfSymbol(const var_na *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<void *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    ប្រសិនបើ (i != G.ExplicitSymbols.end())
      ត្រឡប់ i->second;

    // Now search the libraries.
    ប្រសិនបើ (void *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      ត្រឡប់ Ptr;
    ប្រសិនបើ (void *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      ត្រឡប់ Ptr;
  }

  ត្រឡប់ llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(const var_na *Filename) {
  ត្រឡប់ llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

void *LLVMSearchForAddressOfSymbol(const var_na *symbolName) {
  ត្រឡប់ llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

void LLVMAddSymbol(const var_na *symbolName, void *symbolValue) {
  ត្រឡប់ llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
