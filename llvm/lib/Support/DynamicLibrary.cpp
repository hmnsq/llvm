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

ورتوں نطاق llvm;
ورتوں نطاق llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
صنف DynamicLibrary::Handle_سیٹ {
  ورتوں HandleList = std::سمتیہ<فراغ *>;
  HandleList Handles;
  فراغ *Process = &Invalid;

عام:
  ساكن فراغ *DLOpen(ثابت حرف *Filename, std::string *Err);
  ساكن فراغ DLClose(فراغ *Handle);
  ساكن فراغ *DLSym(فراغ *Handle, ثابت حرف *علامت);

  Handle_سیٹ() = افتراضي;
  ~Handle_سیٹ();

  HandleList::iterator لبھو_2(فراغ *Handle) { أرجع لبھو(Handles, Handle); }

  منطقي شامل_اے_2(فراغ *Handle) {
    أرجع Handle == Process || لبھو_2(Handle) != Handles.سرا();
  }

  منطقي جوڑو_Library(فراغ *Handle, منطقي IsProcess = خطأ, منطقي CanClose = سچ,
                  منطقي AllowDuplicates = خطأ) {
#جے_تعریف_شدہ _WIN32
    أكد((Handle == هذا ? IsProcess : !IsProcess) && "Bad Handle.");
#جے_انت
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    إذا (LLVM_LIKELY(!IsProcess)) {
      إذا (!AllowDuplicates && لبھو_2(Handle) != Handles.سرا()) {
        إذا (CanClose)
          DLClose(Handle);
        أرجع خطأ;
      }
      Handles.ڈالو_پٹھ(Handle);
    } وإلا {
#جے_بنا_تعریف _WIN32
      إذا (Process != &Invalid) {
        إذا (CanClose)
          DLClose(Process);
        إذا (Process == Handle)
          أرجع خطأ;
      }
#جے_انت
      Process = Handle;
    }
    أرجع سچ;
  }

  فراغ CloseLibrary(فراغ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = لبھو_2(Handle);
    إذا (it != Handles.سرا()) {
      Handles.erase(it);
    }
  }

  فراغ *LibLookup(ثابت حرف *علامت, DynamicLibrary::تلاش_Ordering ترتیب) {
    إذا (ترتیب & SO_لدو_ترتیب) {
      لكل (فراغ *Handle : Handles) {
        إذا (فراغ *Ptr = DLSym(Handle, علامت))
          أرجع Ptr;
      }
    } وإلا {
      لكل (فراغ *Handle : llvm::الٹ(Handles)) {
        إذا (فراغ *Ptr = DLSym(Handle, علامت))
          أرجع Ptr;
      }
    }
    أرجع خالی_سوچک;
  }

  فراغ *Lookup(ثابت حرف *علامت, DynamicLibrary::تلاش_Ordering ترتیب) {
    أكد(!((ترتیب & SO_Loaded_پہلا) && (ترتیب & SO_Loaded_آخری)) &&
           "Invalid Ordering");

    إذا (Process == &Invalid || (ترتیب & SO_Loaded_پہلا)) {
      إذا (فراغ *Ptr = LibLookup(علامت, ترتیب))
        أرجع Ptr;
    }
    إذا (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      إذا (فراغ *Ptr = DLSym(Process, علامت))
        أرجع Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      إذا (ترتیب & SO_Loaded_آخری) {
        إذا (فراغ *Ptr = LibLookup(علامت, ترتیب))
          أرجع Ptr;
      }
    }
    أرجع خالی_سوچک;
  }
};

نطاق {

بنية Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<فراغ *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::Handle_سیٹ OpenedHandles;
  DynamicLibrary::Handle_سیٹ OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<سچ> SymbolsMutex;
};

Globals &getGlobals() {
  ساكن Globals G;
  أرجع G;
}

} // namespace

#جے_تعریف_شدہ _WIN32

#ضمن "Windows/DynamicLibrary.inc"

#وإلا

#ضمن "Unix/DynamicLibrary.inc"

#جے_انت

حرف DynamicLibrary::Invalid;
DynamicLibrary::تلاش_Ordering DynamicLibrary::تلاش_ترتیب =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *تلاش_For_پتہ_دا_Special_علامت(ثابت حرف *علامت_Name) {
  أرجع Do_تلاش(علامت_Name); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::جوڑو_علامت(StringRef علامت_Name, فراغ *علامت_قدر) {
  خودکار &G = getGlobals();
  Smart_Scoped_جندرا<سچ> جندرا_2(G.SymbolsMutex);
  G.ExplicitSymbols[علامت_Name] = علامت_قدر;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  خودکار &G = getGlobals();
  فراغ *Handle = Handle_سیٹ::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    Smart_Scoped_جندرا<سچ> جندرا_2(G.SymbolsMutex);
    G.OpenedHandles.جوڑو_Library(Handle, /*IsProcess*/ FileName == خالی_سوچک);
  }

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::جوڑو_Permanent_Library(فراغ *Handle,
                                                   std::string *Err) {
  خودکار &G = getGlobals();
  Smart_Scoped_جندرا<سچ> جندرا_2(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  إذا (!G.OpenedHandles.جوڑو_Library(Handle, /*IsProcess*/ خطأ,
                                  /*CanClose*/ خطأ))
    *Err = "Library already loaded";

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت حرف *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  فراغ *Handle = Handle_سیٹ::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    خودکار &G = getGlobals();
    Smart_Scoped_جندرا<سچ> جندرا_2(G.SymbolsMutex);
    G.OpenedTemporaryHandles.جوڑو_Library(Handle, /*IsProcess*/ خطأ,
                                        /*CanClose*/ خطأ,
                                        /*AllowDuplicates*/ سچ);
  }
  أرجع DynamicLibrary(Handle);
}

فراغ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  خودکار &G = getGlobals();
  Smart_Scoped_جندرا<سچ> جندرا_2(G.SymbolsMutex);
  إذا (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.انکڑے_2);
    Lib.انکڑے_2 = &Invalid;
  }
}

فراغ *DynamicLibrary::get_پتہ_دا_علامت(ثابت حرف *علامت_Name) {
  إذا (!isValid())
    أرجع خالی_سوچک;
  أرجع Handle_سیٹ::DLSym(انکڑے_2, علامت_Name);
}

فراغ *DynamicLibrary::تلاش_For_پتہ_دا_علامت(ثابت حرف *علامت_Name) {
  {
    خودکار &G = getGlobals();
    Smart_Scoped_جندرا<سچ> جندرا_2(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.لبھو(علامت_Name);

    إذا (i != G.ExplicitSymbols.سرا())
      أرجع i->دوجا_2;

    // Now search the libraries.
    إذا (فراغ *Ptr = G.OpenedHandles.Lookup(علامت_Name, تلاش_ترتیب))
      أرجع Ptr;
    إذا (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(علامت_Name, تلاش_ترتیب))
      أرجع Ptr;
  }

  أرجع llvm::تلاش_For_پتہ_دا_Special_علامت(علامت_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  أرجع llvm::sys::DynamicLibrary::لدو_Library_Permanently(Filename);
}

فراغ *LLVMSearch_For_پتہ_دا_علامت(ثابت حرف *علامت_Name_2) {
  أرجع llvm::sys::DynamicLibrary::تلاش_For_پتہ_دا_علامت(علامت_Name_2);
}

فراغ LLVMAdd_علامت(ثابت حرف *علامت_Name_2, فراغ *علامت_قدر_2) {
  أرجع llvm::sys::DynamicLibrary::جوڑو_علامت(علامت_Name_2, علامت_قدر_2);
}
