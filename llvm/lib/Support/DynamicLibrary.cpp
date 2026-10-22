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

  HandleList::iterator Find(void *Handle) { ສົ່ງຄືນ find(Handles, Handle); }

  bool Contains(void *Handle) {
    ສົ່ງຄືນ Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(void *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#ifdef _WIN32
    assert((Handle == this ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    assert((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ຖ້າ (LLVM_LIKELY(!IsProcess)) {
      ຖ້າ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        ຖ້າ (CanClose)
          DLClose(Handle);
        ສົ່ງຄືນ false;
      }
      Handles.push_back(Handle);
    } ບໍ່ດັ່ງນັ້ນ {
#ifndef _WIN32
      ຖ້າ (Process != &Invalid) {
        ຖ້າ (CanClose)
          DLClose(Process);
        ຖ້າ (Process == Handle)
          ສົ່ງຄືນ false;
      }
#endif
      Process = Handle;
    }
    ສົ່ງຄືນ true;
  }

  void CloseLibrary(void *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    ຖ້າ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  void *LibLookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    ຖ້າ (Order & SO_LoadOrder) {
      ສຳລັບ (void *Handle : Handles) {
        ຖ້າ (void *Ptr = DLSym(Handle, Symbol))
          ສົ່ງຄືນ Ptr;
      }
    } ບໍ່ດັ່ງນັ້ນ {
      ສຳລັບ (void *Handle : llvm::reverse(Handles)) {
        ຖ້າ (void *Ptr = DLSym(Handle, Symbol))
          ສົ່ງຄືນ Ptr;
      }
    }
    ສົ່ງຄືນ nullptr;
  }

  void *Lookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    assert(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    ຖ້າ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      ຖ້າ (void *Ptr = LibLookup(Symbol, Order))
        ສົ່ງຄືນ Ptr;
    }
    ຖ້າ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ຖ້າ (void *Ptr = DLSym(Process, Symbol))
        ສົ່ງຄືນ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ຖ້າ (Order & SO_LoadedLast) {
        ຖ້າ (void *Ptr = LibLookup(Symbol, Order))
          ສົ່ງຄືນ Ptr;
      }
    }
    ສົ່ງຄືນ nullptr;
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
  ສົ່ງຄືນ G;
}

} // namespace

#ifdef _WIN32

#include "Windows/DynamicLibrary.inc"

#ບໍ່ດັ່ງນັ້ນ

#include "Unix/DynamicLibrary.inc"

#endif

var_na DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

namespace llvm {
void *SearchForAddressOfSpecialSymbol(const var_na *SymbolName) {
  ສົ່ງຄືນ DoSearch(SymbolName); // DynamicLibrary.inc
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
  ຖ້າ (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ສົ່ງຄືນ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(void *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ຖ້າ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  ສົ່ງຄືນ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(const var_na *FileName,
                                          std::string *Err) {
  assert(FileName && "Use getPermanentLibrary() for opening process handle");
  void *Handle = HandleSet::DLOpen(FileName, Err);
  ຖ້າ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  ສົ່ງຄືນ DynamicLibrary(Handle);
}

void DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  ຖ້າ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

void *DynamicLibrary::getAddressOfSymbol(const var_na *SymbolName) {
  ຖ້າ (!isValid())
    ສົ່ງຄືນ nullptr;
  ສົ່ງຄືນ HandleSet::DLSym(Data, SymbolName);
}

void *DynamicLibrary::SearchForAddressOfSymbol(const var_na *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<void *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    ຖ້າ (i != G.ExplicitSymbols.end())
      ສົ່ງຄືນ i->second;

    // Now search the libraries.
    ຖ້າ (void *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      ສົ່ງຄືນ Ptr;
    ຖ້າ (void *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      ສົ່ງຄືນ Ptr;
  }

  ສົ່ງຄືນ llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(const var_na *Filename) {
  ສົ່ງຄືນ llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

void *LLVMSearchForAddressOfSymbol(const var_na *symbolName) {
  ສົ່ງຄືນ llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

void LLVMAddSymbol(const var_na *symbolName, void *symbolValue) {
  ສົ່ງຄືນ llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
