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
  ساكن فراغ *DLSym(فراغ *Handle, ثابت حرف *رمز);

  HandleSet() = افتراضي;
  ~HandleSet();

  HandleList::iterator Find(فراغ *Handle) { إرجاع find(Handles, Handle); }

  منطقي Contains(فراغ *Handle) {
    إرجاع Handle == Process || Find(Handle) != Handles.end();
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
        إرجاع خطأ;
      }
      Handles.ادفع_back(Handle);
    } وإلا {
#ifndef _WIN32
      إذا (Process != &Invalid) {
        إذا (CanClose)
          DLClose(Process);
        إذا (Process == Handle)
          إرجاع خطأ;
      }
#endif
      Process = Handle;
    }
    إرجاع true;
  }

  فراغ CloseLibrary(فراغ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    إذا (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  فراغ *LibLookup(ثابت حرف *رمز, DynamicLibrary::بحث_Ordering Order) {
    إذا (Order & SO_LoadOrder) {
      لكل (فراغ *Handle : Handles) {
        إذا (فراغ *Ptr = DLSym(Handle, رمز))
          إرجاع Ptr;
      }
    } وإلا {
      لكل (فراغ *Handle : llvm::reverse(Handles)) {
        إذا (فراغ *Ptr = DLSym(Handle, رمز))
          إرجاع Ptr;
      }
    }
    إرجاع nullptr;
  }

  فراغ *Lookup(ثابت حرف *رمز, DynamicLibrary::بحث_Ordering Order) {
    أكد(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    إذا (Process == &Invalid || (Order & SO_LoadedFirst)) {
      إذا (فراغ *Ptr = LibLookup(رمز, Order))
        إرجاع Ptr;
    }
    إذا (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      إذا (فراغ *Ptr = DLSym(Process, رمز))
        إرجاع Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      إذا (Order & SO_LoadedLast) {
        إذا (فراغ *Ptr = LibLookup(رمز, Order))
          إرجاع Ptr;
      }
    }
    إرجاع nullptr;
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
  إرجاع G;
}

} // namespace

#ifdef _WIN32

#ضمن "Windows/DynamicLibrary.inc"

#وإلا

#ضمن "Unix/DynamicLibrary.inc"

#endif

حرف DynamicLibrary::Invalid;
DynamicLibrary::بحث_Ordering DynamicLibrary::بحث_Order =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *بحث_For_Address_Of_Special_رمز(ثابت حرف *رمز_Name) {
  إرجاع Do_بحث(رمز_Name); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::Add_رمز(StringRef رمز_Name, فراغ *رمز_قيمة) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[رمز_Name] = رمز_قيمة;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  فراغ *Handle = HandleSet::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    SmartScopedLock<true> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  إرجاع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(فراغ *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  إذا (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ خطأ,
                                  /*CanClose*/ خطأ))
    *Err = "Library already loaded";

  إرجاع DynamicLibrary(Handle);
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
  إرجاع DynamicLibrary(Handle);
}

فراغ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<true> Lock(G.SymbolsMutex);
  إذا (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

فراغ *DynamicLibrary::get_Address_Of_رمز(ثابت حرف *رمز_Name) {
  إذا (!isValid())
    إرجاع nullptr;
  إرجاع HandleSet::DLSym(Data, رمز_Name);
}

فراغ *DynamicLibrary::بحث_For_Address_Of_رمز(ثابت حرف *رمز_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<true> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.find(رمز_Name);

    إذا (i != G.ExplicitSymbols.end())
      إرجاع i->second;

    // Now search the libraries.
    إذا (فراغ *Ptr = G.OpenedHandles.Lookup(رمز_Name, بحث_Order))
      إرجاع Ptr;
    إذا (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(رمز_Name, بحث_Order))
      إرجاع Ptr;
  }

  إرجاع llvm::بحث_For_Address_Of_Special_رمز(رمز_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  إرجاع llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

فراغ *LLVMSearch_For_Address_Of_رمز(ثابت حرف *رمز_Name_2) {
  إرجاع llvm::sys::DynamicLibrary::بحث_For_Address_Of_رمز(رمز_Name_2);
}

فراغ LLVMAdd_رمز(ثابت حرف *رمز_Name_2, فراغ *رمز_قيمة_2) {
  إرجاع llvm::sys::DynamicLibrary::Add_رمز(رمز_Name_2, رمز_قيمة_2);
}
