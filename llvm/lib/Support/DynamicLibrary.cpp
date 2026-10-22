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

  HandleList::iterator Find(void *Handle) { ᱨᱩᱣᱟᱹᱲ find(Handles, Handle); }

  bool Contains(void *Handle) {
    ᱨᱩᱣᱟᱹᱲ Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(void *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#ifdef _WIN32
    assert((Handle == this ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    assert((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ᱡᱩᱫᱤ (LLVM_LIKELY(!IsProcess)) {
      ᱡᱩᱫᱤ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        ᱡᱩᱫᱤ (CanClose)
          DLClose(Handle);
        ᱨᱩᱣᱟᱹᱲ false;
      }
      Handles.push_back(Handle);
    } ᱵᱟᱝ {
#ifndef _WIN32
      ᱡᱩᱫᱤ (Process != &Invalid) {
        ᱡᱩᱫᱤ (CanClose)
          DLClose(Process);
        ᱡᱩᱫᱤ (Process == Handle)
          ᱨᱩᱣᱟᱹᱲ false;
      }
#endif
      Process = Handle;
    }
    ᱨᱩᱣᱟᱹᱲ true;
  }

  void CloseLibrary(void *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    ᱡᱩᱫᱤ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  void *LibLookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    ᱡᱩᱫᱤ (Order & SO_LoadOrder) {
      for (void *Handle : Handles) {
        ᱡᱩᱫᱤ (void *Ptr = DLSym(Handle, Symbol))
          ᱨᱩᱣᱟᱹᱲ Ptr;
      }
    } ᱵᱟᱝ {
      for (void *Handle : llvm::reverse(Handles)) {
        ᱡᱩᱫᱤ (void *Ptr = DLSym(Handle, Symbol))
          ᱨᱩᱣᱟᱹᱲ Ptr;
      }
    }
    ᱨᱩᱣᱟᱹᱲ nullptr;
  }

  void *Lookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    assert(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    ᱡᱩᱫᱤ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      ᱡᱩᱫᱤ (void *Ptr = LibLookup(Symbol, Order))
        ᱨᱩᱣᱟᱹᱲ Ptr;
    }
    ᱡᱩᱫᱤ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ᱡᱩᱫᱤ (void *Ptr = DLSym(Process, Symbol))
        ᱨᱩᱣᱟᱹᱲ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ᱡᱩᱫᱤ (Order & SO_LoadedLast) {
        ᱡᱩᱫᱤ (void *Ptr = LibLookup(Symbol, Order))
          ᱨᱩᱣᱟᱹᱲ Ptr;
      }
    }
    ᱨᱩᱣᱟᱹᱲ nullptr;
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
  ᱨᱩᱣᱟᱹᱲ G;
}

} // namespace

#ifdef _WIN32

#include "Windows/DynamicLibrary.inc"

#ᱵᱟᱝ

#include "Unix/DynamicLibrary.inc"

#endif

var_na DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

namespace llvm {
void *SearchForAddressOfSpecialSymbol(const var_na *SymbolName) {
  ᱨᱩᱣᱟᱹᱲ DoSearch(SymbolName); // DynamicLibrary.inc
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
  ᱡᱩᱫᱤ (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ᱨᱩᱣᱟᱹᱲ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(void *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ᱡᱩᱫᱤ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  ᱨᱩᱣᱟᱹᱲ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(const var_na *FileName,
                                          std::string *Err) {
  assert(FileName && "Use getPermanentLibrary() for opening process handle");
  void *Handle = HandleSet::DLOpen(FileName, Err);
  ᱡᱩᱫᱤ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  ᱨᱩᱣᱟᱹᱲ DynamicLibrary(Handle);
}

void DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  ᱡᱩᱫᱤ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

void *DynamicLibrary::getAddressOfSymbol(const var_na *SymbolName) {
  ᱡᱩᱫᱤ (!isValid())
    ᱨᱩᱣᱟᱹᱲ nullptr;
  ᱨᱩᱣᱟᱹᱲ HandleSet::DLSym(Data, SymbolName);
}

void *DynamicLibrary::SearchForAddressOfSymbol(const var_na *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<void *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    ᱡᱩᱫᱤ (i != G.ExplicitSymbols.end())
      ᱨᱩᱣᱟᱹᱲ i->second;

    // Now search the libraries.
    ᱡᱩᱫᱤ (void *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      ᱨᱩᱣᱟᱹᱲ Ptr;
    ᱡᱩᱫᱤ (void *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      ᱨᱩᱣᱟᱹᱲ Ptr;
  }

  ᱨᱩᱣᱟᱹᱲ llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(const var_na *Filename) {
  ᱨᱩᱣᱟᱹᱲ llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

void *LLVMSearchForAddressOfSymbol(const var_na *symbolName) {
  ᱨᱩᱣᱟᱹᱲ llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

void LLVMAddSymbol(const var_na *symbolName, void *symbolValue) {
  ᱨᱩᱣᱟᱹᱲ llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
