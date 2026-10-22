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
  ساكن فراغ *DLSym(فراغ *Handle, ثابت حرف *هێما);

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
      Handles.پاڵنان_back(Handle);
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

  فراغ *LibLookup(ثابت حرف *هێما, DynamicLibrary::گەڕان_Ordering Order) {
    إذا (Order & SO_LoadOrder) {
      لكل (فراغ *Handle : Handles) {
        إذا (فراغ *Ptr = DLSym(Handle, هێما))
          أرجع Ptr;
      }
    } وإلا {
      لكل (فراغ *Handle : llvm::reverse(Handles)) {
        إذا (فراغ *Ptr = DLSym(Handle, هێما))
          أرجع Ptr;
      }
    }
    أرجع nullptr;
  }

  فراغ *Lookup(ثابت حرف *هێما, DynamicLibrary::گەڕان_Ordering Order) {
    أكد(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    إذا (Process == &Invalid || (Order & SO_LoadedFirst)) {
      إذا (فراغ *Ptr = LibLookup(هێما, Order))
        أرجع Ptr;
    }
    إذا (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      إذا (فراغ *Ptr = DLSym(Process, هێما))
        أرجع Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      إذا (Order & SO_LoadedLast) {
        إذا (فراغ *Ptr = LibLookup(هێما, Order))
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
DynamicLibrary::گەڕان_Ordering DynamicLibrary::گەڕان_Order =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *گەڕان_For_Address_Of_Special_هێما(ثابت حرف *هێما_Name) {
  أرجع Do_گەڕان(هێما_Name); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::Add_هێما(StringRef هێما_Name, فراغ *هێما_بەها) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[هێما_Name] = هێما_بەها;
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

فراغ *DynamicLibrary::get_Address_Of_هێما(ثابت حرف *هێما_Name) {
  إذا (!isValid())
    أرجع nullptr;
  أرجع HandleSet::DLSym(Data, هێما_Name);
}

فراغ *DynamicLibrary::گەڕان_For_Address_Of_هێما(ثابت حرف *هێما_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.find(هێما_Name);

    إذا (i != G.ExplicitSymbols.end())
      أرجع i->second;

    // Now search the libraries.
    إذا (فراغ *Ptr = G.OpenedHandles.Lookup(هێما_Name, گەڕان_Order))
      أرجع Ptr;
    إذا (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(هێما_Name, گەڕان_Order))
      أرجع Ptr;
  }

  أرجع llvm::گەڕان_For_Address_Of_Special_هێما(هێما_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  أرجع llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

فراغ *LLVMSearch_For_Address_Of_هێما(ثابت حرف *هێما_Name_2) {
  أرجع llvm::sys::DynamicLibrary::گەڕان_For_Address_Of_هێما(هێما_Name_2);
}

فراغ LLVMAdd_هێما(ثابت حرف *هێما_Name_2, فراغ *هێما_بەها_2) {
  أرجع llvm::sys::DynamicLibrary::Add_هێما(هێما_Name_2, هێما_بەها_2);
}
