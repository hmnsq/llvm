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

استفاده نطاق llvm;
استفاده نطاق llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
صنف DynamicLibrary::Handle_مجموعه {
  استفاده HandleList = std::وکتور<فراغ *>;
  HandleList Handles;
  فراغ *Process = &Invalid;

عام:
  ساكن فراغ *DLOpen(ثابت حرف *Filename, std::string *Err);
  ساكن فراغ DLClose(فراغ *Handle);
  ساكن فراغ *DLSym(فراغ *Handle, ثابت حرف *نماد);

  Handle_مجموعه() = افتراضي;
  ~Handle_مجموعه();

  HandleList::iterator پیدا_کن_2(فراغ *Handle) { أرجع پیدا_کن(Handles, Handle); }

  منطقي شامل_است_2(فراغ *Handle) {
    أرجع Handle == Process || پیدا_کن_2(Handle) != Handles.انتها();
  }

  منطقي جمع_کن_Library(فراغ *Handle, منطقي IsProcess = خطأ, منطقي CanClose = درست,
                  منطقي AllowDuplicates = خطأ) {
#اگر_تعریف_شده _WIN32
    أكد((Handle == هذا ? IsProcess : !IsProcess) && "Bad Handle.");
#پایان_اگر
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    إذا (LLVM_LIKELY(!IsProcess)) {
      إذا (!AllowDuplicates && پیدا_کن_2(Handle) != Handles.انتها()) {
        إذا (CanClose)
          DLClose(Handle);
        أرجع خطأ;
      }
      Handles.افزودن_پشت(Handle);
    } وإلا {
#اگر_تعریف_نشده _WIN32
      إذا (Process != &Invalid) {
        إذا (CanClose)
          DLClose(Process);
        إذا (Process == Handle)
          أرجع خطأ;
      }
#پایان_اگر
      Process = Handle;
    }
    أرجع درست;
  }

  فراغ CloseLibrary(فراغ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = پیدا_کن_2(Handle);
    إذا (it != Handles.انتها()) {
      Handles.erase(it);
    }
  }

  فراغ *LibLookup(ثابت حرف *نماد, DynamicLibrary::جستجو_Ordering ترتیب) {
    إذا (ترتیب & SO_بارگیری_ترتیب) {
      لكل (فراغ *Handle : Handles) {
        إذا (فراغ *Ptr = DLSym(Handle, نماد))
          أرجع Ptr;
      }
    } وإلا {
      لكل (فراغ *Handle : llvm::معکوس(Handles)) {
        إذا (فراغ *Ptr = DLSym(Handle, نماد))
          أرجع Ptr;
      }
    }
    أرجع اشاره_تهی;
  }

  فراغ *Lookup(ثابت حرف *نماد, DynamicLibrary::جستجو_Ordering ترتیب) {
    أكد(!((ترتیب & SO_Loaded_اولی) && (ترتیب & SO_Loaded_آخرین)) &&
           "Invalid Ordering");

    إذا (Process == &Invalid || (ترتیب & SO_Loaded_اولی)) {
      إذا (فراغ *Ptr = LibLookup(نماد, ترتیب))
        أرجع Ptr;
    }
    إذا (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      إذا (فراغ *Ptr = DLSym(Process, نماد))
        أرجع Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      إذا (ترتیب & SO_Loaded_آخرین) {
        إذا (فراغ *Ptr = LibLookup(نماد, ترتیب))
          أرجع Ptr;
      }
    }
    أرجع اشاره_تهی;
  }
};

نطاق {

بنية Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<فراغ *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::Handle_مجموعه OpenedHandles;
  DynamicLibrary::Handle_مجموعه OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<درست> SymbolsMutex;
};

Globals &getGlobals() {
  ساكن Globals G;
  أرجع G;
}

} // namespace

#اگر_تعریف_شده _WIN32

#ضمن "Windows/DynamicLibrary.inc"

#وإلا

#ضمن "Unix/DynamicLibrary.inc"

#پایان_اگر

حرف DynamicLibrary::Invalid;
DynamicLibrary::جستجو_Ordering DynamicLibrary::جستجو_ترتیب =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *جستجو_For_آدرس_از_Special_نماد(ثابت حرف *نماد_Name) {
  أرجع Do_جستجو(نماد_Name); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::جمع_کن_نماد(StringRef نماد_Name, فراغ *نماد_مقدار) {
  خودکار &G = getGlobals();
  Smart_Scoped_قفل<درست> قفل_3(G.SymbolsMutex);
  G.ExplicitSymbols[نماد_Name] = نماد_مقدار;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  خودکار &G = getGlobals();
  فراغ *Handle = Handle_مجموعه::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    Smart_Scoped_قفل<درست> قفل_3(G.SymbolsMutex);
    G.OpenedHandles.جمع_کن_Library(Handle, /*IsProcess*/ FileName == اشاره_تهی);
  }

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::جمع_کن_Permanent_Library(فراغ *Handle,
                                                   std::string *Err) {
  خودکار &G = getGlobals();
  Smart_Scoped_قفل<درست> قفل_3(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  إذا (!G.OpenedHandles.جمع_کن_Library(Handle, /*IsProcess*/ خطأ,
                                  /*CanClose*/ خطأ))
    *Err = "Library already loaded";

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت حرف *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  فراغ *Handle = Handle_مجموعه::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    خودکار &G = getGlobals();
    Smart_Scoped_قفل<درست> قفل_3(G.SymbolsMutex);
    G.OpenedTemporaryHandles.جمع_کن_Library(Handle, /*IsProcess*/ خطأ,
                                        /*CanClose*/ خطأ,
                                        /*AllowDuplicates*/ درست);
  }
  أرجع DynamicLibrary(Handle);
}

فراغ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  خودکار &G = getGlobals();
  Smart_Scoped_قفل<درست> قفل_3(G.SymbolsMutex);
  إذا (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.داده_2);
    Lib.داده_2 = &Invalid;
  }
}

فراغ *DynamicLibrary::get_آدرس_از_نماد(ثابت حرف *نماد_Name) {
  إذا (!isValid())
    أرجع اشاره_تهی;
  أرجع Handle_مجموعه::DLSym(داده_2, نماد_Name);
}

فراغ *DynamicLibrary::جستجو_For_آدرس_از_نماد(ثابت حرف *نماد_Name) {
  {
    خودکار &G = getGlobals();
    Smart_Scoped_قفل<درست> قفل_3(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.پیدا_کن(نماد_Name);

    إذا (i != G.ExplicitSymbols.انتها())
      أرجع i->دومی_2;

    // Now search the libraries.
    إذا (فراغ *Ptr = G.OpenedHandles.Lookup(نماد_Name, جستجو_ترتیب))
      أرجع Ptr;
    إذا (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(نماد_Name, جستجو_ترتیب))
      أرجع Ptr;
  }

  أرجع llvm::جستجو_For_آدرس_از_Special_نماد(نماد_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  أرجع llvm::sys::DynamicLibrary::بارگیری_Library_Permanently(Filename);
}

فراغ *LLVMSearch_For_آدرس_از_نماد(ثابت حرف *نماد_Name_2) {
  أرجع llvm::sys::DynamicLibrary::جستجو_For_آدرس_از_نماد(نماد_Name_2);
}

فراغ LLVMAdd_نماد(ثابت حرف *نماد_Name_2, فراغ *نماد_مقدار_2) {
  أرجع llvm::sys::DynamicLibrary::جمع_کن_نماد(نماد_Name_2, نماد_مقدار_2);
}
