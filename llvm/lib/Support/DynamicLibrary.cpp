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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/DynamicLibrary.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm-c/Support.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Mutex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <vector>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
ਵਰਗ DynamicLibrary::Handle_ਸੈੱਟ {
  ਵਰਤੋਂ HandleList = std::ਸਦਿਸ਼<ਖਾਲੀ *>;
  HandleList Handles;
  ਖਾਲੀ *Process = &Invalid;

ਜਨਤਕ:
  ਸਥਿਰ ਖਾਲੀ *DLOpen(ਅਚਲ ਅੱਖਰ *Filename, std::string *Err);
  ਸਥਿਰ ਖਾਲੀ DLClose(ਖਾਲੀ *Handle);
  ਸਥਿਰ ਖਾਲੀ *DLSym(ਖਾਲੀ *Handle, ਅਚਲ ਅੱਖਰ *ਚਿੰਨ੍ਹ);

  Handle_ਸੈੱਟ() = ਮੂਲ;
  ~Handle_ਸੈੱਟ();

  HandleList::iterator ਲੱਭੋ_2(ਖਾਲੀ *Handle) { ਵਾਪਸ ਲੱਭੋ(Handles, Handle); }

  ਤਾਰਕਿਕ ਸ਼ਾਮਲ_ਹੈ_2(ਖਾਲੀ *Handle) {
    ਵਾਪਸ Handle == Process || ਲੱਭੋ_2(Handle) != Handles.ਸਿਰਾ();
  }

  ਤਾਰਕਿਕ ਜੋੜੋ_Library(ਖਾਲੀ *Handle, ਤਾਰਕਿਕ IsProcess = ਝੂਠ, ਤਾਰਕਿਕ CanClose = ਸੱਚ,
                  ਤਾਰਕਿਕ AllowDuplicates = ਝੂਠ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
    ਪ੍ਰਤਿਜ੍ਞਾ((Handle == ਇਹ ? IsProcess : !IsProcess) && "Bad Handle.");
#ਜੇ_ਅੰਤ
    ਪ੍ਰਤਿਜ੍ਞਾ((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ਜੇ (LLVM_LIKELY(!IsProcess)) {
      ਜੇ (!AllowDuplicates && ਲੱਭੋ_2(Handle) != Handles.ਸਿਰਾ()) {
        ਜੇ (CanClose)
          DLClose(Handle);
        ਵਾਪਸ ਝੂਠ;
      }
      Handles.ਧੱਕੋ_ਪਿੱਠ(Handle);
    } ਨਹੀਂਤਾਂ {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
      ਜੇ (Process != &Invalid) {
        ਜੇ (CanClose)
          DLClose(Process);
        ਜੇ (Process == Handle)
          ਵਾਪਸ ਝੂਠ;
      }
#ਜੇ_ਅੰਤ
      Process = Handle;
    }
    ਵਾਪਸ ਸੱਚ;
  }

  ਖਾਲੀ CloseLibrary(ਖਾਲੀ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = ਲੱਭੋ_2(Handle);
    ਜੇ (it != Handles.ਸਿਰਾ()) {
      Handles.erase(it);
    }
  }

  ਖਾਲੀ *LibLookup(ਅਚਲ ਅੱਖਰ *ਚਿੰਨ੍ਹ, DynamicLibrary::ਖੋਜ_Ordering ਕ੍ਰਮ) {
    ਜੇ (ਕ੍ਰਮ & SO_ਲੱਦੋ_ਕ੍ਰਮ) {
      ਲਈ (ਖਾਲੀ *Handle : Handles) {
        ਜੇ (ਖਾਲੀ *Ptr = DLSym(Handle, ਚਿੰਨ੍ਹ))
          ਵਾਪਸ Ptr;
      }
    } ਨਹੀਂਤਾਂ {
      ਲਈ (ਖਾਲੀ *Handle : llvm::ਉਲਟ(Handles)) {
        ਜੇ (ਖਾਲੀ *Ptr = DLSym(Handle, ਚਿੰਨ੍ਹ))
          ਵਾਪਸ Ptr;
      }
    }
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }

  ਖਾਲੀ *Lookup(ਅਚਲ ਅੱਖਰ *ਚਿੰਨ੍ਹ, DynamicLibrary::ਖੋਜ_Ordering ਕ੍ਰਮ) {
    ਪ੍ਰਤਿਜ੍ਞਾ(!((ਕ੍ਰਮ & SO_Loaded_ਪਹਿਲਾ) && (ਕ੍ਰਮ & SO_Loaded_ਆਖਰੀ)) &&
           "Invalid Ordering");

    ਜੇ (Process == &Invalid || (ਕ੍ਰਮ & SO_Loaded_ਪਹਿਲਾ)) {
      ਜੇ (ਖਾਲੀ *Ptr = LibLookup(ਚਿੰਨ੍ਹ, ਕ੍ਰਮ))
        ਵਾਪਸ Ptr;
    }
    ਜੇ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ਜੇ (ਖਾਲੀ *Ptr = DLSym(Process, ਚਿੰਨ੍ਹ))
        ਵਾਪਸ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ਜੇ (ਕ੍ਰਮ & SO_Loaded_ਆਖਰੀ) {
        ਜੇ (ਖਾਲੀ *Ptr = LibLookup(ਚਿੰਨ੍ਹ, ਕ੍ਰਮ))
          ਵਾਪਸ Ptr;
      }
    }
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }
};

ਨਾਮਾਕਾਸ਼ {

ਬਣਤਰ Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<ਖਾਲੀ *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::Handle_ਸੈੱਟ OpenedHandles;
  DynamicLibrary::Handle_ਸੈੱਟ OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<ਸੱਚ> SymbolsMutex;
};

Globals &getGlobals() {
  ਸਥਿਰ Globals G;
  ਵਾਪਸ G;
}

} // namespace

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32

#ਅਨ੍ਤਰ੍ਭਾਵ "Windows/DynamicLibrary.inc"

#ਨਹੀਂਤਾਂ

#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/DynamicLibrary.inc"

#ਜੇ_ਅੰਤ

ਅੱਖਰ DynamicLibrary::Invalid;
DynamicLibrary::ਖੋਜ_Ordering DynamicLibrary::ਖੋਜ_ਕ੍ਰਮ =
    DynamicLibrary::SO_Linker;

ਨਾਮਾਕਾਸ਼ llvm {
ਖਾਲੀ *ਖੋਜ_For_ਪਤਾ_ਦਾ_Special_ਚਿੰਨ੍ਹ(ਅਚਲ ਅੱਖਰ *ਚਿੰਨ੍ਹ_Name) {
  ਵਾਪਸ Do_ਖੋਜ(ਚਿੰਨ੍ਹ_Name); // DynamicLibrary.inc
}
} // namespace llvm

ਖਾਲੀ DynamicLibrary::ਜੋੜੋ_ਚਿੰਨ੍ਹ(StringRef ਚਿੰਨ੍ਹ_Name, ਖਾਲੀ *ਚਿੰਨ੍ਹ_ਮੁੱਲ) {
  ਸਵੈਚਾਲਿਤ &G = getGlobals();
  Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(G.SymbolsMutex);
  G.ExplicitSymbols[ਚਿੰਨ੍ਹ_Name] = ਚਿੰਨ੍ਹ_ਮੁੱਲ;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ਅਚਲ ਅੱਖਰ *FileName,
                                                   std::string *Err) {
  ਸਵੈਚਾਲਿਤ &G = getGlobals();
  ਖਾਲੀ *Handle = Handle_ਸੈੱਟ::DLOpen(FileName, Err);
  ਜੇ (Handle != &Invalid) {
    Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(G.SymbolsMutex);
    G.OpenedHandles.ਜੋੜੋ_Library(Handle, /*IsProcess*/ FileName == ਖਾਲੀ_ਸੂਚਕ);
  }

  ਵਾਪਸ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::ਜੋੜੋ_Permanent_Library(ਖਾਲੀ *Handle,
                                                   std::string *Err) {
  ਸਵੈਚਾਲਿਤ &G = getGlobals();
  Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ਜੇ (!G.OpenedHandles.ਜੋੜੋ_Library(Handle, /*IsProcess*/ ਝੂਠ,
                                  /*CanClose*/ ਝੂਠ))
    *Err = "Library already loaded";

  ਵਾਪਸ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ਅਚਲ ਅੱਖਰ *FileName,
                                          std::string *Err) {
  ਪ੍ਰਤਿਜ੍ਞਾ(FileName && "Use getPermanentLibrary() for opening process handle");
  ਖਾਲੀ *Handle = Handle_ਸੈੱਟ::DLOpen(FileName, Err);
  ਜੇ (Handle != &Invalid) {
    ਸਵੈਚਾਲਿਤ &G = getGlobals();
    Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(G.SymbolsMutex);
    G.OpenedTemporaryHandles.ਜੋੜੋ_Library(Handle, /*IsProcess*/ ਝੂਠ,
                                        /*CanClose*/ ਝੂਠ,
                                        /*AllowDuplicates*/ ਸੱਚ);
  }
  ਵਾਪਸ DynamicLibrary(Handle);
}

ਖਾਲੀ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  ਸਵੈਚਾਲਿਤ &G = getGlobals();
  Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(G.SymbolsMutex);
  ਜੇ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.ਅੰਕੜੇ_2);
    Lib.ਅੰਕੜੇ_2 = &Invalid;
  }
}

ਖਾਲੀ *DynamicLibrary::get_ਪਤਾ_ਦਾ_ਚਿੰਨ੍ਹ(ਅਚਲ ਅੱਖਰ *ਚਿੰਨ੍ਹ_Name) {
  ਜੇ (!isValid())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  ਵਾਪਸ Handle_ਸੈੱਟ::DLSym(ਅੰਕੜੇ_2, ਚਿੰਨ੍ਹ_Name);
}

ਖਾਲੀ *DynamicLibrary::ਖੋਜ_For_ਪਤਾ_ਦਾ_ਚਿੰਨ੍ਹ(ਅਚਲ ਅੱਖਰ *ਚਿੰਨ੍ਹ_Name) {
  {
    ਸਵੈਚਾਲਿਤ &G = getGlobals();
    Smart_Scoped_ਤਾਲਾ<ਸੱਚ> ਤਾਲਾ_2(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<ਖਾਲੀ *>::iterator i = G.ExplicitSymbols.ਲੱਭੋ(ਚਿੰਨ੍ਹ_Name);

    ਜੇ (i != G.ExplicitSymbols.ਸਿਰਾ())
      ਵਾਪਸ i->ਦੂਜਾ_2;

    // Now search the libraries.
    ਜੇ (ਖਾਲੀ *Ptr = G.OpenedHandles.Lookup(ਚਿੰਨ੍ਹ_Name, ਖੋਜ_ਕ੍ਰਮ))
      ਵਾਪਸ Ptr;
    ਜੇ (ਖਾਲੀ *Ptr = G.OpenedTemporaryHandles.Lookup(ਚਿੰਨ੍ਹ_Name, ਖੋਜ_ਕ੍ਰਮ))
      ਵਾਪਸ Ptr;
  }

  ਵਾਪਸ llvm::ਖੋਜ_For_ਪਤਾ_ਦਾ_Special_ਚਿੰਨ੍ਹ(ਚਿੰਨ੍ਹ_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ਅਚਲ ਅੱਖਰ *Filename) {
  ਵਾਪਸ llvm::sys::DynamicLibrary::ਲੱਦੋ_Library_Permanently(Filename);
}

ਖਾਲੀ *LLVMSearch_For_ਪਤਾ_ਦਾ_ਚਿੰਨ੍ਹ(ਅਚਲ ਅੱਖਰ *ਚਿੰਨ੍ਹ_Name_2) {
  ਵਾਪਸ llvm::sys::DynamicLibrary::ਖੋਜ_For_ਪਤਾ_ਦਾ_ਚਿੰਨ੍ਹ(ਚਿੰਨ੍ਹ_Name_2);
}

ਖਾਲੀ LLVMAdd_ਚਿੰਨ੍ਹ(ਅਚਲ ਅੱਖਰ *ਚਿੰਨ੍ਹ_Name_2, ਖਾਲੀ *ਚਿੰਨ੍ਹ_ਮੁੱਲ_2) {
  ਵਾਪਸ llvm::sys::DynamicLibrary::ਜੋੜੋ_ਚਿੰਨ੍ਹ(ਚਿੰਨ੍ਹ_Name_2, ਚਿੰਨ੍ਹ_ਮੁੱਲ_2);
}
