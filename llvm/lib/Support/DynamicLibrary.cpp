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

  HandleList::iterator Find(void *Handle) { 返回 find(Handles, Handle); }

  bool Contains(void *Handle) {
    返回 Handle == Process || Find(Handle) != Handles.end();
  }

  bool AddLibrary(void *Handle, bool IsProcess = false, bool CanClose = true,
                  bool AllowDuplicates = false) {
#ifdef _WIN32
    assert((Handle == this ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    assert((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    如果 (LLVM_LIKELY(!IsProcess)) {
      如果 (!AllowDuplicates && Find(Handle) != Handles.end()) {
        如果 (CanClose)
          DLClose(Handle);
        返回 false;
      }
      Handles.push_back(Handle);
    } 否则 {
#ifndef _WIN32
      如果 (Process != &Invalid) {
        如果 (CanClose)
          DLClose(Process);
        如果 (Process == Handle)
          返回 false;
      }
#endif
      Process = Handle;
    }
    返回 true;
  }

  void CloseLibrary(void *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    如果 (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  void *LibLookup(const char *Symbol, DynamicLibrary::SearchOrdering Order) {
    如果 (Order & SO_LoadOrder) {
      为 (void *Handle : Handles) {
        如果 (void *Ptr = DLSym(Handle, Symbol))
          返回 Ptr;
      }
    } 否则 {
      为 (void *Handle : llvm::reverse(Handles)) {
        如果 (void *Ptr = DLSym(Handle, Symbol))
          返回 Ptr;
      }
    }
    返回 nullptr;
  }

  void *Lookup(const char *Symbol, DynamicLibrary::SearchOrdering Order) {
    assert(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    如果 (Process == &Invalid || (Order & SO_LoadedFirst)) {
      如果 (void *Ptr = LibLookup(Symbol, Order))
        返回 Ptr;
    }
    如果 (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      如果 (void *Ptr = DLSym(Process, Symbol))
        返回 Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      如果 (Order & SO_LoadedLast) {
        如果 (void *Ptr = LibLookup(Symbol, Order))
          返回 Ptr;
      }
    }
    返回 nullptr;
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
  返回 G;
}

} // namespace

#ifdef _WIN32

#include "Windows/DynamicLibrary.inc"

#否则

#include "Unix/DynamicLibrary.inc"

#endif

char DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

namespace llvm {
void *SearchForAddressOfSpecialSymbol(const char *SymbolName) {
  返回 DoSearch(SymbolName); // DynamicLibrary.inc
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
  如果 (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  返回 DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(void *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  如果 (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                  /*CanClose*/ false))
    *Err = "Library already loaded";

  返回 DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(const char *FileName,
                                          std::string *Err) {
  assert(FileName && "Use getPermanentLibrary() for opening process handle");
  void *Handle = HandleSet::DLOpen(FileName, Err);
  如果 (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ false,
                                        /*CanClose*/ false,
                                        /*AllowDuplicates*/ true);
  }
  返回 DynamicLibrary(Handle);
}

void DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  如果 (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

void *DynamicLibrary::getAddressOfSymbol(const char *SymbolName) {
  如果 (!isValid())
    返回 nullptr;
  返回 HandleSet::DLSym(Data, SymbolName);
}

void *DynamicLibrary::SearchForAddressOfSymbol(const char *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<void *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    如果 (i != G.ExplicitSymbols.end())
      返回 i->second;

    // Now search the libraries.
    如果 (void *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      返回 Ptr;
    如果 (void *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      返回 Ptr;
  }

  返回 llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(const char *Filename) {
  返回 llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

void *LLVMSearchForAddressOfSymbol(const char *symbolName) {
  返回 llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

void LLVMAddSymbol(const char *symbolName, void *symbolValue) {
  返回 llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
