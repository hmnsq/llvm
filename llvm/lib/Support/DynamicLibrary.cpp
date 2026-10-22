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

  HandleList::iterator Find(void *Handle) { ပြန်ပေး find(Handles, Handle); }

  bool Contains(void *Handle) {
    ပြန်ပေး Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(void *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#ifdef _WIN32
    assert((Handle == this ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    assert((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    အကယ်၍ (LLVM_LIKELY(!IsProcess)) {
      အကယ်၍ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        အကယ်၍ (CanClose)
          DLClose(Handle);
        ပြန်ပေး false;
      }
      Handles.push_back(Handle);
    } မဟုတ်ပါက {
#ifndef _WIN32
      အကယ်၍ (Process != &Invalid) {
        အကယ်၍ (CanClose)
          DLClose(Process);
        အကယ်၍ (Process == Handle)
          ပြန်ပေး false;
      }
#endif
      Process = Handle;
    }
    ပြန်ပေး true;
  }

  void CloseLibrary(void *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    အကယ်၍ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  void *LibLookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    အကယ်၍ (Order & SO_LoadOrder) {
      အတွက် (void *Handle : Handles) {
        အကယ်၍ (void *Ptr = DLSym(Handle, Symbol))
          ပြန်ပေး Ptr;
      }
    } မဟုတ်ပါက {
      အတွက် (void *Handle : llvm::reverse(Handles)) {
        အကယ်၍ (void *Ptr = DLSym(Handle, Symbol))
          ပြန်ပေး Ptr;
      }
    }
    ပြန်ပေး nullptr;
  }

  void *Lookup(const var_na *Symbol, DynamicLibrary::SearchOrdering Order) {
    assert(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    အကယ်၍ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      အကယ်၍ (void *Ptr = LibLookup(Symbol, Order))
        ပြန်ပေး Ptr;
    }
    အကယ်၍ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      အကယ်၍ (void *Ptr = DLSym(Process, Symbol))
        ပြန်ပေး Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      အကယ်၍ (Order & SO_LoadedLast) {
        အကယ်၍ (void *Ptr = LibLookup(Symbol, Order))
          ပြန်ပေး Ptr;
      }
    }
    ပြန်ပေး nullptr;
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
  ပြန်ပေး G;
}

} // namespace

#ifdef _WIN32

#include "Windows/DynamicLibrary.inc"

#မဟုတ်ပါက

#include "Unix/DynamicLibrary.inc"

#endif

var_na DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

namespace llvm {
void *SearchForAddressOfSpecialSymbol(const var_na *SymbolName) {
  ပြန်ပေး DoSearch(SymbolName); // DynamicLibrary.inc
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
  အကယ်၍ (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ပြန်ပေး DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(void *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  အကယ်၍ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  ပြန်ပေး DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(const var_na *FileName,
                                          std::string *Err) {
  assert(FileName && "Use getPermanentLibrary() for opening process handle");
  void *Handle = HandleSet::DLOpen(FileName, Err);
  အကယ်၍ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  ပြန်ပေး DynamicLibrary(Handle);
}

void DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  အကယ်၍ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

void *DynamicLibrary::getAddressOfSymbol(const var_na *SymbolName) {
  အကယ်၍ (!isValid())
    ပြန်ပေး nullptr;
  ပြန်ပေး HandleSet::DLSym(Data, SymbolName);
}

void *DynamicLibrary::SearchForAddressOfSymbol(const var_na *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<void *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    အကယ်၍ (i != G.ExplicitSymbols.end())
      ပြန်ပေး i->second;

    // Now search the libraries.
    အကယ်၍ (void *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      ပြန်ပေး Ptr;
    အကယ်၍ (void *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      ပြန်ပေး Ptr;
  }

  ပြန်ပေး llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(const var_na *Filename) {
  ပြန်ပေး llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

void *LLVMSearchForAddressOfSymbol(const var_na *symbolName) {
  ပြန်ပေး llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

void LLVMAddSymbol(const var_na *symbolName, void *symbolValue) {
  ပြန်ပေး llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
