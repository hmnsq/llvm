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

استعمال نطاق llvm;
استعمال نطاق llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
صنف DynamicLibrary::Handle_سيٽ {
  استعمال HandleList = std::ويڪٽر<فراغ *>;
  HandleList Handles;
  فراغ *Process = &Invalid;

عام:
  ساكن فراغ *DLOpen(ثابت حرف *Filename, std::string *Err);
  ساكن فراغ DLClose(فراغ *Handle);
  ساكن فراغ *DLSym(فراغ *Handle, ثابت حرف *نشان);

  Handle_سيٽ() = افتراضي;
  ~Handle_سيٽ();

  HandleList::iterator ڳوليو_2(فراغ *Handle) { أرجع ڳوليو(Handles, Handle); }

  منطقي شامل_آهي_2(فراغ *Handle) {
    أرجع Handle == Process || ڳوليو_2(Handle) != Handles.پڇاڙي();
  }

  منطقي جوڙيو_Library(فراغ *Handle, منطقي IsProcess = خطأ, منطقي CanClose = سچ,
                  منطقي AllowDuplicates = خطأ) {
#جيڪڏهن_وصف_ٿيل _WIN32
    أكد((Handle == هذا ? IsProcess : !IsProcess) && "Bad Handle.");
#جيڪڏهن_پورو
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    إذا (LLVM_LIKELY(!IsProcess)) {
      إذا (!AllowDuplicates && ڳوليو_2(Handle) != Handles.پڇاڙي()) {
        إذا (CanClose)
          DLClose(Handle);
        أرجع خطأ;
      }
      Handles.ڌڪو_پٺ(Handle);
    } وإلا {
#جيڪڏهن_وصف_نه_ٿيل _WIN32
      إذا (Process != &Invalid) {
        إذا (CanClose)
          DLClose(Process);
        إذا (Process == Handle)
          أرجع خطأ;
      }
#جيڪڏهن_پورو
      Process = Handle;
    }
    أرجع سچ;
  }

  فراغ CloseLibrary(فراغ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = ڳوليو_2(Handle);
    إذا (it != Handles.پڇاڙي()) {
      Handles.erase(it);
    }
  }

  فراغ *LibLookup(ثابت حرف *نشان, DynamicLibrary::ڳولا_Ordering ترتيب) {
    إذا (ترتيب & SO_ڀريو_ترتيب) {
      لكل (فراغ *Handle : Handles) {
        إذا (فراغ *Ptr = DLSym(Handle, نشان))
          أرجع Ptr;
      }
    } وإلا {
      لكل (فراغ *Handle : llvm::ابتو(Handles)) {
        إذا (فراغ *Ptr = DLSym(Handle, نشان))
          أرجع Ptr;
      }
    }
    أرجع خالي_اشارو;
  }

  فراغ *Lookup(ثابت حرف *نشان, DynamicLibrary::ڳولا_Ordering ترتيب) {
    أكد(!((ترتيب & SO_Loaded_پهريون) && (ترتيب & SO_Loaded_آخري)) &&
           "Invalid Ordering");

    إذا (Process == &Invalid || (ترتيب & SO_Loaded_پهريون)) {
      إذا (فراغ *Ptr = LibLookup(نشان, ترتيب))
        أرجع Ptr;
    }
    إذا (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      إذا (فراغ *Ptr = DLSym(Process, نشان))
        أرجع Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      إذا (ترتيب & SO_Loaded_آخري) {
        إذا (فراغ *Ptr = LibLookup(نشان, ترتيب))
          أرجع Ptr;
      }
    }
    أرجع خالي_اشارو;
  }
};

نطاق {

بنية Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<فراغ *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::Handle_سيٽ OpenedHandles;
  DynamicLibrary::Handle_سيٽ OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<سچ> SymbolsMutex;
};

Globals &getGlobals() {
  ساكن Globals G;
  أرجع G;
}

} // namespace

#جيڪڏهن_وصف_ٿيل _WIN32

#ضمن "Windows/DynamicLibrary.inc"

#وإلا

#ضمن "Unix/DynamicLibrary.inc"

#جيڪڏهن_پورو

حرف DynamicLibrary::Invalid;
DynamicLibrary::ڳولا_Ordering DynamicLibrary::ڳولا_ترتيب =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *ڳولا_For_پتو_جو_Special_نشان(ثابت حرف *نشان_Name) {
  أرجع Do_ڳولا(نشان_Name); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::جوڙيو_نشان(StringRef نشان_Name, فراغ *نشان_قدر) {
  خودڪار &G = getGlobals();
  Smart_Scoped_تالو<سچ> تالو_2(G.SymbolsMutex);
  G.ExplicitSymbols[نشان_Name] = نشان_قدر;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  خودڪار &G = getGlobals();
  فراغ *Handle = Handle_سيٽ::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    Smart_Scoped_تالو<سچ> تالو_2(G.SymbolsMutex);
    G.OpenedHandles.جوڙيو_Library(Handle, /*IsProcess*/ FileName == خالي_اشارو);
  }

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::جوڙيو_Permanent_Library(فراغ *Handle,
                                                   std::string *Err) {
  خودڪار &G = getGlobals();
  Smart_Scoped_تالو<سچ> تالو_2(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  إذا (!G.OpenedHandles.جوڙيو_Library(Handle, /*IsProcess*/ خطأ,
                                  /*CanClose*/ خطأ))
    *Err = "Library already loaded";

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت حرف *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  فراغ *Handle = Handle_سيٽ::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    خودڪار &G = getGlobals();
    Smart_Scoped_تالو<سچ> تالو_2(G.SymbolsMutex);
    G.OpenedTemporaryHandles.جوڙيو_Library(Handle, /*IsProcess*/ خطأ,
                                        /*CanClose*/ خطأ,
                                        /*AllowDuplicates*/ سچ);
  }
  أرجع DynamicLibrary(Handle);
}

فراغ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  خودڪار &G = getGlobals();
  Smart_Scoped_تالو<سچ> تالو_2(G.SymbolsMutex);
  إذا (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.معلومات_2);
    Lib.معلومات_2 = &Invalid;
  }
}

فراغ *DynamicLibrary::get_پتو_جو_نشان(ثابت حرف *نشان_Name) {
  إذا (!isValid())
    أرجع خالي_اشارو;
  أرجع Handle_سيٽ::DLSym(معلومات_2, نشان_Name);
}

فراغ *DynamicLibrary::ڳولا_For_پتو_جو_نشان(ثابت حرف *نشان_Name) {
  {
    خودڪار &G = getGlobals();
    Smart_Scoped_تالو<سچ> تالو_2(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.ڳوليو(نشان_Name);

    إذا (i != G.ExplicitSymbols.پڇاڙي())
      أرجع i->ٻيو_2;

    // Now search the libraries.
    إذا (فراغ *Ptr = G.OpenedHandles.Lookup(نشان_Name, ڳولا_ترتيب))
      أرجع Ptr;
    إذا (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(نشان_Name, ڳولا_ترتيب))
      أرجع Ptr;
  }

  أرجع llvm::ڳولا_For_پتو_جو_Special_نشان(نشان_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  أرجع llvm::sys::DynamicLibrary::ڀريو_Library_Permanently(Filename);
}

فراغ *LLVMSearch_For_پتو_جو_نشان(ثابت حرف *نشان_Name_2) {
  أرجع llvm::sys::DynamicLibrary::ڳولا_For_پتو_جو_نشان(نشان_Name_2);
}

فراغ LLVMAdd_نشان(ثابت حرف *نشان_Name_2, فراغ *نشان_قدر_2) {
  أرجع llvm::sys::DynamicLibrary::جوڙيو_نشان(نشان_Name_2, نشان_قدر_2);
}
