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

#অন্তর্ভাব "llvm/Support/DynamicLibrary.h"
#অন্তর্ভাব "llvm-c/Support.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/StringMap.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/Mutex.h"
#অন্তর্ভাব <vector>

using নামাকাশ llvm;
using নামাকাশ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
শ্রেণী DynamicLibrary::HandleSet {
  using HandleList = std::vector<শূন্য *>;
  HandleList Handles;
  শূন্য *Process = &Invalid;

সর্বজনীন:
  স্থির শূন্য *DLOpen(ধ্রুবক অক্ষর *Filename, std::string *Err);
  স্থির শূন্য DLClose(শূন্য *Handle);
  স্থির শূন্য *DLSym(শূন্য *Handle, ধ্রুবক অক্ষর *প্রতীক);

  HandleSet() = পূর্বনির্ধারিত;
  ~HandleSet();

  HandleList::iterator Find(শূন্য *Handle) { ফেরত find(Handles, Handle); }

  যৌক্তিক Contains(শূন্য *Handle) {
    ফেরত Handle == Process || Find(Handle) != Handles.end();
  }

  যৌক্তিক AddLibrary(শূন্য *Handle, যৌক্তিক IsProcess = মিথ্যা, যৌক্তিক CanClose = সত্য,
                  যৌক্তিক AllowDuplicates = মিথ্যা) {
#ifdef _WIN32
    প্রতিজ্ঞা((Handle == এই ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    প্রতিজ্ঞা((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    যদি (LLVM_LIKELY(!IsProcess)) {
      যদি (!AllowDuplicates && Find(Handle) != Handles.end()) {
        যদি (CanClose)
          DLClose(Handle);
        ফেরত মিথ্যা;
      }
      Handles.ঠেলো_back(Handle);
    } অন্যথা {
#ifndef _WIN32
      যদি (Process != &Invalid) {
        যদি (CanClose)
          DLClose(Process);
        যদি (Process == Handle)
          ফেরত মিথ্যা;
      }
#endif
      Process = Handle;
    }
    ফেরত সত্য;
  }

  শূন্য CloseLibrary(শূন্য *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    যদি (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  শূন্য *LibLookup(ধ্রুবক অক্ষর *প্রতীক, DynamicLibrary::অনুসন্ধান_Ordering Order) {
    যদি (Order & SO_LoadOrder) {
      জন্য (শূন্য *Handle : Handles) {
        যদি (শূন্য *Ptr = DLSym(Handle, প্রতীক))
          ফেরত Ptr;
      }
    } অন্যথা {
      জন্য (শূন্য *Handle : llvm::reverse(Handles)) {
        যদি (শূন্য *Ptr = DLSym(Handle, প্রতীক))
          ফেরত Ptr;
      }
    }
    ফেরত nullptr;
  }

  শূন্য *Lookup(ধ্রুবক অক্ষর *প্রতীক, DynamicLibrary::অনুসন্ধান_Ordering Order) {
    প্রতিজ্ঞা(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    যদি (Process == &Invalid || (Order & SO_LoadedFirst)) {
      যদি (শূন্য *Ptr = LibLookup(প্রতীক, Order))
        ফেরত Ptr;
    }
    যদি (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      যদি (শূন্য *Ptr = DLSym(Process, প্রতীক))
        ফেরত Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      যদি (Order & SO_LoadedLast) {
        যদি (শূন্য *Ptr = LibLookup(প্রতীক, Order))
          ফেরত Ptr;
      }
    }
    ফেরত nullptr;
  }
};

নামাকাশ {

কাঠামো Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<শূন্য *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<সত্য> SymbolsMutex;
};

Globals &getGlobals() {
  স্থির Globals G;
  ফেরত G;
}

} // namespace

#ifdef _WIN32

#অন্তর্ভাব "Windows/DynamicLibrary.inc"

#অন্যথা

#অন্তর্ভাব "Unix/DynamicLibrary.inc"

#endif

অক্ষর DynamicLibrary::Invalid;
DynamicLibrary::অনুসন্ধান_Ordering DynamicLibrary::অনুসন্ধান_Order =
    DynamicLibrary::SO_Linker;

নামাকাশ llvm {
শূন্য *অনুসন্ধান_For_Address_Of_Special_প্রতীক(ধ্রুবক অক্ষর *প্রতীক_Name) {
  ফেরত Do_অনুসন্ধান(প্রতীক_Name); // DynamicLibrary.inc
}
} // namespace llvm

শূন্য DynamicLibrary::Add_প্রতীক(StringRef প্রতীক_Name, শূন্য *প্রতীক_মান) {
  auto &G = getGlobals();
  SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[প্রতীক_Name] = প্রতীক_মান;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ধ্রুবক অক্ষর *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  শূন্য *Handle = HandleSet::DLOpen(FileName, Err);
  যদি (Handle != &Invalid) {
    SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ফেরত DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(শূন্য *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  যদি (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ মিথ্যা,
                                  /*CanClose*/ মিথ্যা))
    *Err = "Library already loaded";

  ফেরত DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ধ্রুবক অক্ষর *FileName,
                                          std::string *Err) {
  প্রতিজ্ঞা(FileName && "Use getPermanentLibrary() for opening process handle");
  শূন্য *Handle = HandleSet::DLOpen(FileName, Err);
  যদি (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ মিথ্যা,
                                        /*CanClose*/ মিথ্যা,
                                        /*AllowDuplicates*/ সত্য);
  }
  ফেরত DynamicLibrary(Handle);
}

শূন্য DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
  যদি (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.তথ্য_2);
    Lib.তথ্য_2 = &Invalid;
  }
}

শূন্য *DynamicLibrary::get_Address_Of_প্রতীক(ধ্রুবক অক্ষর *প্রতীক_Name) {
  যদি (!isValid())
    ফেরত nullptr;
  ফেরত HandleSet::DLSym(তথ্য_2, প্রতীক_Name);
}

শূন্য *DynamicLibrary::অনুসন্ধান_For_Address_Of_প্রতীক(ধ্রুবক অক্ষর *প্রতীক_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<সত্য> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<শূন্য *>::iterator i = G.ExplicitSymbols.find(প্রতীক_Name);

    যদি (i != G.ExplicitSymbols.end())
      ফেরত i->second;

    // Now search the libraries.
    যদি (শূন্য *Ptr = G.OpenedHandles.Lookup(প্রতীক_Name, অনুসন্ধান_Order))
      ফেরত Ptr;
    যদি (শূন্য *Ptr = G.OpenedTemporaryHandles.Lookup(প্রতীক_Name, অনুসন্ধান_Order))
      ফেরত Ptr;
  }

  ফেরত llvm::অনুসন্ধান_For_Address_Of_Special_প্রতীক(প্রতীক_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ধ্রুবক অক্ষর *Filename) {
  ফেরত llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

শূন্য *LLVMSearch_For_Address_Of_প্রতীক(ধ্রুবক অক্ষর *প্রতীক_Name_2) {
  ফেরত llvm::sys::DynamicLibrary::অনুসন্ধান_For_Address_Of_প্রতীক(প্রতীক_Name_2);
}

শূন্য LLVMAdd_প্রতীক(ধ্রুবক অক্ষর *প্রতীক_Name_2, শূন্য *প্রতীক_মান_2) {
  ফেরত llvm::sys::DynamicLibrary::Add_প্রতীক(প্রতীক_Name_2, প্রতীক_মান_2);
}
