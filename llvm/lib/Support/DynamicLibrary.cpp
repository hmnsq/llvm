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
বর্গ DynamicLibrary::HandleSet {
  using HandleList = std::vector<শূন্য *>;
  HandleList Handles;
  শূন্য *Process = &Invalid;

সার্বজনিক:
  স্থির শূন্য *DLOpen(ধ্রুব বর্ণ *Filename, std::string *Err);
  স্থির শূন্য DLClose(শূন্য *Handle);
  স্থির শূন্য *DLSym(শূন্য *Handle, ধ্রুব বর্ণ *প্রতীক);

  HandleSet() = স্বভাব;
  ~HandleSet();

  HandleList::iterator Find(শূন্য *Handle) { প্রত্যাগম find(Handles, Handle); }

  তার্কিক Contains(শূন্য *Handle) {
    প্রত্যাগম Handle == Process || Find(Handle) != Handles.end();
  }

  তার্কিক AddLibrary(শূন্য *Handle, তার্কিক IsProcess = অসত্য, তার্কিক CanClose = সত্য,
                  তার্কিক AllowDuplicates = অসত্য) {
#ifdef _WIN32
    প্রতিজ্ঞা((Handle == এতত্ ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    প্রতিজ্ঞা((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    যদি (LLVM_LIKELY(!IsProcess)) {
      যদি (!AllowDuplicates && Find(Handle) != Handles.end()) {
        যদি (CanClose)
          DLClose(Handle);
        প্রত্যাগম অসত্য;
      }
      Handles.ঠেলো_back(Handle);
    } অন্যথা {
#ifndef _WIN32
      যদি (Process != &Invalid) {
        যদি (CanClose)
          DLClose(Process);
        যদি (Process == Handle)
          প্রত্যাগম অসত্য;
      }
#endif
      Process = Handle;
    }
    প্রত্যাগম সত্য;
  }

  শূন্য CloseLibrary(শূন্য *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    যদি (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  শূন্য *LibLookup(ধ্রুব বর্ণ *প্রতীক, DynamicLibrary::অনুসন্ধান_Ordering Order) {
    যদি (Order & SO_LoadOrder) {
      প্রতি (শূন্য *Handle : Handles) {
        যদি (শূন্য *Ptr = DLSym(Handle, প্রতীক))
          প্রত্যাগম Ptr;
      }
    } অন্যথা {
      প্রতি (শূন্য *Handle : llvm::reverse(Handles)) {
        যদি (শূন্য *Ptr = DLSym(Handle, প্রতীক))
          প্রত্যাগম Ptr;
      }
    }
    প্রত্যাগম nullptr;
  }

  শূন্য *Lookup(ধ্রুব বর্ণ *প্রতীক, DynamicLibrary::অনুসন্ধান_Ordering Order) {
    প্রতিজ্ঞা(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    যদি (Process == &Invalid || (Order & SO_LoadedFirst)) {
      যদি (শূন্য *Ptr = LibLookup(প্রতীক, Order))
        প্রত্যাগম Ptr;
    }
    যদি (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      যদি (শূন্য *Ptr = DLSym(Process, প্রতীক))
        প্রত্যাগম Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      যদি (Order & SO_LoadedLast) {
        যদি (শূন্য *Ptr = LibLookup(প্রতীক, Order))
          প্রত্যাগম Ptr;
      }
    }
    প্রত্যাগম nullptr;
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
  llvm::sys::SmartMutex<সত্য> SymbolsMutex;
};

Globals &getGlobals() {
  স্থির Globals G;
  প্রত্যাগম G;
}

} // namespace

#ifdef _WIN32

#অন্তর্ভাব "Windows/DynamicLibrary.inc"

#অন্যথা

#অন্তর্ভাব "Unix/DynamicLibrary.inc"

#endif

বর্ণ DynamicLibrary::Invalid;
DynamicLibrary::অনুসন্ধান_Ordering DynamicLibrary::অনুসন্ধান_Order =
    DynamicLibrary::SO_Linker;

নামাকাশ llvm {
শূন্য *অনুসন্ধান_For_Address_Of_Special_প্রতীক(ধ্রুব বর্ণ *প্রতীক_Name) {
  প্রত্যাগম Do_অনুসন্ধান(প্রতীক_Name); // DynamicLibrary.inc
}
} // namespace llvm

শূন্য DynamicLibrary::Add_প্রতীক(StringRef প্রতীক_Name, শূন্য *প্রতীক_মান) {
  auto &G = getGlobals();
  SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[প্রতীক_Name] = প্রতীক_মান;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ধ্রুব বর্ণ *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  শূন্য *Handle = HandleSet::DLOpen(FileName, Err);
  যদি (Handle != &Invalid) {
    SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  প্রত্যাগম DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(শূন্য *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  যদি (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ অসত্য,
                                  /*CanClose*/ অসত্য))
    *Err = "Library already loaded";

  প্রত্যাগম DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ধ্রুব বর্ণ *FileName,
                                          std::string *Err) {
  প্রতিজ্ঞা(FileName && "Use getPermanentLibrary() for opening process handle");
  শূন্য *Handle = HandleSet::DLOpen(FileName, Err);
  যদি (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ অসত্য,
                                        /*CanClose*/ অসত্য,
                                        /*AllowDuplicates*/ সত্য);
  }
  প্রত্যাগম DynamicLibrary(Handle);
}

শূন্য DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<সত্য> Lock(G.SymbolsMutex);
  যদি (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

শূন্য *DynamicLibrary::get_Address_Of_প্রতীক(ধ্রুব বর্ণ *প্রতীক_Name) {
  যদি (!isValid())
    প্রত্যাগম nullptr;
  প্রত্যাগম HandleSet::DLSym(Data, প্রতীক_Name);
}

শূন্য *DynamicLibrary::অনুসন্ধান_For_Address_Of_প্রতীক(ধ্রুব বর্ণ *প্রতীক_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<সত্য> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<শূন্য *>::iterator i = G.ExplicitSymbols.find(প্রতীক_Name);

    যদি (i != G.ExplicitSymbols.end())
      প্রত্যাগম i->second;

    // Now search the libraries.
    যদি (শূন্য *Ptr = G.OpenedHandles.Lookup(প্রতীক_Name, অনুসন্ধান_Order))
      প্রত্যাগম Ptr;
    যদি (শূন্য *Ptr = G.OpenedTemporaryHandles.Lookup(প্রতীক_Name, অনুসন্ধান_Order))
      প্রত্যাগম Ptr;
  }

  প্রত্যাগম llvm::অনুসন্ধান_For_Address_Of_Special_প্রতীক(প্রতীক_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ধ্রুব বর্ণ *Filename) {
  প্রত্যাগম llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

শূন্য *LLVMSearch_For_Address_Of_প্রতীক(ধ্রুব বর্ণ *প্রতীক_Name_2) {
  প্রত্যাগম llvm::sys::DynamicLibrary::অনুসন্ধান_For_Address_Of_প্রতীক(প্রতীক_Name_2);
}

শূন্য LLVMAdd_প্রতীক(ধ্রুব বর্ণ *প্রতীক_Name_2, শূন্য *প্রতীক_মান_2) {
  প্রত্যাগম llvm::sys::DynamicLibrary::Add_প্রতীক(প্রতীক_Name_2, প্রতীক_মান_2);
}
