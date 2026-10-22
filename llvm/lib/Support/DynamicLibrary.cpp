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

کارول نطاق llvm;
کارول نطاق llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
صنف DynamicLibrary::Handle_سېټ {
  کارول HandleList = std::ویکټور<فراغ *>;
  HandleList Handles;
  فراغ *Process = &Invalid;

عام:
  ساكن فراغ *DLOpen(ثابت حرف *Filename, std::string *Err);
  ساكن فراغ DLClose(فراغ *Handle);
  ساكن فراغ *DLSym(فراغ *Handle, ثابت حرف *نښه);

  Handle_سېټ() = افتراضي;
  ~Handle_سېټ();

  HandleList::iterator پیدا_کړه_2(فراغ *Handle) { أرجع پیدا_کړه(Handles, Handle); }

  منطقي لري_2(فراغ *Handle) {
    أرجع Handle == Process || پیدا_کړه_2(Handle) != Handles.پای_ټکی();
  }

  منطقي جمع_کړه_Library(فراغ *Handle, منطقي IsProcess = خطأ, منطقي CanClose = ریښتیا,
                  منطقي AllowDuplicates = خطأ) {
#که_تعریف_شوی _WIN32
    أكد((Handle == هذا ? IsProcess : !IsProcess) && "Bad Handle.");
#د_که_پای
    أكد((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    إذا (LLVM_LIKELY(!IsProcess)) {
      إذا (!AllowDuplicates && پیدا_کړه_2(Handle) != Handles.پای_ټکی()) {
        إذا (CanClose)
          DLClose(Handle);
        أرجع خطأ;
      }
      Handles.ټېل_شا(Handle);
    } وإلا {
#که_نه_تعریف_شوی _WIN32
      إذا (Process != &Invalid) {
        إذا (CanClose)
          DLClose(Process);
        إذا (Process == Handle)
          أرجع خطأ;
      }
#د_که_پای
      Process = Handle;
    }
    أرجع ریښتیا;
  }

  فراغ CloseLibrary(فراغ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = پیدا_کړه_2(Handle);
    إذا (it != Handles.پای_ټکی()) {
      Handles.erase(it);
    }
  }

  فراغ *LibLookup(ثابت حرف *نښه, DynamicLibrary::پلټنه_Ordering ترتیب) {
    إذا (ترتیب & SO_بارول_ترتیب) {
      لكل (فراغ *Handle : Handles) {
        إذا (فراغ *Ptr = DLSym(Handle, نښه))
          أرجع Ptr;
      }
    } وإلا {
      لكل (فراغ *Handle : llvm::سرچپه(Handles)) {
        إذا (فراغ *Ptr = DLSym(Handle, نښه))
          أرجع Ptr;
      }
    }
    أرجع تش_اشاره;
  }

  فراغ *Lookup(ثابت حرف *نښه, DynamicLibrary::پلټنه_Ordering ترتیب) {
    أكد(!((ترتیب & SO_Loaded_لومړی) && (ترتیب & SO_Loaded_وروستی)) &&
           "Invalid Ordering");

    إذا (Process == &Invalid || (ترتیب & SO_Loaded_لومړی)) {
      إذا (فراغ *Ptr = LibLookup(نښه, ترتیب))
        أرجع Ptr;
    }
    إذا (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      إذا (فراغ *Ptr = DLSym(Process, نښه))
        أرجع Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      إذا (ترتیب & SO_Loaded_وروستی) {
        إذا (فراغ *Ptr = LibLookup(نښه, ترتیب))
          أرجع Ptr;
      }
    }
    أرجع تش_اشاره;
  }
};

نطاق {

بنية Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<فراغ *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::Handle_سېټ OpenedHandles;
  DynamicLibrary::Handle_سېټ OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<ریښتیا> SymbolsMutex;
};

Globals &getGlobals() {
  ساكن Globals G;
  أرجع G;
}

} // namespace

#که_تعریف_شوی _WIN32

#ضمن "Windows/DynamicLibrary.inc"

#وإلا

#ضمن "Unix/DynamicLibrary.inc"

#د_که_پای

حرف DynamicLibrary::Invalid;
DynamicLibrary::پلټنه_Ordering DynamicLibrary::پلټنه_ترتیب =
    DynamicLibrary::SO_Linker;

نطاق llvm {
فراغ *پلټنه_For_پته_د_Special_نښه(ثابت حرف *نښه_Name) {
  أرجع Do_پلټنه(نښه_Name); // DynamicLibrary.inc
}
} // namespace llvm

فراغ DynamicLibrary::جمع_کړه_نښه(StringRef نښه_Name, فراغ *نښه_ارزښت) {
  خپلکاری &G = getGlobals();
  Smart_Scoped_قلف<ریښتیا> قلف_2(G.SymbolsMutex);
  G.ExplicitSymbols[نښه_Name] = نښه_ارزښت;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ثابت حرف *FileName,
                                                   std::string *Err) {
  خپلکاری &G = getGlobals();
  فراغ *Handle = Handle_سېټ::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    Smart_Scoped_قلف<ریښتیا> قلف_2(G.SymbolsMutex);
    G.OpenedHandles.جمع_کړه_Library(Handle, /*IsProcess*/ FileName == تش_اشاره);
  }

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::جمع_کړه_Permanent_Library(فراغ *Handle,
                                                   std::string *Err) {
  خپلکاری &G = getGlobals();
  Smart_Scoped_قلف<ریښتیا> قلف_2(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  إذا (!G.OpenedHandles.جمع_کړه_Library(Handle, /*IsProcess*/ خطأ,
                                  /*CanClose*/ خطأ))
    *Err = "Library already loaded";

  أرجع DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ثابت حرف *FileName,
                                          std::string *Err) {
  أكد(FileName && "Use getPermanentLibrary() for opening process handle");
  فراغ *Handle = Handle_سېټ::DLOpen(FileName, Err);
  إذا (Handle != &Invalid) {
    خپلکاری &G = getGlobals();
    Smart_Scoped_قلف<ریښتیا> قلف_2(G.SymbolsMutex);
    G.OpenedTemporaryHandles.جمع_کړه_Library(Handle, /*IsProcess*/ خطأ,
                                        /*CanClose*/ خطأ,
                                        /*AllowDuplicates*/ ریښتیا);
  }
  أرجع DynamicLibrary(Handle);
}

فراغ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  خپلکاری &G = getGlobals();
  Smart_Scoped_قلف<ریښتیا> قلف_2(G.SymbolsMutex);
  إذا (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.معلومات_2);
    Lib.معلومات_2 = &Invalid;
  }
}

فراغ *DynamicLibrary::get_پته_د_نښه(ثابت حرف *نښه_Name) {
  إذا (!isValid())
    أرجع تش_اشاره;
  أرجع Handle_سېټ::DLSym(معلومات_2, نښه_Name);
}

فراغ *DynamicLibrary::پلټنه_For_پته_د_نښه(ثابت حرف *نښه_Name) {
  {
    خپلکاری &G = getGlobals();
    Smart_Scoped_قلف<ریښتیا> قلف_2(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<فراغ *>::iterator i = G.ExplicitSymbols.پیدا_کړه(نښه_Name);

    إذا (i != G.ExplicitSymbols.پای_ټکی())
      أرجع i->دوهم_2;

    // Now search the libraries.
    إذا (فراغ *Ptr = G.OpenedHandles.Lookup(نښه_Name, پلټنه_ترتیب))
      أرجع Ptr;
    إذا (فراغ *Ptr = G.OpenedTemporaryHandles.Lookup(نښه_Name, پلټنه_ترتیب))
      أرجع Ptr;
  }

  أرجع llvm::پلټنه_For_پته_د_Special_نښه(نښه_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ثابت حرف *Filename) {
  أرجع llvm::sys::DynamicLibrary::بارول_Library_Permanently(Filename);
}

فراغ *LLVMSearch_For_پته_د_نښه(ثابت حرف *نښه_Name_2) {
  أرجع llvm::sys::DynamicLibrary::پلټنه_For_پته_د_نښه(نښه_Name_2);
}

فراغ LLVMAdd_نښه(ثابت حرف *نښه_Name_2, فراغ *نښه_ارزښت_2) {
  أرجع llvm::sys::DynamicLibrary::جمع_کړه_نښه(نښه_Name_2, نښه_ارزښت_2);
}
