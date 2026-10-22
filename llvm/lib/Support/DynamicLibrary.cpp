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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/DynamicLibrary.h"
#ଅନ୍ତର୍ଭାଵ "llvm-c/Support.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Mutex.h"
#ଅନ୍ତର୍ଭାଵ <vector>

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
ଶ୍ରେଣୀ DynamicLibrary::HandleSet {
  using HandleList = std::vector<ଶୂନ୍ୟ *>;
  HandleList Handles;
  ଶୂନ୍ୟ *Process = &Invalid;

ସର୍ବସାଧାରଣ:
  ସ୍ଥିର ଶୂନ୍ୟ *DLOpen(ଅଚଳ ଅକ୍ଷର *Filename, std::string *Err);
  ସ୍ଥିର ଶୂନ୍ୟ DLClose(ଶୂନ୍ୟ *Handle);
  ସ୍ଥିର ଶୂନ୍ୟ *DLSym(ଶୂନ୍ୟ *Handle, ଅଚଳ ଅକ୍ଷର *ପ୍ରତୀକ);

  HandleSet() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
  ~HandleSet();

  HandleList::iterator Find(ଶୂନ୍ୟ *Handle) { ଫେରନ୍ତୁ find(Handles, Handle); }

  ତାର୍କିକ Contains(ଶୂନ୍ୟ *Handle) {
    ଫେରନ୍ତୁ Handle == Process || Find(Handle) != Handles.end();
  }

  ତାର୍କିକ AddLibrary(ଶୂନ୍ୟ *Handle, ତାର୍କିକ IsProcess = ମିଥ୍ୟା, ତାର୍କିକ CanClose = ସତ୍ୟ,
                  ତାର୍କିକ AllowDuplicates = ମିଥ୍ୟା) {
#ifdef _WIN32
    ପ୍ରତିଜ୍ଞା((Handle == ଏହି ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    ପ୍ରତିଜ୍ଞା((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ଯଦି (LLVM_LIKELY(!IsProcess)) {
      ଯଦି (!AllowDuplicates && Find(Handle) != Handles.end()) {
        ଯଦି (CanClose)
          DLClose(Handle);
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      }
      Handles.ଠେଲ_back(Handle);
    } ଅନ୍ୟଥା {
#ifndef _WIN32
      ଯଦି (Process != &Invalid) {
        ଯଦି (CanClose)
          DLClose(Process);
        ଯଦି (Process == Handle)
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
      }
#endif
      Process = Handle;
    }
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ଶୂନ୍ୟ CloseLibrary(ଶୂନ୍ୟ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    ଯଦି (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  ଶୂନ୍ୟ *LibLookup(ଅଚଳ ଅକ୍ଷର *ପ୍ରତୀକ, DynamicLibrary::ସନ୍ଧାନ_Ordering Order) {
    ଯଦି (Order & SO_LoadOrder) {
      ପାଇଁ (ଶୂନ୍ୟ *Handle : Handles) {
        ଯଦି (ଶୂନ୍ୟ *Ptr = DLSym(Handle, ପ୍ରତୀକ))
          ଫେରନ୍ତୁ Ptr;
      }
    } ଅନ୍ୟଥା {
      ପାଇଁ (ଶୂନ୍ୟ *Handle : llvm::reverse(Handles)) {
        ଯଦି (ଶୂନ୍ୟ *Ptr = DLSym(Handle, ପ୍ରତୀକ))
          ଫେରନ୍ତୁ Ptr;
      }
    }
    ଫେରନ୍ତୁ nullptr;
  }

  ଶୂନ୍ୟ *Lookup(ଅଚଳ ଅକ୍ଷର *ପ୍ରତୀକ, DynamicLibrary::ସନ୍ଧାନ_Ordering Order) {
    ପ୍ରତିଜ୍ଞା(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    ଯଦି (Process == &Invalid || (Order & SO_LoadedFirst)) {
      ଯଦି (ଶୂନ୍ୟ *Ptr = LibLookup(ପ୍ରତୀକ, Order))
        ଫେରନ୍ତୁ Ptr;
    }
    ଯଦି (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ଯଦି (ଶୂନ୍ୟ *Ptr = DLSym(Process, ପ୍ରତୀକ))
        ଫେରନ୍ତୁ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ଯଦି (Order & SO_LoadedLast) {
        ଯଦି (ଶୂନ୍ୟ *Ptr = LibLookup(ପ୍ରତୀକ, Order))
          ଫେରନ୍ତୁ Ptr;
      }
    }
    ଫେରନ୍ତୁ nullptr;
  }
};

ନାମାକାଶ {

ସଂରଚନା Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<ଶୂନ୍ୟ *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<ସତ୍ୟ> SymbolsMutex;
};

Globals &getGlobals() {
  ସ୍ଥିର Globals G;
  ଫେରନ୍ତୁ G;
}

} // namespace

#ifdef _WIN32

#ଅନ୍ତର୍ଭାଵ "Windows/DynamicLibrary.inc"

#ଅନ୍ୟଥା

#ଅନ୍ତର୍ଭାଵ "Unix/DynamicLibrary.inc"

#endif

ଅକ୍ଷର DynamicLibrary::Invalid;
DynamicLibrary::ସନ୍ଧାନ_Ordering DynamicLibrary::ସନ୍ଧାନ_Order =
    DynamicLibrary::SO_Linker;

ନାମାକାଶ llvm {
ଶୂନ୍ୟ *ସନ୍ଧାନ_For_Address_Of_Special_ପ୍ରତୀକ(ଅଚଳ ଅକ୍ଷର *ପ୍ରତୀକ_Name) {
  ଫେରନ୍ତୁ Do_ସନ୍ଧାନ(ପ୍ରତୀକ_Name); // DynamicLibrary.inc
}
} // namespace llvm

ଶୂନ୍ୟ DynamicLibrary::Add_ପ୍ରତୀକ(StringRef ପ୍ରତୀକ_Name, ଶୂନ୍ୟ *ପ୍ରତୀକ_ମୂଲ୍ୟ) {
  auto &G = getGlobals();
  SmartScopedLock<ସତ୍ୟ> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[ପ୍ରତୀକ_Name] = ପ୍ରତୀକ_ମୂଲ୍ୟ;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ଅଚଳ ଅକ୍ଷର *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  ଶୂନ୍ୟ *Handle = HandleSet::DLOpen(FileName, Err);
  ଯଦି (Handle != &Invalid) {
    SmartScopedLock<ସତ୍ୟ> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  ଫେରନ୍ତୁ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(ଶୂନ୍ୟ *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<ସତ୍ୟ> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ଯଦି (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ ମିଥ୍ୟା,
                                  /*CanClose*/ ମିଥ୍ୟା))
    *Err = "Library already loaded";

  ଫେରନ୍ତୁ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ଅଚଳ ଅକ୍ଷର *FileName,
                                          std::string *Err) {
  ପ୍ରତିଜ୍ଞା(FileName && "Use getPermanentLibrary() for opening process handle");
  ଶୂନ୍ୟ *Handle = HandleSet::DLOpen(FileName, Err);
  ଯଦି (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<ସତ୍ୟ> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ ମିଥ୍ୟା,
                                        /*CanClose*/ ମିଥ୍ୟା,
                                        /*AllowDuplicates*/ ସତ୍ୟ);
  }
  ଫେରନ୍ତୁ DynamicLibrary(Handle);
}

ଶୂନ୍ୟ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<ସତ୍ୟ> Lock(G.SymbolsMutex);
  ଯଦି (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

ଶୂନ୍ୟ *DynamicLibrary::get_Address_Of_ପ୍ରତୀକ(ଅଚଳ ଅକ୍ଷର *ପ୍ରତୀକ_Name) {
  ଯଦି (!isValid())
    ଫେରନ୍ତୁ nullptr;
  ଫେରନ୍ତୁ HandleSet::DLSym(Data, ପ୍ରତୀକ_Name);
}

ଶୂନ୍ୟ *DynamicLibrary::ସନ୍ଧାନ_For_Address_Of_ପ୍ରତୀକ(ଅଚଳ ଅକ୍ଷର *ପ୍ରତୀକ_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<ସତ୍ୟ> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<ଶୂନ୍ୟ *>::iterator i = G.ExplicitSymbols.find(ପ୍ରତୀକ_Name);

    ଯଦି (i != G.ExplicitSymbols.end())
      ଫେରନ୍ତୁ i->second;

    // Now search the libraries.
    ଯଦି (ଶୂନ୍ୟ *Ptr = G.OpenedHandles.Lookup(ପ୍ରତୀକ_Name, ସନ୍ଧାନ_Order))
      ଫେରନ୍ତୁ Ptr;
    ଯଦି (ଶୂନ୍ୟ *Ptr = G.OpenedTemporaryHandles.Lookup(ପ୍ରତୀକ_Name, ସନ୍ଧାନ_Order))
      ଫେରନ୍ତୁ Ptr;
  }

  ଫେରନ୍ତୁ llvm::ସନ୍ଧାନ_For_Address_Of_Special_ପ୍ରତୀକ(ପ୍ରତୀକ_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ଅଚଳ ଅକ୍ଷର *Filename) {
  ଫେରନ୍ତୁ llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

ଶୂନ୍ୟ *LLVMSearch_For_Address_Of_ପ୍ରତୀକ(ଅଚଳ ଅକ୍ଷର *ପ୍ରତୀକ_Name_2) {
  ଫେରନ୍ତୁ llvm::sys::DynamicLibrary::ସନ୍ଧାନ_For_Address_Of_ପ୍ରତୀକ(ପ୍ରତୀକ_Name_2);
}

ଶୂନ୍ୟ LLVMAdd_ପ୍ରତୀକ(ଅଚଳ ଅକ୍ଷର *ପ୍ରତୀକ_Name_2, ଶୂନ୍ୟ *ପ୍ରତୀକ_ମୂଲ୍ୟ_2) {
  ଫେରନ୍ତୁ llvm::sys::DynamicLibrary::Add_ପ୍ରତୀକ(ପ୍ରତୀକ_Name_2, ପ୍ରତୀକ_ମୂଲ୍ୟ_2);
}
