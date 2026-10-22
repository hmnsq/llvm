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

#अन्तर्भाव "llvm/Support/DynamicLibrary.h"
#अन्तर्भाव "llvm-c/Support.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Mutex.h"
#अन्तर्भाव <vector>

using नामाकाश llvm;
using नामाकाश llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
वर्ग DynamicLibrary::HandleSet {
  using HandleList = std::vector<शून्य *>;
  HandleList Handles;
  शून्य *Process = &Invalid;

सार्वजनिक:
  स्थिर शून्य *DLOpen(ध्रुव वर्ण *Filename, std::string *Err);
  स्थिर शून्य DLClose(शून्य *Handle);
  स्थिर शून्य *DLSym(शून्य *Handle, ध्रुव वर्ण *प्रतीक);

  HandleSet() = स्वभाव;
  ~HandleSet();

  HandleList::iterator Find(शून्य *Handle) { प्रत्यागम find(Handles, Handle); }

  तार्किक Contains(शून्य *Handle) {
    प्रत्यागम Handle == Process || Find(Handle) != Handles.end();
  }

  तार्किक AddLibrary(शून्य *Handle, तार्किक IsProcess = असत्य, तार्किक CanClose = सत्य,
                  तार्किक AllowDuplicates = असत्य) {
#ifdef _WIN32
    प्रतिज्ञा((Handle == एतत् ? IsProcess : !IsProcess) && "Bad Handle.");
#endif
    प्रतिज्ञा((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    यदि (LLVM_LIKELY(!IsProcess)) {
      यदि (!AllowDuplicates && Find(Handle) != Handles.end()) {
        यदि (CanClose)
          DLClose(Handle);
        प्रत्यागम असत्य;
      }
      Handles.धकेलो_back(Handle);
    } अन्यथा {
#ifndef _WIN32
      यदि (Process != &Invalid) {
        यदि (CanClose)
          DLClose(Process);
        यदि (Process == Handle)
          प्रत्यागम असत्य;
      }
#endif
      Process = Handle;
    }
    प्रत्यागम सत्य;
  }

  शून्य CloseLibrary(शून्य *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    यदि (it != Handles.end()) {
      Handles.erase(it);
    }
  }

  शून्य *LibLookup(ध्रुव वर्ण *प्रतीक, DynamicLibrary::खोज_Ordering Order) {
    यदि (Order & SO_LoadOrder) {
      प्रति (शून्य *Handle : Handles) {
        यदि (शून्य *Ptr = DLSym(Handle, प्रतीक))
          प्रत्यागम Ptr;
      }
    } अन्यथा {
      प्रति (शून्य *Handle : llvm::reverse(Handles)) {
        यदि (शून्य *Ptr = DLSym(Handle, प्रतीक))
          प्रत्यागम Ptr;
      }
    }
    प्रत्यागम nullptr;
  }

  शून्य *Lookup(ध्रुव वर्ण *प्रतीक, DynamicLibrary::खोज_Ordering Order) {
    प्रतिज्ञा(!((Order & SO_LoadedFirst) && (Order & SO_LoadedLast)) &&
           "Invalid Ordering");

    यदि (Process == &Invalid || (Order & SO_LoadedFirst)) {
      यदि (शून्य *Ptr = LibLookup(प्रतीक, Order))
        प्रत्यागम Ptr;
    }
    यदि (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      यदि (शून्य *Ptr = DLSym(Process, प्रतीक))
        प्रत्यागम Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      यदि (Order & SO_LoadedLast) {
        यदि (शून्य *Ptr = LibLookup(प्रतीक, Order))
          प्रत्यागम Ptr;
      }
    }
    प्रत्यागम nullptr;
  }
};

नामाकाश {

संरचना Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<शून्य *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::HandleSet OpenedHandles;
  DynamicLibrary::HandleSet OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<सत्य> SymbolsMutex;
};

Globals &getGlobals() {
  स्थिर Globals G;
  प्रत्यागम G;
}

} // namespace

#ifdef _WIN32

#अन्तर्भाव "Windows/DynamicLibrary.inc"

#अन्यथा

#अन्तर्भाव "Unix/DynamicLibrary.inc"

#endif

वर्ण DynamicLibrary::Invalid;
DynamicLibrary::खोज_Ordering DynamicLibrary::खोज_Order =
    DynamicLibrary::SO_Linker;

नामाकाश llvm {
शून्य *खोज_For_Address_Of_Special_प्रतीक(ध्रुव वर्ण *प्रतीक_Name) {
  प्रत्यागम Do_खोज(प्रतीक_Name); // DynamicLibrary.inc
}
} // namespace llvm

शून्य DynamicLibrary::Add_प्रतीक(StringRef प्रतीक_Name, शून्य *प्रतीक_मूल्य) {
  auto &G = getGlobals();
  SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
  G.ExplicitSymbols[प्रतीक_Name] = प्रतीक_मूल्य;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ध्रुव वर्ण *FileName,
                                                   std::string *Err) {
  auto &G = getGlobals();
  शून्य *Handle = HandleSet::DLOpen(FileName, Err);
  यदि (Handle != &Invalid) {
    SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
    G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ FileName == nullptr);
  }

  प्रत्यागम DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::addPermanentLibrary(शून्य *Handle,
                                                   std::string *Err) {
  auto &G = getGlobals();
  SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  यदि (!G.OpenedHandles.AddLibrary(Handle, /*IsProcess*/ असत्य,
                                  /*CanClose*/ असत्य))
    *Err = "Library already loaded";

  प्रत्यागम DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ध्रुव वर्ण *FileName,
                                          std::string *Err) {
  प्रतिज्ञा(FileName && "Use getPermanentLibrary() for opening process handle");
  शून्य *Handle = HandleSet::DLOpen(FileName, Err);
  यदि (Handle != &Invalid) {
    auto &G = getGlobals();
    SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
    G.OpenedTemporaryHandles.AddLibrary(Handle, /*IsProcess*/ असत्य,
                                        /*CanClose*/ असत्य,
                                        /*AllowDuplicates*/ सत्य);
  }
  प्रत्यागम DynamicLibrary(Handle);
}

शून्य DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  auto &G = getGlobals();
  SmartScopedLock<सत्य> Lock(G.SymbolsMutex);
  यदि (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.Data);
    Lib.Data = &Invalid;
  }
}

शून्य *DynamicLibrary::get_Address_Of_प्रतीक(ध्रुव वर्ण *प्रतीक_Name) {
  यदि (!isValid())
    प्रत्यागम nullptr;
  प्रत्यागम HandleSet::DLSym(Data, प्रतीक_Name);
}

शून्य *DynamicLibrary::खोज_For_Address_Of_प्रतीक(ध्रुव वर्ण *प्रतीक_Name) {
  {
    auto &G = getGlobals();
    SmartScopedLock<सत्य> Lock(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<शून्य *>::iterator i = G.ExplicitSymbols.find(प्रतीक_Name);

    यदि (i != G.ExplicitSymbols.end())
      प्रत्यागम i->second;

    // Now search the libraries.
    यदि (शून्य *Ptr = G.OpenedHandles.Lookup(प्रतीक_Name, खोज_Order))
      प्रत्यागम Ptr;
    यदि (शून्य *Ptr = G.OpenedTemporaryHandles.Lookup(प्रतीक_Name, खोज_Order))
      प्रत्यागम Ptr;
  }

  प्रत्यागम llvm::खोज_For_Address_Of_Special_प्रतीक(प्रतीक_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ध्रुव वर्ण *Filename) {
  प्रत्यागम llvm::sys::DynamicLibrary::LoadLibraryPermanently(Filename);
}

शून्य *LLVMSearch_For_Address_Of_प्रतीक(ध्रुव वर्ण *प्रतीक_Name_2) {
  प्रत्यागम llvm::sys::DynamicLibrary::खोज_For_Address_Of_प्रतीक(प्रतीक_Name_2);
}

शून्य LLVMAdd_प्रतीक(ध्रुव वर्ण *प्रतीक_Name_2, शून्य *प्रतीक_मूल्य_2) {
  प्रत्यागम llvm::sys::DynamicLibrary::Add_प्रतीक(प्रतीक_Name_2, प्रतीक_मूल्य_2);
}
