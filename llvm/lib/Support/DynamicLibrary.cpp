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

  HandleList::iterator Find(فراغ *Handle) { أرجع find(Handles, Handle); }

  منطقي Contains(فراغ *Handle) {
    أرجع Handle == Process || Find(Handle) != Handles.end();
  }

  منطقي AddLibrary(فراغ *Handle, منطقي IsProcess = خطأ, منطقي CanClose = 𐡒𐡔𐡈,
                  منطقي AllowDuplicates = خطأ) {
#ifdef _WIN32
    أكد((Handle == هذا ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    𐡄𐡍 (LLVM_LIKELY(!IsProcess)) {
      𐡄𐡍 (!AllowDuplicates && Find(Handle) != Handles.end()) {
        𐡄𐡍 (CanClose)
          DLClose(Handle);
        أرجع خطأ;
      }
      Handles.push_back(Handle);
    } وإلا {
#ifndef _WIN32
      𐡄𐡍 (Process != &Invalid) {
        𐡄𐡍 (CanClose)
          DLClose(Process);
        𐡄𐡍 (Process == Handle)
          أرجع خطأ;
      }
#endif
      Process = Handle;
    }
    أرجع 𐡒𐡔𐡈;
  }

  فراغ CloseLibrary(فراغ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    𐡄𐡍 (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  فراغ *LibLookup(ثابت حرف *Symbol, DynamicLibrary::SearchOrdering Order) {
    𐡄𐡍 (Order & SO_LoadOrder) {
      لكل (فراغ *Handle : Handles) {
        𐡄𐡍 (فراغ *Ptr = DLSym(Handle, Symbol))
          أرجع Ptr;
      }
    } وإلا {
      لكل (فراغ *Handle : llvm::reverse(Handles)) {
        𐡄𐡍 (فراغ *Ptr = DLSym(Handle, Symbol))
          أرجع Ptr;
      }
    }
    أرجع nullptr;
  }

  فراغ *Lookup(ثابت حرف *Symbol, DynamicLibrary::SearchOrdering Order) {
    أكد(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    𐡄𐡍 (Process == &Invalid || (Order & SO_LoadedFirst)) {
      𐡄𐡍 (فراغ *Ptr = LibLookup(Symbol, Order))
        أرجع Ptr;
    }
    𐡄𐡍 (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      𐡄𐡍 (فراغ *Ptr = DLSym(Process, Symbol))
        أرجع Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      𐡄𐡍 (Order & SO_LoadedLast) {
        𐡄𐡍 (فراغ *Ptr = LibLookup(Symbol, Order))
          أرجع Ptr;
      }
    }
    أرجع nullptr;
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
  llvm::sys::SmartMutex<𐡒𐡔𐡈> SymbolsMutex;
};

Globals &getGlobals() {
  ساكن Globals G;
  أرجع G;
}

} // namespace

#ifdef _WIN32

#ضمن "Windows/DynamicLibrary.inc"

#وإلا

#ضمن "Unix/DynamicLibrary.inc"

#endif

حرف DynamicLibrary::Invalid;
DynamicLibrary::SearchOrdering DynamicLibrary::SearchOrder =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *SearchForAddressOfSpecialSymbol(ثابت حرف *SymbolName) {
  أرجع DoSearch(SymbolName); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::AddSymbol(StringRef SymbolName, فراغ *SymbolValue) {
  auto &G = getGlobals();
  SmartScopedLock<𐡒𐡔𐡈> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[SymbolName] = SymbolValue;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  فراغ *Handle = HandleSet::DLOpen(FileName, Err);
  𐡄𐡍 (Handle != &Invalid) {
    SmartScopedLock<𐡒𐡔𐡈> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(فراغ *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<𐡒𐡔𐡈> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  𐡄𐡍 (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ خطأ,
                                  /*CanClose*/ خطأ))
    *Err = "Library already loaded";

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت حرف *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  فراغ *Handle = HandleSet::DLOpen(FileName, Err);
  𐡄𐡍 (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<𐡒𐡔𐡈> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ خطأ,
                                        /*CanClose*/ خطأ,
                                        /*AllowDuplicates*/ 𐡒𐡔𐡈);
  }
  أرجع DynamicLibrary(Handle);
}

فراغ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<𐡒𐡔𐡈> Lock(G.SymbolsMutex);
  𐡄𐡍 (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

فراغ *DynamicLibrary::getAddressOfSymbol(ثابت حرف *SymbolName) {
  𐡄𐡍 (!isValid())
    أرجع nullptr;
  أرجع HandleSet::DLSym(Data, SymbolName);
}

فراغ *DynamicLibrary::SearchForAddressOfSymbol(ثابت حرف *SymbolName) {
  {
    auto &G = getGlobals();
    SmartScopedLock<𐡒𐡔𐡈> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.find(SymbolName);

    𐡄𐡍 (i != G.ExplicitSymbols.end())
      أرجع i->second;

    // Now search the libraries.
    𐡄𐡍 (فراغ *Ptr = G.OpenedHandles.Lookup(SymbolName, SearchOrder))
      أرجع Ptr;
    𐡄𐡍 (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(SymbolName, SearchOrder))
      أرجع Ptr;
  }

  أرجع llvm::SearchForAddressOfSpecialSymbol(SymbolName);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  أرجع llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

فراغ *LLVMSearchForAddressOfSymbol(ثابت حرف *symbolName) {
  أرجع llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(symbolName);
}

فراغ LLVMAddSymbol(ثابت حرف *symbolName, فراغ *symbolValue) {
  أرجع llvm::sys::DynamicLibrary::AddSymbol(symbolName, symbolValue);
}
