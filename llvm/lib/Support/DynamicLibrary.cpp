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

#അന്തര്ഭാവ "llvm/Support/DynamicLibrary.h"
#അന്തര്ഭാവ "llvm-c/Support.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringMap.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Mutex.h"
#അന്തര്ഭാവ <vector>

using നാമാകാശ llvm;
using നാമാകാശ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
വർഗ്ഗം DynamicLibrary::HandleSet {
  using HandleList = std::vector<ശൂന്യം *>;
  HandleList Handles;
  ശൂന്യം *Process = &Invalid;

പൊതു:
  സ്ഥിതം ശൂന്യം *DLOpen(സ്ഥിരം അക്ഷരം *Filename, std::string *Err);
  സ്ഥിതം ശൂന്യം DLClose(ശൂന്യം *Handle);
  സ്ഥിതം ശൂന്യം *DLSym(ശൂന്യം *Handle, സ്ഥിരം അക്ഷരം *ചിഹ്നം);

  HandleSet() = സ്ഥിരസ്ഥിതി;
  ~HandleSet();

  HandleList::iterator Find(ശൂന്യം *Handle) { തിരികെ find(Handles, Handle); }

  തർക്കം Contains(ശൂന്യം *Handle) {
    തിരികെ Handle == Process || Find(Handle) != Handles.end();
  }

  തർക്കം AddLibrary(ശൂന്യം *Handle, തർക്കം IsProcess = അസത്യം, തർക്കം CanClose = സത്യം,
                  തർക്കം AllowDuplicates = അസത്യം) {
#ifdef _WIN32
    പ്രതിജ്ഞാ((Handle == ഇത് ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    പ്രതിജ്ഞാ((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    എങ്കിൽ (LLVM_LIKELY(!IsProcess)) {
      എങ്കിൽ (!AllowDuplicates && Find(Handle) != Handles.end()) {
        എങ്കിൽ (CanClose)
          DLClose(Handle);
        തിരികെ അസത്യം;
      }
      Handles.തള്ളുക_back(Handle);
    } അല്ലെങ്കിൽ {
#ifndef _WIN32
      എങ്കിൽ (Process != &Invalid) {
        എങ്കിൽ (CanClose)
          DLClose(Process);
        എങ്കിൽ (Process == Handle)
          തിരികെ അസത്യം;
      }
#endif
      Process = Handle;
    }
    തിരികെ സത്യം;
  }

  ശൂന്യം CloseLibrary(ശൂന്യം *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    എങ്കിൽ (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  ശൂന്യം *LibLookup(സ്ഥിരം അക്ഷരം *ചിഹ്നം, DynamicLibrary::തിരയൽ_Ordering Order) {
    എങ്കിൽ (Order & SO_LoadOrder) {
      വേണ്ടി (ശൂന്യം *Handle : Handles) {
        എങ്കിൽ (ശൂന്യം *Ptr = DLSym(Handle, ചിഹ്നം))
          തിരികെ Ptr;
      }
    } അല്ലെങ്കിൽ {
      വേണ്ടി (ശൂന്യം *Handle : llvm::reverse(Handles)) {
        എങ്കിൽ (ശൂന്യം *Ptr = DLSym(Handle, ചിഹ്നം))
          തിരികെ Ptr;
      }
    }
    തിരികെ nullptr;
  }

  ശൂന്യം *Lookup(സ്ഥിരം അക്ഷരം *ചിഹ്നം, DynamicLibrary::തിരയൽ_Ordering Order) {
    പ്രതിജ്ഞാ(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    എങ്കിൽ (Process == &Invalid || (Order & SO_LoadedFirst)) {
      എങ്കിൽ (ശൂന്യം *Ptr = LibLookup(ചിഹ്നം, Order))
        തിരികെ Ptr;
    }
    എങ്കിൽ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      എങ്കിൽ (ശൂന്യം *Ptr = DLSym(Process, ചിഹ്നം))
        തിരികെ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      എങ്കിൽ (Order & SO_LoadedLast) {
        എങ്കിൽ (ശൂന്യം *Ptr = LibLookup(ചിഹ്നം, Order))
          തിരികെ Ptr;
      }
    }
    തിരികെ nullptr;
  }
};

നാമാകാശ {

ഘടന Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<ശൂന്യം *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<സത്യം> SymbolsMutex;
};

Globals &getGlobals() {
  സ്ഥിതം Globals G;
  തിരികെ G;
}

} // namespace

#ifdef _WIN32

#അന്തര്ഭാവ "Windows/DynamicLibrary.inc"

#അല്ലെങ്കിൽ

#അന്തര്ഭാവ "Unix/DynamicLibrary.inc"

#endif

അക്ഷരം DynamicLibrary::Invalid;
DynamicLibrary::തിരയൽ_Ordering DynamicLibrary::തിരയൽ_Order =
    DynamicLibrary::SO_Linker;

നാമാകാശ llvm {
ശൂന്യം *തിരയൽ_For_Address_Of_Special_ചിഹ്നം(സ്ഥിരം അക്ഷരം *ചിഹ്നം_Name) {
  തിരികെ Do_തിരയൽ(ചിഹ്നം_Name); // DynamicLibrary.inc
}
} // namespace llvm

ശൂന്യം DynamicLibrary::Add_ചിഹ്നം(StringRef ചിഹ്നം_Name, ശൂന്യം *ചിഹ്നം_മൂല്യം) {
  auto &G = getGlobals();
  SmartScopedLock<സത്യം> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[ചിഹ്നം_Name] = ചിഹ്നം_മൂല്യം;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(സ്ഥിരം അക്ഷരം *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  ശൂന്യം *Handle = HandleSet::DLOpen(FileName, Err);
  എങ്കിൽ (Handle != &Invalid) {
    SmartScopedLock<സത്യം> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  തിരികെ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(ശൂന്യം *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<സത്യം> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  എങ്കിൽ (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ അസത്യം,
                                  /*CanClose*/ അസത്യം))
    *Err = "Library already loaded";

  തിരികെ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(സ്ഥിരം അക്ഷരം *FileName,
                                          std::string *Err) {
  പ്രതിജ്ഞാ(FileName && "Use getPermanentLibrary() for opening process handle");
  ശൂന്യം *Handle = HandleSet::DLOpen(FileName, Err);
  എങ്കിൽ (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<സത്യം> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ അസത്യം,
                                        /*CanClose*/ അസത്യം,
                                        /*AllowDuplicates*/ സത്യം);
  }
  തിരികെ DynamicLibrary(Handle);
}

ശൂന്യം DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<സത്യം> Lock(G.SymbolsMutex);
  എങ്കിൽ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

ശൂന്യം *DynamicLibrary::get_Address_Of_ചിഹ്നം(സ്ഥിരം അക്ഷരം *ചിഹ്നം_Name) {
  എങ്കിൽ (!isValid())
    തിരികെ nullptr;
  തിരികെ HandleSet::DLSym(Data, ചിഹ്നം_Name);
}

ശൂന്യം *DynamicLibrary::തിരയൽ_For_Address_Of_ചിഹ്നം(സ്ഥിരം അക്ഷരം *ചിഹ്നം_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<സത്യം> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<ശൂന്യം *>::iterator i = G.ExplicitSymbols.find(ചിഹ്നം_Name);

    എങ്കിൽ (i != G.ExplicitSymbols.end())
      തിരികെ i->second;

    // Now search the libraries.
    എങ്കിൽ (ശൂന്യം *Ptr = G.OpenedHandles.Lookup(ചിഹ്നം_Name, തിരയൽ_Order))
      തിരികെ Ptr;
    എങ്കിൽ (ശൂന്യം *Ptr = G.OpenedTemporaryHandles.Lookup(ചിഹ്നം_Name, തിരയൽ_Order))
      തിരികെ Ptr;
  }

  തിരികെ llvm::തിരയൽ_For_Address_Of_Special_ചിഹ്നം(ചിഹ്നം_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(സ്ഥിരം അക്ഷരം *Filename) {
  തിരികെ llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

ശൂന്യം *LLVMSearch_For_Address_Of_ചിഹ്നം(സ്ഥിരം അക്ഷരം *ചിഹ്നം_Name_2) {
  തിരികെ llvm::sys::DynamicLibrary::തിരയൽ_For_Address_Of_ചിഹ്നം(ചിഹ്നം_Name_2);
}

ശൂന്യം LLVMAdd_ചിഹ്നം(സ്ഥിരം അക്ഷരം *ചിഹ്നം_Name_2, ശൂന്യം *ചിഹ്നം_മൂല്യം_2) {
  തിരികെ llvm::sys::DynamicLibrary::Add_ചിഹ്നം(ചിഹ്നം_Name_2, ചിഹ്നം_മൂല്യം_2);
}
