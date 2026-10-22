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

  HandleList::iterator Find(void *Handle) { ꦮꦭꦶ find(Handles, Handle); }

  bool Contains(void *Handle) {
    ꦮꦭꦶ Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(void *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#ifdef _WIN32
    assert((Handle == this ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    assert((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ꦪꦼꦤ꧀ (LLVM_LIKELY(!IsProcess)) {
      ꦪꦼꦤ꧀ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        ꦪꦼꦤ꧀ (CanClose)
          DLClose(Handle);
        ꦮꦭꦶ false;
      }
      Handles.push_back(Handle);
    } ꦭꦶꦪ {
#ifndef _WIN32
      ꦪꦼꦤ꧀ (Process != &Invalid) {
        ꦪꦼꦤ꧀ (CanClose)
          DLClose(Process);
        ꦪꦼꦤ꧀ (Process == Handle)
          ꦮꦭꦶ false;
      }
#endif
      Process = Handle;
    }
    ꦮꦭꦶ true;
  }

  void CloseLibrary(void *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    ꦪꦼꦤ꧀ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  void *LibLookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    ꦪꦼꦤ꧀ (Order & SO_LoadOrder) {
      ꦏꦁꦒꦺꦴ (void *Handle : Handles) {
        ꦪꦼꦤ꧀ (void *Ptr = DLSym(Handle, Symbol))
          ꦮꦭꦶ Ptr;
      }
    } ꦭꦶꦪ {
      ꦏꦁꦒꦺꦴ (void *Handle : llvm::reverse(Handles)) {
        ꦪꦼꦤ꧀ (void *Ptr = DLSym(Handle, Symbol))
          ꦮꦭꦶ Ptr;
      }
    }
    ꦮꦭꦶ nullptr;
  }

  void *Lookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    assert(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    ꦪꦼꦤ꧀ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      ꦪꦼꦤ꧀ (void *Ptr = LibLookup(Symbol, Order))
        ꦮꦭꦶ Ptr;
    }
    ꦪꦼꦤ꧀ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ꦪꦼꦤ꧀ (void *Ptr = DLSym(Process, Symbol))
        ꦮꦭꦶ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ꦪꦼꦤ꧀ (Order & SO_LoadedLast) {
        ꦪꦼꦤ꧀ (void *Ptr = LibLookup(Symbol, Order))
          ꦮꦭꦶ Ptr;
      }
    }
    ꦮꦭꦶ nullptr;
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
  ꦮꦭꦶ G;
}

} // namespace

#ifdef _WIN32

#include "Windows/DynamicLibrary.inc"

#ꦭꦶꦪ

#include "Unix/DynamicLibrary.inc"

#endif

var_na DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

namespace llvm {
void *SearchForAddressOfSpecialSymbol(const var_na *SymbolName) {
  ꦮꦭꦶ DoSearch(SymbolName); // DynamicLibrary.inc
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
  ꦪꦼꦤ꧀ (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ꦮꦭꦶ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(void *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ꦪꦼꦤ꧀ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  ꦮꦭꦶ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(const var_na *FileName,
                                          std::string *Err) {
  assert(FileName && "Use getPermanentLibrary() for opening process handle");
  void *Handle = HandleSet::DLOpen(FileName, Err);
  ꦪꦼꦤ꧀ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  ꦮꦭꦶ DynamicLibrary(Handle);
}

void DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  ꦪꦼꦤ꧀ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

void *DynamicLibrary::getAddressOfSymbol(const var_na *SymbolName) {
  ꦪꦼꦤ꧀ (!isValid())
    ꦮꦭꦶ nullptr;
  ꦮꦭꦶ HandleSet::DLSym(Data, SymbolName);
}

void *DynamicLibrary::SearchForAddressOfSymbol(const var_na *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<void *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    ꦪꦼꦤ꧀ (i != G.ExplicitSymbols.end())
      ꦮꦭꦶ i->second;

    // Now search the libraries.
    ꦪꦼꦤ꧀ (void *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      ꦮꦭꦶ Ptr;
    ꦪꦼꦤ꧀ (void *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      ꦮꦭꦶ Ptr;
  }

  ꦮꦭꦶ llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(const var_na *Filename) {
  ꦮꦭꦶ llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

void *LLVMSearchForAddressOfSymbol(const var_na *symbolName) {
  ꦮꦭꦶ llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

void LLVMAddSymbol(const var_na *symbolName, void *symbolValue) {
  ꦮꦭꦶ llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
