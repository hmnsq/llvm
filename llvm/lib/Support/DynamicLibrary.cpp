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

#شامل "llvm/Support/DynamicLibrary.h"
#شامل "llvm-c/Support.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/StringMap.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Mutex.h"
#شامل <vector>

using فضاینام llvm;
using فضاینام llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
رده DynamicLibrary::HandleSet {
  using HandleList = std::vector<تهی *>;
  HandleList Handles;
  تهی *Process = &Invalid;

عمومی:
  ایستا تهی *DLOpen(ثابت نویسه *Filename, std::string *Err);
  ایستا تهی DLClose(تهی *Handle);
  ایستا تهی *DLSym(تهی *Handle, ثابت نویسه *نماد);

  HandleSet() = پیشفرض;
  ~HandleSet();

  HandleList::iterator Find(تهی *Handle) { بازگشت find(Handles, Handle); }

  منطقی Contains(تهی *Handle) {
    بازگشت Handle == Process || Find(Handle) != Handles.end();
  }

  منطقی AddLibrary(تهی *Handle, منطقی IsProcess = نادرست, منطقی CanClose = درست,
                  منطقی AllowDuplicates = نادرست) {
#ifdef _WIN32
    أكد((Handle == این ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    اگر (LLVM_LIKELY(!IsProcess)) {
      اگر (!AllowDuplicates && Find(Handle) != Handles.end()) {
        اگر (CanClose)
          DLClose(Handle);
        بازگشت نادرست;
      }
      Handles.افزودن_back(Handle);
    } وگرنه {
#ifndef _WIN32
      اگر (Process != &Invalid) {
        اگر (CanClose)
          DLClose(Process);
        اگر (Process == Handle)
          بازگشت نادرست;
      }
#endif
      Process = Handle;
    }
    بازگشت درست;
  }

  تهی CloseLibrary(تهی *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    اگر (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  تهی *LibLookup(ثابت نویسه *نماد, DynamicLibrary::جستجو_Ordering Order) {
    اگر (Order & SO_LoadOrder) {
      برای (تهی *Handle : Handles) {
        اگر (تهی *Ptr = DLSym(Handle, نماد))
          بازگشت Ptr;
      }
    } وگرنه {
      برای (تهی *Handle : llvm::reverse(Handles)) {
        اگر (تهی *Ptr = DLSym(Handle, نماد))
          بازگشت Ptr;
      }
    }
    بازگشت nullptr;
  }

  تهی *Lookup(ثابت نویسه *نماد, DynamicLibrary::جستجو_Ordering Order) {
    أكد(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    اگر (Process == &Invalid || (Order & SO_LoadedFirst)) {
      اگر (تهی *Ptr = LibLookup(نماد, Order))
        بازگشت Ptr;
    }
    اگر (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      اگر (تهی *Ptr = DLSym(Process, نماد))
        بازگشت Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      اگر (Order & SO_LoadedLast) {
        اگر (تهی *Ptr = LibLookup(نماد, Order))
          بازگشت Ptr;
      }
    }
    بازگشت nullptr;
  }
};

فضاینام {

ساختار Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<تهی *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<درست> SymbolsMutex;
};

Globals &getGlobals() {
  ایستا Globals G;
  بازگشت G;
}

} // namespace

#ifdef _WIN32

#شامل "Windows/DynamicLibrary.inc"

#وگرنه

#شامل "Unix/DynamicLibrary.inc"

#endif

نویسه DynamicLibrary::Invalid;
DynamicLibrary::جستجو_Ordering DynamicLibrary::جستجو_Order =
    DynamicLibrary::SO_Linker;

فضاینام llvm {
تهی *جستجو_For_Address_Of_Special_نماد(ثابت نویسه *نماد_Name) {
  بازگشت Do_جستجو(نماد_Name); // DynamicLibrary.inc
}
} // namespace llvm

تهی DynamicLibrary::Add_نماد(StringRef نماد_Name, تهی *نماد_مقدار) {
  auto &G = getGlobals();
  SmartScopedLock<درست> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[نماد_Name] = نماد_مقدار;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت نویسه *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  تهی *Handle = HandleSet::DLOpen(FileName, Err);
  اگر (Handle != &Invalid) {
    SmartScopedLock<درست> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  بازگشت DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(تهی *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<درست> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  اگر (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ نادرست,
                                  /*CanClose*/ نادرست))
    *Err = "Library already loaded";

  بازگشت DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت نویسه *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  تهی *Handle = HandleSet::DLOpen(FileName, Err);
  اگر (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<درست> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ نادرست,
                                        /*CanClose*/ نادرست,
                                        /*AllowDuplicates*/ درست);
  }
  بازگشت DynamicLibrary(Handle);
}

تهی DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<درست> Lock(G.SymbolsMutex);
  اگر (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

تهی *DynamicLibrary::get_Address_Of_نماد(ثابت نویسه *نماد_Name) {
  اگر (!isValid())
    بازگشت nullptr;
  بازگشت HandleSet::DLSym(Data, نماد_Name);
}

تهی *DynamicLibrary::جستجو_For_Address_Of_نماد(ثابت نویسه *نماد_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<درست> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<تهی *>::iterator i = G.ExplicitSymbols.find(نماد_Name);

    اگر (i != G.ExplicitSymbols.end())
      بازگشت i->second;

    // Now search the libraries.
    اگر (تهی *Ptr = G.OpenedHandles.Lookup(نماد_Name, جستجو_Order))
      بازگشت Ptr;
    اگر (تهی *Ptr = G.OpenedTemporaryHandles.Lookup(نماد_Name, جستجو_Order))
      بازگشت Ptr;
  }

  بازگشت llvm::جستجو_For_Address_Of_Special_نماد(نماد_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت نویسه *Filename) {
  بازگشت llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

تهی *LLVMSearch_For_Address_Of_نماد(ثابت نویسه *نماد_Name_2) {
  بازگشت llvm::sys::DynamicLibrary::جستجو_For_Address_Of_نماد(نماد_Name_2);
}

تهی LLVMAdd_نماد(ثابت نویسه *نماد_Name_2, تهی *نماد_مقدار_2) {
  بازگشت llvm::sys::DynamicLibrary::Add_نماد(نماد_Name_2, نماد_مقدار_2);
}
