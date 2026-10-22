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

استعمال نامخلا llvm;
استعمال نامخلا llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
جماعت DynamicLibrary::Handle_سیٹ {
  استعمال HandleList = std::سمتیہ<خالی *>;
  HandleList Handles;
  خالی *Process = &Invalid;

عوامی:
  ساکن خالی *DLOpen(ثابت حرف *Filename, std::string *Err);
  ساکن خالی DLClose(خالی *Handle);
  ساکن خالی *DLSym(خالی *Handle, ثابت حرف *علامت);

  Handle_سیٹ() = طےشدہ;
  ~Handle_سیٹ();

  HandleList::iterator ڈھونڈو_2(خالی *Handle) { واپس ڈھونڈو(Handles, Handle); }

  منطقی شامل_ہے_2(خالی *Handle) {
    واپس Handle == Process || ڈھونڈو_2(Handle) != Handles.انتہا();
  }

  منطقی جوڑو_Library(خالی *Handle, منطقی IsProcess = جھوٹ, منطقی CanClose = سچ,
                  منطقی AllowDuplicates = جھوٹ) {
#اگر_تعریف_شدہ _WIN32
    أكد((Handle == یہ ? IsProcess : !IsProcess) && "Bad Handle.");
#اگر_ختم
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    اگر (LLVM_LIKELY(!IsProcess)) {
      اگر (!AllowDuplicates && ڈھونڈو_2(Handle) != Handles.انتہا()) {
        اگر (CanClose)
          DLClose(Handle);
        واپس جھوٹ;
      }
      Handles.ڈالو_عقب(Handle);
    } ورنہ {
#اگر_غیر_تعریف_شدہ _WIN32
      اگر (Process != &Invalid) {
        اگر (CanClose)
          DLClose(Process);
        اگر (Process == Handle)
          واپس جھوٹ;
      }
#اگر_ختم
      Process = Handle;
    }
    واپس سچ;
  }

  خالی CloseLibrary(خالی *Handle) {
    DLClose(Handle);
    HandleList::iterator it = ڈھونڈو_2(Handle);
    اگر (it != Handles.انتہا()) {
      Handles.erase(it);
    }
  }

  خالی *LibLookup(ثابت حرف *علامت, DynamicLibrary::تلاش_Ordering ترتیب) {
    اگر (ترتیب & SO_بھرو_ترتیب) {
      برائے (خالی *Handle : Handles) {
        اگر (خالی *Ptr = DLSym(Handle, علامت))
          واپس Ptr;
      }
    } ورنہ {
      برائے (خالی *Handle : llvm::الٹ(Handles)) {
        اگر (خالی *Ptr = DLSym(Handle, علامت))
          واپس Ptr;
      }
    }
    واپس خالی_اشاریہ;
  }

  خالی *Lookup(ثابت حرف *علامت, DynamicLibrary::تلاش_Ordering ترتیب) {
    أكد(!((ترتیب & SO_Loaded_پہلا) && (ترتیب & SO_Loaded_آخری)) &&
           "Invalid Ordering");

    اگر (Process == &Invalid || (ترتیب & SO_Loaded_پہلا)) {
      اگر (خالی *Ptr = LibLookup(علامت, ترتیب))
        واپس Ptr;
    }
    اگر (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      اگر (خالی *Ptr = DLSym(Process, علامت))
        واپس Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      اگر (ترتیب & SO_Loaded_آخری) {
        اگر (خالی *Ptr = LibLookup(علامت, ترتیب))
          واپس Ptr;
      }
    }
    واپس خالی_اشاریہ;
  }
};

نامخلا {

ساخت Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<خالی *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::Handle_سیٹ OpenedHandles;
  DynamicLibrary::Handle_سیٹ OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<سچ> SymbolsMutex;
};

Globals &getGlobals() {
  ساکن Globals G;
  واپس G;
}

} // namespace

#اگر_تعریف_شدہ _WIN32

#شامل "Windows/DynamicLibrary.inc"

#ورنہ

#شامل "Unix/DynamicLibrary.inc"

#اگر_ختم

حرف DynamicLibrary::Invalid;
DynamicLibrary::تلاش_Ordering DynamicLibrary::تلاش_ترتیب =
    DynamicLibrary::SO_Linker;

نامخلا llvm {
خالی *تلاش_For_پتہ_کا_Special_علامت(ثابت حرف *علامت_Name) {
  واپس Do_تلاش(علامت_Name); // DynamicLibrary.inc
}
} // namespace llvm

خالی DynamicLibrary::جوڑو_علامت(StringRef علامت_Name, خالی *علامت_قدر) {
  خودکار &G = getGlobals();
  Smart_Scoped_تالا<سچ> تالا_3(G.SymbolsMutex);
  G.ExplicitSymbols[علامت_Name] = علامت_قدر;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  خودکار &G = getGlobals();
  خالی *Handle = Handle_سیٹ::DLOpen(FileName, Err);
  اگر (Handle != &Invalid) {
    Smart_Scoped_تالا<سچ> تالا_3(G.SymbolsMutex);
    G.OpenedHandles.جوڑو_Library(Handle, /*IsProcess*/ FileName == خالی_اشاریہ);
  }

  واپس DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::جوڑو_Permanent_Library(خالی *Handle,
                                                   std::string *Err) {
  خودکار &G = getGlobals();
  Smart_Scoped_تالا<سچ> تالا_3(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  اگر (!G.OpenedHandles.جوڑو_Library(Handle, /*IsProcess*/ جھوٹ,
                                  /*CanClose*/ جھوٹ))
    *Err = "Library already loaded";

  واپس DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت حرف *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  خالی *Handle = Handle_سیٹ::DLOpen(FileName, Err);
  اگر (Handle != &Invalid) {
    خودکار &G = getGlobals();
    Smart_Scoped_تالا<سچ> تالا_3(G.SymbolsMutex);
    G.OpenedTemporaryHandles.جوڑو_Library(Handle, /*IsProcess*/ جھوٹ,
                                        /*CanClose*/ جھوٹ,
                                        /*AllowDuplicates*/ سچ);
  }
  واپس DynamicLibrary(Handle);
}

خالی DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  خودکار &G = getGlobals();
  Smart_Scoped_تالا<سچ> تالا_3(G.SymbolsMutex);
  اگر (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.ڈیٹا_2);
    Lib.ڈیٹا_2 = &Invalid;
  }
}

خالی *DynamicLibrary::get_پتہ_کا_علامت(ثابت حرف *علامت_Name) {
  اگر (!isValid())
    واپس خالی_اشاریہ;
  واپس Handle_سیٹ::DLSym(ڈیٹا_2, علامت_Name);
}

خالی *DynamicLibrary::تلاش_For_پتہ_کا_علامت(ثابت حرف *علامت_Name) {
  {
    خودکار &G = getGlobals();
    Smart_Scoped_تالا<سچ> تالا_3(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<خالی *>::iterator i = G.ExplicitSymbols.ڈھونڈو(علامت_Name);

    اگر (i != G.ExplicitSymbols.انتہا())
      واپس i->دوسرا_2;

    // Now search the libraries.
    اگر (خالی *Ptr = G.OpenedHandles.Lookup(علامت_Name, تلاش_ترتیب))
      واپس Ptr;
    اگر (خالی *Ptr = G.OpenedTemporaryHandles.Lookup(علامت_Name, تلاش_ترتیب))
      واپس Ptr;
  }

  واپس llvm::تلاش_For_پتہ_کا_Special_علامت(علامت_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  واپس llvm::sys::DynamicLibrary::بھرو_Library_Permanently(Filename);
}

خالی *LLVMSearch_For_پتہ_کا_علامت(ثابت حرف *علامت_Name_2) {
  واپس llvm::sys::DynamicLibrary::تلاش_For_پتہ_کا_علامت(علامت_Name_2);
}

خالی LLVMAdd_علامت(ثابت حرف *علامت_Name_2, خالی *علامت_قدر_2) {
  واپس llvm::sys::DynamicLibrary::جوڑو_علامت(علامت_Name_2, علامت_قدر_2);
}
