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
  static void *DLOpen(const char *Filename, std::string *Err);
  static void DLClose(void *Handle);
  static void *DLSym(void *Handle, const char *Symbol);

  HandleSet() = default;
  ~HandleSet();

  HandleList::iterator Find(void *Handle) { врати find(Handles, Handle); }

  bool Contains(void *Handle) {
    врати Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(void *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#ifdef _WIN32
    assert((Handle == this ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    assert((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ако (LLVM_LIKELY(!IsProcess)) {
      ако (!AllowDuplicates && Find(Handle) != Handles.end()) {
        ако (CanClose)
          DLClose(Handle);
        врати false;
      }
      Handles.push_back(Handle);
    } инако {
#ifndef _WIN32
      ако (Process != &Invalid) {
        ако (CanClose)
          DLClose(Process);
        ако (Process == Handle)
          врати false;
      }
#endif
      Process = Handle;
    }
    врати true;
  }

  void CloseLibrary(void *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    ако (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  void *LibLookup(const char *Symbol, DynamicLibrary::SearchOrdering Order) {
    ако (Order & SO_LoadOrder) {
      за (void *Handle : Handles) {
        ако (void *Ptr = DLSym(Handle, Symbol))
          врати Ptr;
      }
    } инако {
      за (void *Handle : llvm::reverse(Handles)) {
        ако (void *Ptr = DLSym(Handle, Symbol))
          врати Ptr;
      }
    }
    врати nullptr;
  }

  void *Lookup(const char *Symbol, DynamicLibrary::SearchOrdering Order) {
    assert(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    ако (Process == &Invalid || (Order & SO_LoadedFirst)) {
      ако (void *Ptr = LibLookup(Symbol, Order))
        врати Ptr;
    }
    ако (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ако (void *Ptr = DLSym(Process, Symbol))
        врати Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ако (Order & SO_LoadedLast) {
        ако (void *Ptr = LibLookup(Symbol, Order))
          врати Ptr;
      }
    }
    врати nullptr;
  }
};

namespace {

struct Globals {
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
  врати G;
}

} // namespace

#ifdef _WIN32

#include "Windows/DynamicLibrary.inc"

#инако

#include "Unix/DynamicLibrary.inc"

#endif

char DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

namespace llvm {
void *SearchForAddressOfSpecialSymbol(const char *SymbolName) {
  врати DoSearch(SymbolName); // DynamicLibrary.inc
}
} // namespace llvm

void DynamicLibrary::AddSymbol(StringRef SymbolName, void *SymbolValue) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[SymbolName] = SymbolValue;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(const char *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  void *Handle = HandleSet::DLOpen(FileName, Err);
  ако (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  врати DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(void *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ако (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  врати DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(const char *FileName,
                                          std::string *Err) {
  assert(FileName && "Use getPermanentLibrary() for opening process handle");
  void *Handle = HandleSet::DLOpen(FileName, Err);
  ако (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  врати DynamicLibrary(Handle);
}

void DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  ако (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

void *DynamicLibrary::getAddressOfSymbol(const char *SymbolName) {
  ако (!isValid())
    врати nullptr;
  врати HandleSet::DLSym(Data, SymbolName);
}

void *DynamicLibrary::SearchForAddressOfSymbol(const char *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<void *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    ако (i != G.ExplicitSymbols.end())
      врати i->second;

    // Now search the libraries.
    ако (void *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      врати Ptr;
    ако (void *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      врати Ptr;
  }

  врати llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(const char *Filename) {
  врати llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

void *LLVMSearchForAddressOfSymbol(const char *symbolName) {
  врати llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

void LLVMAddSymbol(const char *symbolName, void *symbolValue) {
  врати llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
