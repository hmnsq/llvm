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

#ಅನ್ತರ್ಭಾವ "llvm/Support/DynamicLibrary.h"
#ಅನ್ತರ್ಭಾವ "llvm-c/Support.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Mutex.h"
#ಅನ್ತರ್ಭಾವ <vector>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
ವರ್ಗ DynamicLibrary::Handle_ಗಣ {
  ಬಳಸಿ HandleList = std::ಸದಿಶ<ಶೂನ್ಯ *>;
  HandleList Handles;
  ಶೂನ್ಯ *Process = &Invalid;

ಸಾರ್ವಜನಿಕ:
  ಸ್ಥಿರ ಶೂನ್ಯ *DLOpen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Filename, std::string *Err);
  ಸ್ಥಿರ ಶೂನ್ಯ DLClose(ಶೂನ್ಯ *Handle);
  ಸ್ಥಿರ ಶೂನ್ಯ *DLSym(ಶೂನ್ಯ *Handle, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಚಿಹ್ನೆ);

  Handle_ಗಣ() = ಪೂರ್ವನಿಯೋಜಿತ;
  ~Handle_ಗಣ();

  HandleList::iterator ಕಂಡುಹಿಡಿ_2(ಶೂನ್ಯ *Handle) { ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ(Handles, Handle); }

  ತಾರ್ಕಿಕ ಒಳಗೊಂಡಿದೆ_2(ಶೂನ್ಯ *Handle) {
    ಹಿಂತಿರುಗಿ Handle == Process || ಕಂಡುಹಿಡಿ_2(Handle) != Handles.ಕೊನೆ();
  }

  ತಾರ್ಕಿಕ ಕೂಡಿಸು_Library(ಶೂನ್ಯ *Handle, ತಾರ್ಕಿಕ IsProcess = ಸುಳ್ಳು, ತಾರ್ಕಿಕ CanClose = ನಿಜ,
                  ತಾರ್ಕಿಕ AllowDuplicates = ಸುಳ್ಳು) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
    ಪ್ರತಿಜ್ಞಾ((Handle == ಇದು ? IsProcess : !IsProcess) && "Bad Handle.");
#ಆದರೆ_ಅಂತ್ಯ
    ಪ್ರತಿಜ್ಞಾ((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ಆದರೆ (LLVM_LIKELY(!IsProcess)) {
      ಆದರೆ (!AllowDuplicates && ಕಂಡುಹಿಡಿ_2(Handle) != Handles.ಕೊನೆ()) {
        ಆದರೆ (CanClose)
          DLClose(Handle);
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      }
      Handles.ತಳ್ಳು_ಹಿಂಭಾಗ(Handle);
    } ಇಲ್ಲದಿದ್ದರೆ {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
      ಆದರೆ (Process != &Invalid) {
        ಆದರೆ (CanClose)
          DLClose(Process);
        ಆದರೆ (Process == Handle)
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      }
#ಆದರೆ_ಅಂತ್ಯ
      Process = Handle;
    }
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ಶೂನ್ಯ CloseLibrary(ಶೂನ್ಯ *Handle) {
    DLClose(Handle);
    HandleList::iterator it = ಕಂಡುಹಿಡಿ_2(Handle);
    ಆದರೆ (it != Handles.ಕೊನೆ()) {
      Handles.erase(it);
    }
  }

  ಶೂನ್ಯ *LibLookup(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಚಿಹ್ನೆ, DynamicLibrary::ಹುಡುಕು_Ordering ಕ್ರಮ) {
    ಆದರೆ (ಕ್ರಮ & SO_ತುಂಬು_ಕ್ರಮ) {
      ಗಾಗಿ (ಶೂನ್ಯ *Handle : Handles) {
        ಆದರೆ (ಶೂನ್ಯ *Ptr = DLSym(Handle, ಚಿಹ್ನೆ))
          ಹಿಂತಿರುಗಿ Ptr;
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಗಾಗಿ (ಶೂನ್ಯ *Handle : llvm::ಹಿಮ್ಮುಖ(Handles)) {
        ಆದರೆ (ಶೂನ್ಯ *Ptr = DLSym(Handle, ಚಿಹ್ನೆ))
          ಹಿಂತಿರುಗಿ Ptr;
      }
    }
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }

  ಶೂನ್ಯ *Lookup(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಚಿಹ್ನೆ, DynamicLibrary::ಹುಡುಕು_Ordering ಕ್ರಮ) {
    ಪ್ರತಿಜ್ಞಾ(!((ಕ್ರಮ & SO_Loaded_ಮೊದಲ) && (ಕ್ರಮ & SO_Loaded_ಕೊನೆಯ)) &&
           "Invalid Ordering");

    ಆದರೆ (Process == &Invalid || (ಕ್ರಮ & SO_Loaded_ಮೊದಲ)) {
      ಆದರೆ (ಶೂನ್ಯ *Ptr = LibLookup(ಚಿಹ್ನೆ, ಕ್ರಮ))
        ಹಿಂತಿರುಗಿ Ptr;
    }
    ಆದರೆ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ಆದರೆ (ಶೂನ್ಯ *Ptr = DLSym(Process, ಚಿಹ್ನೆ))
        ಹಿಂತಿರುಗಿ Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ಆದರೆ (ಕ್ರಮ & SO_Loaded_ಕೊನೆಯ) {
        ಆದರೆ (ಶೂನ್ಯ *Ptr = LibLookup(ಚಿಹ್ನೆ, ಕ್ರಮ))
          ಹಿಂತಿರುಗಿ Ptr;
      }
    }
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }
};

ನಾಮಾಕಾಶ {

ರಚನೆ Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<ಶೂನ್ಯ *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::Handle_ಗಣ OpenedHandles;
  DynamicLibrary::Handle_ಗಣ OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<ನಿಜ> SymbolsMutex;
};

Globals &getGlobals() {
  ಸ್ಥಿರ Globals G;
  ಹಿಂತಿರುಗಿ G;
}

} // namespace

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32

#ಅನ್ತರ್ಭಾವ "Windows/DynamicLibrary.inc"

#ಇಲ್ಲದಿದ್ದರೆ

#ಅನ್ತರ್ಭಾವ "Unix/DynamicLibrary.inc"

#ಆದರೆ_ಅಂತ್ಯ

ಅಕ್ಷರ DynamicLibrary::Invalid;
DynamicLibrary::ಹುಡುಕು_Ordering DynamicLibrary::ಹುಡುಕು_ಕ್ರಮ =
    DynamicLibrary::SO_Linker;

ನಾಮಾಕಾಶ llvm {
ಶೂನ್ಯ *ಹುಡುಕು_For_ವಿಳಾಸ_ದ_Special_ಚಿಹ್ನೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಚಿಹ್ನೆ_Name) {
  ಹಿಂತಿರುಗಿ Do_ಹುಡುಕು(ಚಿಹ್ನೆ_Name); // DynamicLibrary.inc
}
} // namespace llvm

ಶೂನ್ಯ DynamicLibrary::ಕೂಡಿಸು_ಚಿಹ್ನೆ(StringRef ಚಿಹ್ನೆ_Name, ಶೂನ್ಯ *ಚಿಹ್ನೆ_ಮೌಲ್ಯ) {
  ಸ್ವಯಂಚಾಲಿತ &G = getGlobals();
  Smart_Scoped_ಬೀಗ<ನಿಜ> ಬೀಗ_3(G.SymbolsMutex);
  G.ExplicitSymbols[ಚಿಹ್ನೆ_Name] = ಚಿಹ್ನೆ_ಮೌಲ್ಯ;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *FileName,
                                                   std::string *Err) {
  ಸ್ವಯಂಚಾಲಿತ &G = getGlobals();
  ಶೂನ್ಯ *Handle = Handle_ಗಣ::DLOpen(FileName, Err);
  ಆದರೆ (Handle != &Invalid) {
    Smart_Scoped_ಬೀಗ<ನಿಜ> ಬೀಗ_3(G.SymbolsMutex);
    G.OpenedHandles.ಕೂಡಿಸು_Library(Handle, /*IsProcess*/ FileName == ಶೂನ್ಯಸೂಚಕ);
  }

  ಹಿಂತಿರುಗಿ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::ಕೂಡಿಸು_Permanent_Library(ಶೂನ್ಯ *Handle,
                                                   std::string *Err) {
  ಸ್ವಯಂಚಾಲಿತ &G = getGlobals();
  Smart_Scoped_ಬೀಗ<ನಿಜ> ಬೀಗ_3(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ಆದರೆ (!G.OpenedHandles.ಕೂಡಿಸು_Library(Handle, /*IsProcess*/ ಸುಳ್ಳು,
                                  /*CanClose*/ ಸುಳ್ಳು))
    *Err = "Library already loaded";

  ಹಿಂತಿರುಗಿ DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *FileName,
                                          std::string *Err) {
  ಪ್ರತಿಜ್ಞಾ(FileName && "Use getPermanentLibrary() for opening process handle");
  ಶೂನ್ಯ *Handle = Handle_ಗಣ::DLOpen(FileName, Err);
  ಆದರೆ (Handle != &Invalid) {
    ಸ್ವಯಂಚಾಲಿತ &G = getGlobals();
    Smart_Scoped_ಬೀಗ<ನಿಜ> ಬೀಗ_3(G.SymbolsMutex);
    G.OpenedTemporaryHandles.ಕೂಡಿಸು_Library(Handle, /*IsProcess*/ ಸುಳ್ಳು,
                                        /*CanClose*/ ಸುಳ್ಳು,
                                        /*AllowDuplicates*/ ನಿಜ);
  }
  ಹಿಂತಿರುಗಿ DynamicLibrary(Handle);
}

ಶೂನ್ಯ DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  ಸ್ವಯಂಚಾಲಿತ &G = getGlobals();
  Smart_Scoped_ಬೀಗ<ನಿಜ> ಬೀಗ_3(G.SymbolsMutex);
  ಆದರೆ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.ದತ್ತಾಂಶ_2);
    Lib.ದತ್ತಾಂಶ_2 = &Invalid;
  }
}

ಶೂನ್ಯ *DynamicLibrary::get_ವಿಳಾಸ_ದ_ಚಿಹ್ನೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಚಿಹ್ನೆ_Name) {
  ಆದರೆ (!isValid())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  ಹಿಂತಿರುಗಿ Handle_ಗಣ::DLSym(ದತ್ತಾಂಶ_2, ಚಿಹ್ನೆ_Name);
}

ಶೂನ್ಯ *DynamicLibrary::ಹುಡುಕು_For_ವಿಳಾಸ_ದ_ಚಿಹ್ನೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಚಿಹ್ನೆ_Name) {
  {
    ಸ್ವಯಂಚಾಲಿತ &G = getGlobals();
    Smart_Scoped_ಬೀಗ<ನಿಜ> ಬೀಗ_3(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<ಶೂನ್ಯ *>::iterator i = G.ExplicitSymbols.ಕಂಡುಹಿಡಿ(ಚಿಹ್ನೆ_Name);

    ಆದರೆ (i != G.ExplicitSymbols.ಕೊನೆ())
      ಹಿಂತಿರುಗಿ i->ಎರಡನೇ_2;

    // Now search the libraries.
    ಆದರೆ (ಶೂನ್ಯ *Ptr = G.OpenedHandles.Lookup(ಚಿಹ್ನೆ_Name, ಹುಡುಕು_ಕ್ರಮ))
      ಹಿಂತಿರುಗಿ Ptr;
    ಆದರೆ (ಶೂನ್ಯ *Ptr = G.OpenedTemporaryHandles.Lookup(ಚಿಹ್ನೆ_Name, ಹುಡುಕು_ಕ್ರಮ))
      ಹಿಂತಿರುಗಿ Ptr;
  }

  ಹಿಂತಿರುಗಿ llvm::ಹುಡುಕು_For_ವಿಳಾಸ_ದ_Special_ಚಿಹ್ನೆ(ಚಿಹ್ನೆ_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Filename) {
  ಹಿಂತಿರುಗಿ llvm::sys::DynamicLibrary::ತುಂಬು_Library_Permanently(Filename);
}

ಶೂನ್ಯ *LLVMSearch_For_ವಿಳಾಸ_ದ_ಚಿಹ್ನೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಚಿಹ್ನೆ_Name_2) {
  ಹಿಂತಿರುಗಿ llvm::sys::DynamicLibrary::ಹುಡುಕು_For_ವಿಳಾಸ_ದ_ಚಿಹ್ನೆ(ಚಿಹ್ನೆ_Name_2);
}

ಶೂನ್ಯ LLVMAdd_ಚಿಹ್ನೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಚಿಹ್ನೆ_Name_2, ಶೂನ್ಯ *ಚಿಹ್ನೆ_ಮೌಲ್ಯ_2) {
  ಹಿಂತಿರುಗಿ llvm::sys::DynamicLibrary::ಕೂಡಿಸು_ಚಿಹ್ನೆ(ಚಿಹ್ನೆ_Name_2, ಚಿಹ್ನೆ_ಮೌಲ್ಯ_2);
}
