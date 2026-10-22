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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
वर्ग DynamicLibrary::Handle_समुच्चयः {
  उपयुज्य HandleList = std::सदिशम्<शून्य *>;
  HandleList Handles;
  शून्य *Process = &Invalid;

सार्वजनिक:
  स्थिर शून्य *DLOpen(ध्रुव वर्ण *Filename, std::string *Err);
  स्थिर शून्य DLClose(शून्य *Handle);
  स्थिर शून्य *DLSym(शून्य *Handle, ध्रुव वर्ण *चिह्नम्);

  Handle_समुच्चयः() = स्वभाव;
  ~Handle_समुच्चयः();

  HandleList::iterator Find(शून्य *Handle) { प्रत्यागम find(Handles, Handle); }

  तार्किक अन्तर्भवति_2(शून्य *Handle) {
    प्रत्यागम Handle == Process || Find(Handle) != Handles.अन्तः();
  }

  तार्किक सङ्कलय_Library(शून्य *Handle, तार्किक IsProcess = असत्य, तार्किक CanClose = सत्य,
                  तार्किक AllowDuplicates = असत्य) {
#यदि_परिभाषितम् _WIN32
    प्रतिज्ञा((Handle == एतत् ? IsProcess : !IsProcess) && "Bad Handle.");
#यदि_अन्तः
    प्रतिज्ञा((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    यदि (LLVM_LIKELY(!IsProcess)) {
      यदि (!AllowDuplicates && Find(Handle) != Handles.अन्तः()) {
        यदि (CanClose)
          DLClose(Handle);
        प्रत्यागम असत्य;
      }
      Handles.प्रक्षिप_पृष्ठभागः(Handle);
    } अन्यथा {
#यदि_अपरिभाषितम् _WIN32
      यदि (Process != &Invalid) {
        यदि (CanClose)
          DLClose(Process);
        यदि (Process == Handle)
          प्रत्यागम असत्य;
      }
#यदि_अन्तः
      Process = Handle;
    }
    प्रत्यागम सत्य;
  }

  शून्य CloseLibrary(शून्य *Handle) {
    DLClose(Handle);
    HandleList::iterator it = Find(Handle);
    यदि (it != Handles.अन्तः()) {
      Handles.erase(it);
    }
  }

  शून्य *LibLookup(ध्रुव वर्ण *चिह्नम्, DynamicLibrary::अन्वेषणम्_Ordering क्रमः) {
    यदि (क्रमः & SO_आरोपय_क्रमः) {
      प्रति (शून्य *Handle : Handles) {
        यदि (शून्य *Ptr = DLSym(Handle, चिह्नम्))
          प्रत्यागम Ptr;
      }
    } अन्यथा {
      प्रति (शून्य *Handle : llvm::reverse(Handles)) {
        यदि (शून्य *Ptr = DLSym(Handle, चिह्नम्))
          प्रत्यागम Ptr;
      }
    }
    प्रत्यागम शून्यसूचकः;
  }

  शून्य *Lookup(ध्रुव वर्ण *चिह्नम्, DynamicLibrary::अन्वेषणम्_Ordering क्रमः) {
    प्रतिज्ञा(!((क्रमः & SO_LoadedFirst) && (क्रमः & SO_Loaded_चरमः)) &&
           "Invalid Ordering");

    यदि (Process == &Invalid || (क्रमः & SO_LoadedFirst)) {
      यदि (शून्य *Ptr = LibLookup(चिह्नम्, क्रमः))
        प्रत्यागम Ptr;
    }
    यदि (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      यदि (शून्य *Ptr = DLSym(Process, चिह्नम्))
        प्रत्यागम Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      यदि (क्रमः & SO_Loaded_चरमः) {
        यदि (शून्य *Ptr = LibLookup(चिह्नम्, क्रमः))
          प्रत्यागम Ptr;
      }
    }
    प्रत्यागम शून्यसूचकः;
  }
};

नामाकाश {

संरचना Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<शून्य *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::Handle_समुच्चयः OpenedHandles;
  DynamicLibrary::Handle_समुच्चयः OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<सत्य> SymbolsMutex;
};

Globals &getGlobals() {
  स्थिर Globals G;
  प्रत्यागम G;
}

} // namespace

#यदि_परिभाषितम् _WIN32

#अन्तर्भाव "Windows/DynamicLibrary.inc"

#अन्यथा

#अन्तर्भाव "Unix/DynamicLibrary.inc"

#यदि_अन्तः

वर्ण DynamicLibrary::Invalid;
DynamicLibrary::अन्वेषणम्_Ordering DynamicLibrary::अन्वेषणम्_क्रमः =
    DynamicLibrary::SO_Linker;

नामाकाश llvm {
शून्य *अन्वेषणम्_For_स्थानाङ्कः_सम्बन्धि_Special_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name) {
  प्रत्यागम Do_अन्वेषणम्(चिह्नम्_Name); // DynamicLibrary.inc
}
} // namespace llvm

शून्य DynamicLibrary::सङ्कलय_चिह्नम्(StringRef चिह्नम्_Name, शून्य *चिह्नम्_मानम्) {
  स्वचालित &G = getGlobals();
  Smart_Scoped_तालकम्<सत्य> तालकम्_2(G.SymbolsMutex);
  G.ExplicitSymbols[चिह्नम्_Name] = चिह्नम्_मानम्;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(ध्रुव वर्ण *FileName,
                                                   std::string *Err) {
  स्वचालित &G = getGlobals();
  शून्य *Handle = Handle_समुच्चयः::DLOpen(FileName, Err);
  यदि (Handle != &Invalid) {
    Smart_Scoped_तालकम्<सत्य> तालकम्_2(G.SymbolsMutex);
    G.OpenedHandles.सङ्कलय_Library(Handle, /*IsProcess*/ FileName == शून्यसूचकः);
  }

  प्रत्यागम DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::सङ्कलय_Permanent_Library(शून्य *Handle,
                                                   std::string *Err) {
  स्वचालित &G = getGlobals();
  Smart_Scoped_तालकम्<सत्य> तालकम्_2(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  यदि (!G.OpenedHandles.सङ्कलय_Library(Handle, /*IsProcess*/ असत्य,
                                  /*CanClose*/ असत्य))
    *Err = "Library already loaded";

  प्रत्यागम DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(ध्रुव वर्ण *FileName,
                                          std::string *Err) {
  प्रतिज्ञा(FileName && "Use getPermanentLibrary() for opening process handle");
  शून्य *Handle = Handle_समुच्चयः::DLOpen(FileName, Err);
  यदि (Handle != &Invalid) {
    स्वचालित &G = getGlobals();
    Smart_Scoped_तालकम्<सत्य> तालकम्_2(G.SymbolsMutex);
    G.OpenedTemporaryHandles.सङ्कलय_Library(Handle, /*IsProcess*/ असत्य,
                                        /*CanClose*/ असत्य,
                                        /*AllowDuplicates*/ सत्य);
  }
  प्रत्यागम DynamicLibrary(Handle);
}

शून्य DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  स्वचालित &G = getGlobals();
  Smart_Scoped_तालकम्<सत्य> तालकम्_2(G.SymbolsMutex);
  यदि (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.दत्त_2);
    Lib.दत्त_2 = &Invalid;
  }
}

शून्य *DynamicLibrary::get_स्थानाङ्कः_सम्बन्धि_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name) {
  यदि (!isValid())
    प्रत्यागम शून्यसूचकः;
  प्रत्यागम Handle_समुच्चयः::DLSym(दत्त_2, चिह्नम्_Name);
}

शून्य *DynamicLibrary::अन्वेषणम्_For_स्थानाङ्कः_सम्बन्धि_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name) {
  {
    स्वचालित &G = getGlobals();
    Smart_Scoped_तालकम्<सत्य> तालकम्_2(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<शून्य *>::iterator i = G.ExplicitSymbols.find(चिह्नम्_Name);

    यदि (i != G.ExplicitSymbols.अन्तः())
      प्रत्यागम i->second;

    // Now search the libraries.
    यदि (शून्य *Ptr = G.OpenedHandles.Lookup(चिह्नम्_Name, अन्वेषणम्_क्रमः))
      प्रत्यागम Ptr;
    यदि (शून्य *Ptr = G.OpenedTemporaryHandles.Lookup(चिह्नम्_Name, अन्वेषणम्_क्रमः))
      प्रत्यागम Ptr;
  }

  प्रत्यागम llvm::अन्वेषणम्_For_स्थानाङ्कः_सम्बन्धि_Special_चिह्नम्(चिह्नम्_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(ध्रुव वर्ण *Filename) {
  प्रत्यागम llvm::sys::DynamicLibrary::आरोपय_Library_Permanently(Filename);
}

शून्य *LLVMSearch_For_स्थानाङ्कः_सम्बन्धि_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name_2) {
  प्रत्यागम llvm::sys::DynamicLibrary::अन्वेषणम्_For_स्थानाङ्कः_सम्बन्धि_चिह्नम्(चिह्नम्_Name_2);
}

शून्य LLVMAdd_चिह्नम्(ध्रुव वर्ण *चिह्नम्_Name_2, शून्य *चिह्नम्_मानम्_2) {
  प्रत्यागम llvm::sys::DynamicLibrary::सङ्कलय_चिह्नम्(चिह्नम्_Name_2, चिह्नम्_मानम्_2);
}
