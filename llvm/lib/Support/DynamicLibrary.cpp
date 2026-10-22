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

#அந்தர்பாவ "llvm/Support/DynamicLibrary.h"
#அந்தர்பாவ "llvm-c/Support.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/StringMap.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/Mutex.h"
#அந்தர்பாவ <vector>

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
வகுப்பு DynamicLibrary::HandleSet {
  using HandleList = std::vector<வெறுமை *>;
  HandleList Handles;
  வெறுமை *Process = &Invalid;

பொது:
  நிலையான வெறுமை *DLOpen(மாறிலி எழுத்து *Filename, std::string *Err);
  நிலையான வெறுமை DLClose(வெறுமை *Handle);
  நிலையான வெறுமை *DLSym(வெறுமை *Handle, மாறிலி எழுத்து *குறியீடு);

  HandleSet() = இயல்பு;
  ~HandleSet();

  HandleList::iterator Find(வெறுமை *Handle) { திரும்பு find(Handles, Handle); }

  தருக்க Contains(வெறுமை *Handle) {
    திரும்பு Handle == Process || Find(Handle) != Handles.end();
  }

  தருக்க AddLibrary(வெறுமை *Handle, தருக்க IsProcess = பொய், தருக்க CanClose = உண்மை,
                  தருக்க AllowDuplicates = பொய்) {
#ifdef _WIN32
    ப்ரதிஜ்ஞா((Handle == இது ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    ப்ரதிஜ்ஞா((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    எனில் (LLVM_LIKELY(!IsProcess)) {
      எனில் (!AllowDuplicates && Find(Handle) != Handles.end()) {
        எனில் (CanClose)
          DLClose(Handle);
        திரும்பு பொய்;
      }
      Handles.தள்ளு_back(Handle);
    } இல்லையெனில் {
#ifndef _WIN32
      எனில் (Process != &Invalid) {
        எனில் (CanClose)
          DLClose(Process);
        எனில் (Process == Handle)
          திரும்பு பொய்;
      }
#endif
      Process = Handle;
    }
    திரும்பு உண்மை;
  }

  வெறுமை CloseLibrary(வெறுமை *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    எனில் (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  வெறுமை *LibLookup(மாறிலி எழுத்து *குறியீடு, DynamicLibrary::தேடல்_Ordering Order) {
    எனில் (Order & SO_LoadOrder) {
      க்கு (வெறுமை *Handle : Handles) {
        எனில் (வெறுமை *Ptr = DLSym(Handle, குறியீடு))
          திரும்பு Ptr;
      }
    } இல்லையெனில் {
      க்கு (வெறுமை *Handle : llvm::reverse(Handles)) {
        எனில் (வெறுமை *Ptr = DLSym(Handle, குறியீடு))
          திரும்பு Ptr;
      }
    }
    திரும்பு nullptr;
  }

  வெறுமை *Lookup(மாறிலி எழுத்து *குறியீடு, DynamicLibrary::தேடல்_Ordering Order) {
    ப்ரதிஜ்ஞா(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    எனில் (Process == &Invalid || (Order & SO_LoadedFirst)) {
      எனில் (வெறுமை *Ptr = LibLookup(குறியீடு, Order))
        திரும்பு Ptr;
    }
    எனில் (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      எனில் (வெறுமை *Ptr = DLSym(Process, குறியீடு))
        திரும்பு Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      எனில் (Order & SO_LoadedLast) {
        எனில் (வெறுமை *Ptr = LibLookup(குறியீடு, Order))
          திரும்பு Ptr;
      }
    }
    திரும்பு nullptr;
  }
};

நாமாகாஶ {

கட்டமைப்பு Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<வெறுமை *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<உண்மை> SymbolsMutex;
};

Globals &getGlobals() {
  நிலையான Globals G;
  திரும்பு G;
}

} // namespace

#ifdef _WIN32

#அந்தர்பாவ "Windows/DynamicLibrary.inc"

#இல்லையெனில்

#அந்தர்பாவ "Unix/DynamicLibrary.inc"

#endif

எழுத்து DynamicLibrary::Invalid;
DynamicLibrary::தேடல்_Ordering DynamicLibrary::தேடல்_Order =
    DynamicLibrary::SO_Linker;

நாமாகாஶ llvm {
வெறுமை *தேடல்_For_Address_Of_Special_குறியீடு(மாறிலி எழுத்து *குறியீடு_Name) {
  திரும்பு Do_தேடல்(குறியீடு_Name); // DynamicLibrary.inc
}
} // namespace llvm

வெறுமை DynamicLibrary::Add_குறியீடு(StringRef குறியீடு_Name, வெறுமை *குறியீடு_மதிப்பு) {
  auto &G = getGlobals();
  SmartScopedLock<உண்மை> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[குறியீடு_Name] = குறியீடு_மதிப்பு;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(மாறிலி எழுத்து *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  வெறுமை *Handle = HandleSet::DLOpen(FileName, Err);
  எனில் (Handle != &Invalid) {
    SmartScopedLock<உண்மை> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  திரும்பு DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(வெறுமை *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<உண்மை> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  எனில் (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ பொய்,
                                  /*CanClose*/ பொய்))
    *Err = "Library already loaded";

  திரும்பு DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(மாறிலி எழுத்து *FileName,
                                          std::string *Err) {
  ப்ரதிஜ்ஞா(FileName && "Use getPermanentLibrary() for opening process handle");
  வெறுமை *Handle = HandleSet::DLOpen(FileName, Err);
  எனில் (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<உண்மை> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ பொய்,
                                        /*CanClose*/ பொய்,
                                        /*AllowDuplicates*/ உண்மை);
  }
  திரும்பு DynamicLibrary(Handle);
}

வெறுமை DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<உண்மை> Lock(G.SymbolsMutex);
  எனில் (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

வெறுமை *DynamicLibrary::get_Address_Of_குறியீடு(மாறிலி எழுத்து *குறியீடு_Name) {
  எனில் (!isValid())
    திரும்பு nullptr;
  திரும்பு HandleSet::DLSym(Data, குறியீடு_Name);
}

வெறுமை *DynamicLibrary::தேடல்_For_Address_Of_குறியீடு(மாறிலி எழுத்து *குறியீடு_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<உண்மை> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<வெறுமை *>::iterator i = G.ExplicitSymbols.find(குறியீடு_Name);

    எனில் (i != G.ExplicitSymbols.end())
      திரும்பு i->second;

    // Now search the libraries.
    எனில் (வெறுமை *Ptr = G.OpenedHandles.Lookup(குறியீடு_Name, தேடல்_Order))
      திரும்பு Ptr;
    எனில் (வெறுமை *Ptr = G.OpenedTemporaryHandles.Lookup(குறியீடு_Name, தேடல்_Order))
      திரும்பு Ptr;
  }

  திரும்பு llvm::தேடல்_For_Address_Of_Special_குறியீடு(குறியீடு_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(மாறிலி எழுத்து *Filename) {
  திரும்பு llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

வெறுமை *LLVMSearch_For_Address_Of_குறியீடு(மாறிலி எழுத்து *குறியீடு_Name_2) {
  திரும்பு llvm::sys::DynamicLibrary::தேடல்_For_Address_Of_குறியீடு(குறியீடு_Name_2);
}

வெறுமை LLVMAdd_குறியீடு(மாறிலி எழுத்து *குறியீடு_Name_2, வெறுமை *குறியீடு_மதிப்பு_2) {
  திரும்பு llvm::sys::DynamicLibrary::Add_குறியீடு(குறியீடு_Name_2, குறியீடு_மதிப்பு_2);
}
