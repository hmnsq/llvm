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

#અન્તર્ભાવ "llvm/Support/DynamicLibrary.h"
#અન્તર્ભાવ "llvm-c/Support.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringMap.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Mutex.h"
#અન્તર્ભાવ <vector>

using નામાકાશ llvm;
using નામાકાશ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
વર્ગ DynamicLibrary::HandleSet {
  using HandleList = std::vector<શૂન્ય *>;
  HandleList Handles;
  શૂન્ય *Process = &Invalid;

જાહેર:
  સ્થિર શૂન્ય *DLOpen(અચલ અક્ષર *Filename, std::string *Err);
  સ્થિર શૂન્ય DLClose(શૂન્ય *Handle);
  સ્થિર શૂન્ય *DLSym(શૂન્ય *Handle, અચલ અક્ષર *પ્રતીક);

  HandleSet() = મૂળભૂત;
  ~HandleSet();

  HandleList::iterator Find(શૂન્ય *Handle) { પાછા find(Handles, Handle); }

  તાર્કિક Contains(શૂન્ય *Handle) {
    પાછા Handle == Process || Find(Handle) != Handles.end();
  }

  તાર્કિક AddLibrary(શૂન્ય *Handle, તાર્કિક IsProcess = ખોટું, તાર્કિક CanClose = સાચું,
                  તાર્કિક AllowDuplicates = ખોટું) {
#ifdef _WIN32
    પ્રતિજ્ઞા((Handle == આ ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    પ્રતિજ્ઞા((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    જો (LLVM_LIKELY(!IsProcess)) {
      જો (!AllowDuplicates && Find(Handle) != Handles.end()) {
        જો (CanClose)
          DLClose(Handle);
        પાછા ખોટું;
      }
      Handles.ધકેલો_back(Handle);
    } નહીંતર {
#ifndef _WIN32
      જો (Process != &Invalid) {
        જો (CanClose)
          DLClose(Process);
        જો (Process == Handle)
          પાછા ખોટું;
      }
#endif
      Process = Handle;
    }
    પાછા સાચું;
  }

  શૂન્ય CloseLibrary(શૂન્ય *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    જો (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  શૂન્ય *LibLookup(અચલ અક્ષર *પ્રતીક, DynamicLibrary::શોધ_Ordering Order) {
    જો (Order & SO_LoadOrder) {
      માટે (શૂન્ય *Handle : Handles) {
        જો (શૂન્ય *Ptr = DLSym(Handle, પ્રતીક))
          પાછા Ptr;
      }
    } નહીંતર {
      માટે (શૂન્ય *Handle : llvm::reverse(Handles)) {
        જો (શૂન્ય *Ptr = DLSym(Handle, પ્રતીક))
          પાછા Ptr;
      }
    }
    પાછા nullptr;
  }

  શૂન્ય *Lookup(અચલ અક્ષર *પ્રતીક, DynamicLibrary::શોધ_Ordering Order) {
    પ્રતિજ્ઞા(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    જો (Process == &Invalid || (Order & SO_LoadedFirst)) {
      જો (શૂન્ય *Ptr = LibLookup(પ્રતીક, Order))
        પાછા Ptr;
    }
    જો (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      જો (શૂન્ય *Ptr = DLSym(Process, પ્રતીક))
        પાછા Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      જો (Order & SO_LoadedLast) {
        જો (શૂન્ય *Ptr = LibLookup(પ્રતીક, Order))
          પાછા Ptr;
      }
    }
    પાછા nullptr;
  }
};

નામાકાશ {

સંરચના Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<શૂન્ય *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<સાચું> SymbolsMutex;
};

Globals &getGlobals() {
  સ્થિર Globals G;
  પાછા G;
}

} // namespace

#ifdef _WIN32

#અન્તર્ભાવ "Windows/DynamicLibrary.inc"

#નહીંતર

#અન્તર્ભાવ "Unix/DynamicLibrary.inc"

#endif

અક્ષર DynamicLibrary::Invalid;
DynamicLibrary::શોધ_Ordering DynamicLibrary::શોધ_Order =
    DynamicLibrary::SO_Linker;

નામાકાશ llvm {
શૂન્ય *શોધ_For_Address_Of_Special_પ્રતીક(અચલ અક્ષર *પ્રતીક_Name) {
  પાછા Do_શોધ(પ્રતીક_Name); // DynamicLibrary.inc
}
} // namespace llvm

શૂન્ય DynamicLibrary::Add_પ્રતીક(StringRef પ્રતીક_Name, શૂન્ય *પ્રતીક_કિંમત) {
  auto &G = getGlobals();
  SmartScopedLock<સાચું> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[પ્રતીક_Name] = પ્રતીક_કિંમત;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(અચલ અક્ષર *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  શૂન્ય *Handle = HandleSet::DLOpen(FileName, Err);
  જો (Handle != &Invalid) {
    SmartScopedLock<સાચું> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  પાછા DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(શૂન્ય *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<સાચું> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  જો (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ ખોટું,
                                  /*CanClose*/ ખોટું))
    *Err = "Library already loaded";

  પાછા DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(અચલ અક્ષર *FileName,
                                          std::string *Err) {
  પ્રતિજ્ઞા(FileName && "Use getPermanentLibrary() for opening process handle");
  શૂન્ય *Handle = HandleSet::DLOpen(FileName, Err);
  જો (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<સાચું> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ ખોટું,
                                        /*CanClose*/ ખોટું,
                                        /*AllowDuplicates*/ સાચું);
  }
  પાછા DynamicLibrary(Handle);
}

શૂન્ય DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<સાચું> Lock(G.SymbolsMutex);
  જો (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

શૂન્ય *DynamicLibrary::get_Address_Of_પ્રતીક(અચલ અક્ષર *પ્રતીક_Name) {
  જો (!isValid())
    પાછા nullptr;
  પાછા HandleSet::DLSym(Data, પ્રતીક_Name);
}

શૂન્ય *DynamicLibrary::શોધ_For_Address_Of_પ્રતીક(અચલ અક્ષર *પ્રતીક_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<સાચું> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<શૂન્ય *>::iterator i = G.ExplicitSymbols.find(પ્રતીક_Name);

    જો (i != G.ExplicitSymbols.end())
      પાછા i->second;

    // Now search the libraries.
    જો (શૂન્ય *Ptr = G.OpenedHandles.Lookup(પ્રતીક_Name, શોધ_Order))
      પાછા Ptr;
    જો (શૂન્ય *Ptr = G.OpenedTemporaryHandles.Lookup(પ્રતીક_Name, શોધ_Order))
      પાછા Ptr;
  }

  પાછા llvm::શોધ_For_Address_Of_Special_પ્રતીક(પ્રતીક_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(અચલ અક્ષર *Filename) {
  પાછા llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

શૂન્ય *LLVMSearch_For_Address_Of_પ્રતીક(અચલ અક્ષર *પ્રતીક_Name_2) {
  પાછા llvm::sys::DynamicLibrary::શોધ_For_Address_Of_પ્રતીક(પ્રતીક_Name_2);
}

શૂન્ય LLVMAdd_પ્રતીક(અચલ અક્ષર *પ્રતીક_Name_2, શૂન્ય *પ્રતીક_કિંમત_2) {
  પાછા llvm::sys::DynamicLibrary::Add_પ્રતીક(પ્રતીક_Name_2, પ્રતીક_કિંમત_2);
}
