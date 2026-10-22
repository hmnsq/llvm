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

#অন্তর্ভাৱ "llvm/Support/DynamicLibrary.h"
#অন্তর্ভাৱ "llvm-c/Support.h"
#অন্তর্ভাৱ "llvm/ADT/STLExtras.h"
#অন্তর্ভাৱ "llvm/ADT/StringMap.h"
#অন্তর্ভাৱ "llvm/Config/config.h"
#অন্তর্ভাৱ "llvm/Support/Mutex.h"
#অন্তর্ভাৱ <vector>

using নামাকাশ llvm;
using নামাকাশ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
শ্ৰেণী DynamicLibrary::HandleSet {
  using HandleList = std::vector<শূন্য *>;
  HandleList Handles;
  শূন্য *Process = &Invalid;

সার্ৱজনিক:
  স্থির শূন্য *DLOpen(ধ্রুৱ ৱর্ণ *Filename, std::string *Err);
  স্থির শূন্য DLClose(শূন্য *Handle);
  স্থির শূন্য *DLSym(শূন্য *Handle, ধ্রুৱ ৱর্ণ *প্ৰতীক);

  HandleSet() = স্ৱভাৱ;
  ~HandleSet();

  HandleList::iterator Find(শূন্য *Handle) { ঘূৰাই find(Handles, Handle); }

  তার্কিক Contains(শূন্য *Handle) {
    ঘূৰাই Handle == Process || Find(Handle) != Handles.end();
  }

  তার্কিক AddLibrary(শূন্য *Handle, তার্কিক IsProcess = মিছা, তার্কিক CanClose = সঁচা,
                  তার্কিক AllowDuplicates = মিছা) {
#ifdef _WIN32
    প্রতিজ্ঞা((Handle == এতত্ ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    প্রতিজ্ঞা((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    যদি (LLVM_LIKELY(!IsProcess)) {
      যদি (!AllowDuplicates && Find(Handle) != Handles.end()) {
        যদি (CanClose)
          DLClose(Handle);
        ঘূৰাই মিছা;
      }
      Handles.ঠেলো_back(Handle);
    } অন্যথা {
#ifndef _WIN32
      যদি (Process != &Invalid) {
        যদি (CanClose)
          DLClose(Process);
        যদি (Process == Handle)
          ঘূৰাই মিছা;
      }
#endif
      Process = Handle;
    }
    ঘূৰাই সঁচা;
  }

  শূন্য CloseLibrary(শূন্য *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    যদি (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  শূন্য *LibLookup(ধ্রুৱ ৱর্ণ *প্ৰতীক, DynamicLibrary::অনুসন্ধান_Ordering Order) {
    যদি (Order & SO_LoadOrder) {
      বাবে (শূন্য *Handle : Handles) {
        যদি (শূন্য *Ptr = DLSym(Handle, প্ৰতীক))
          ঘূৰাই Ptr;
      }
    } অন্যথা {
      বাবে (শূন্য *Handle : llvm::reverse(Handles)) {
        যদি (শূন্য *Ptr = DLSym(Handle, প্ৰতীক))
          ঘূৰাই Ptr;
      }
    }
    ঘূৰাই nullptr;
  }

  শূন্য *Lookup(ধ্রুৱ ৱর্ণ *প্ৰতীক, DynamicLibrary::অনুসন্ধান_Ordering Order) {
    প্রতিজ্ঞা(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    যদি (Process == &Invalid || (Order & SO_LoadedFirst)) {
      যদি (শূন্য *Ptr = LibLookup(প্ৰতীক, Order))
        ঘূৰাই Ptr;
    }
    যদি (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      যদি (শূন্য *Ptr = DLSym(Process, প্ৰতীক))
        ঘূৰাই Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      যদি (Order & SO_LoadedLast) {
        যদি (শূন্য *Ptr = LibLookup(প্ৰতীক, Order))
          ঘূৰাই Ptr;
      }
    }
    ঘূৰাই nullptr;
  }
};

নামাকাশ {

সংরচনা Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<শূন্য *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<সঁচা> SymbolsMutex;
};

Globals &getGlobals() {
  স্থির Globals G;
  ঘূৰাই G;
}

} // namespace

#ifdef _WIN32

#অন্তর্ভাৱ "Windows/DynamicLibrary.inc"

#অন্যথা

#অন্তর্ভাৱ "Unix/DynamicLibrary.inc"

#endif

ৱর্ণ DynamicLibrary::Invalid;
DynamicLibrary::অনুসন্ধান_Ordering DynamicLibrary::অনুসন্ধান_Order =
    DynamicLibrary::SO_Linker;

নামাকাশ llvm {
শূন্য *অনুসন্ধান_For_Address_Of_Special_প্ৰতীক(ধ্রুৱ ৱর্ণ *প্ৰতীক_Name) {
  ঘূৰাই Do_অনুসন্ধান(প্ৰতীক_Name); // DynamicLibrary.inc
}
} // namespace llvm

শূন্য DynamicLibrary::Add_প্ৰতীক(StringRef প্ৰতীক_Name, শূন্য *প্ৰতীক_মান) {
  auto &G = getGlobals();
  SmartScopedLock<সঁচা> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[প্ৰতীক_Name] = প্ৰতীক_মান;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ধ্রুৱ ৱর্ণ *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  শূন্য *Handle = HandleSet::DLOpen(FileName, Err);
  যদি (Handle != &Invalid) {
    SmartScopedLock<সঁচা> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ঘূৰাই DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(শূন্য *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<সঁচা> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  যদি (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ মিছা,
                                  /*CanClose*/ মিছা))
    *Err = "Library already loaded";

  ঘূৰাই DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ধ্রুৱ ৱর্ণ *FileName,
                                          std::string *Err) {
  প্রতিজ্ঞা(FileName && "Use getPermanentLibrary() for opening process handle");
  শূন্য *Handle = HandleSet::DLOpen(FileName, Err);
  যদি (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<সঁচা> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ মিছা,
                                        /*CanClose*/ মিছা,
                                        /*AllowDuplicates*/ সঁচা);
  }
  ঘূৰাই DynamicLibrary(Handle);
}

শূন্য DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<সঁচা> Lock(G.SymbolsMutex);
  যদি (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

শূন্য *DynamicLibrary::get_Address_Of_প্ৰতীক(ধ্রুৱ ৱর্ণ *প্ৰতীক_Name) {
  যদি (!isValid())
    ঘূৰাই nullptr;
  ঘূৰাই HandleSet::DLSym(Data, প্ৰতীক_Name);
}

শূন্য *DynamicLibrary::অনুসন্ধান_For_Address_Of_প্ৰতীক(ধ্রুৱ ৱর্ণ *প্ৰতীক_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<সঁচা> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<শূন্য *>::iterator i = G.ExplicitSymbols.find(প্ৰতীক_Name);

    যদি (i != G.ExplicitSymbols.end())
      ঘূৰাই i->second;

    // Now search the libraries.
    যদি (শূন্য *Ptr = G.OpenedHandles.Lookup(প্ৰতীক_Name, অনুসন্ধান_Order))
      ঘূৰাই Ptr;
    যদি (শূন্য *Ptr = G.OpenedTemporaryHandles.Lookup(প্ৰতীক_Name, অনুসন্ধান_Order))
      ঘূৰাই Ptr;
  }

  ঘূৰাই llvm::অনুসন্ধান_For_Address_Of_Special_প্ৰতীক(প্ৰতীক_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ধ্রুৱ ৱর্ণ *Filename) {
  ঘূৰাই llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

শূন্য *LLVMSearch_For_Address_Of_প্ৰতীক(ধ্রুৱ ৱর্ণ *প্ৰতীক_Name_2) {
  ঘূৰাই llvm::sys::DynamicLibrary::অনুসন্ধান_For_Address_Of_প্ৰতীক(প্ৰতীক_Name_2);
}

শূন্য LLVMAdd_প্ৰতীক(ধ্রুৱ ৱর্ণ *প্ৰতীক_Name_2, শূন্য *প্ৰতীক_মান_2) {
  ঘূৰাই llvm::sys::DynamicLibrary::Add_প্ৰতীক(প্ৰতীক_Name_2, প্ৰতীক_মান_2);
}
