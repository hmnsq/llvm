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
  ساكن فراغ *DLSym(فراغ *Handle, ثابت حرف *علامت);

  HandleSet() = افتراضي;
  ~HandleSet();

  HandleList::iterator Find(فراغ *Handle) { أرجع find(Handles, Handle); }

  منطقي Contains(فراغ *Handle) {
    أرجع Handle == Process || Find(Handle) != Handles.end();
  }

  منطقي AddLibrary(فراغ *Handle, منطقي IsProcess = خطأ, منطقي CanClose = true,
                  منطقي AllowDuplicates = خطأ) {
#ifdef _WIN32
    أكد((Handle == هذا ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    إذا (LLVM_LIKELY(!IsProcess)) {
      إذا (!AllowDuplicates && Find(Handle) != Handles.end()) {
        إذا (CanClose)
          DLClose(Handle);
        أرجع خطأ;
      }
      Handles.ڈالو_back(Handle);
    } وإلا {
#ifndef _WIN32
      إذا (Process != &Invalid) {
        إذا (CanClose)
          DLClose(Process);
        إذا (Process == Handle)
          أرجع خطأ;
      }
#endif
      Process = Handle;
    }
    أرجع true;
  }

  فراغ CloseLibrary(فراغ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    إذا (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  فراغ *LibLookup(ثابت حرف *علامت, DynamicLibrary::تلاش_Ordering Order) {
    إذا (Order & SO_LoadOrder) {
      لكل (فراغ *Handle : Handles) {
        إذا (فراغ *Ptr = DLSym(Handle, علامت))
          أرجع Ptr;
      }
    } وإلا {
      لكل (فراغ *Handle : llvm::reverse(Handles)) {
        إذا (فراغ *Ptr = DLSym(Handle, علامت))
          أرجع Ptr;
      }
    }
    أرجع nullptr;
  }

  فراغ *Lookup(ثابت حرف *علامت, DynamicLibrary::تلاش_Ordering Order) {
    أكد(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    إذا (Process == &Invalid || (Order & SO_LoadedFirst)) {
      إذا (فراغ *Ptr = LibLookup(علامت, Order))
        أرجع Ptr;
    }
    إذا (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      إذا (فراغ *Ptr = DLSym(Process, علامت))
        أرجع Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      إذا (Order & SO_LoadedLast) {
        إذا (فراغ *Ptr = LibLookup(علامت, Order))
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
  llvm::sys::SmartMutex<true> SymbolsMutex;
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
DynamicLibrary::تلاش_Ordering DynamicLibrary::تلاش_Order =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *تلاش_For_Address_Of_Special_علامت(ثابت حرف *علامت_Name) {
  أرجع Do_تلاش(علامت_Name); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::Add_علامت(StringRef علامت_Name, فراغ *علامت_قدر) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[علامت_Name] = علامت_قدر;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  فراغ *Handle = HandleSet::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(فراغ *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  إذا (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ خطأ,
                                  /*CanClose*/ خطأ))
    *Err = "Library already loaded";

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت حرف *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  فراغ *Handle = HandleSet::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ خطأ,
                                        /*CanClose*/ خطأ,
                                        /*AllowDuplicates*/ true);
  }
  أرجع DynamicLibrary(Handle);
}

فراغ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  إذا (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

فراغ *DynamicLibrary::get_Address_Of_علامت(ثابت حرف *علامت_Name) {
  إذا (!isValid())
    أرجع nullptr;
  أرجع HandleSet::DLSym(Data, علامت_Name);
}

فراغ *DynamicLibrary::تلاش_For_Address_Of_علامت(ثابت حرف *علامت_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.find(علامت_Name);

    إذا (i != G.ExplicitSymbols.end())
      أرجع i->second;

    // Now search the libraries.
    إذا (فراغ *Ptr = G.OpenedHandles.Lookup(علامت_Name, تلاش_Order))
      أرجع Ptr;
    إذا (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(علامت_Name, تلاش_Order))
      أرجع Ptr;
  }

  أرجع llvm::تلاش_For_Address_Of_Special_علامت(علامت_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  أرجع llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

فراغ *LLVMSearch_For_Address_Of_علامت(ثابت حرف *علامت_Name_2) {
  أرجع llvm::sys::DynamicLibrary::تلاش_For_Address_Of_علامت(علامت_Name_2);
}

فراغ LLVMAdd_علامت(ثابت حرف *علامت_Name_2, فراغ *علامت_قدر_2) {
  أرجع llvm::sys::DynamicLibrary::Add_علامت(علامت_Name_2, علامت_قدر_2);
}
