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

#అన్తర్భావ "llvm/Support/DynamicLibrary.h"
#అన్తర్భావ "llvm-c/Support.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/StringMap.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Mutex.h"
#అన్తర్భావ <vector>

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::sys;

// All methods for HandleSet should be used holding SymbolsMutex.
తరగతి DynamicLibrary::Handle_సమితి {
  ఉపయోగించి HandleList = std::సదిశ<శూన్య *>;
  HandleList Handles;
  శూన్య *Process = &Invalid;

బహిరంగ:
  స్థిర శూన్య *DLOpen(స్థిరరాశి అక్షరం *Filename, std::string *Err);
  స్థిర శూన్య DLClose(శూన్య *Handle);
  స్థిర శూన్య *DLSym(శూన్య *Handle, స్థిరరాశి అక్షరం *చిహ్నం);

  Handle_సమితి() = అప్రమేయ;
  ~Handle_సమితి();

  HandleList::iterator కనుగొను_2(శూన్య *Handle) { తిరిగి కనుగొను(Handles, Handle); }

  తార్కిక కలిగియుంది_2(శూన్య *Handle) {
    తిరిగి Handle == Process || కనుగొను_2(Handle) != Handles.చివర();
  }

  తార్కిక కూడు_Library(శూన్య *Handle, తార్కిక IsProcess = అబద్ధం, తార్కిక CanClose = నిజం,
                  తార్కిక AllowDuplicates = అబద్ధం) {
#నిర్వచితమైతే _WIN32
    ప్రతిజ్ఞా((Handle == ఇది ? IsProcess : !IsProcess) && "Bad Handle.");
#అయితే_ముగింపు
    ప్రతిజ్ఞా((!AllowDuplicates || !CanClose) &&
           "CanClose must be false if AllowDuplicates is true.");

    ఒకవేళ (LLVM_LIKELY(!IsProcess)) {
      ఒకవేళ (!AllowDuplicates && కనుగొను_2(Handle) != Handles.చివర()) {
        ఒకవేళ (CanClose)
          DLClose(Handle);
        తిరిగి అబద్ధం;
      }
      Handles.నెట్టు_వెనుక(Handle);
    } లేకపోతే {
#నిర్వచితం_కాకపోతే _WIN32
      ఒకవేళ (Process != &Invalid) {
        ఒకవేళ (CanClose)
          DLClose(Process);
        ఒకవేళ (Process == Handle)
          తిరిగి అబద్ధం;
      }
#అయితే_ముగింపు
      Process = Handle;
    }
    తిరిగి నిజం;
  }

  శూన్య CloseLibrary(శూన్య *Handle) {
    DLClose(Handle);
    HandleList::iterator it = కనుగొను_2(Handle);
    ఒకవేళ (it != Handles.చివర()) {
      Handles.erase(it);
    }
  }

  శూన్య *LibLookup(స్థిరరాశి అక్షరం *చిహ్నం, DynamicLibrary::శోధన_Ordering క్రమం) {
    ఒకవేళ (క్రమం & SO_ఎక్కించు_క్రమం) {
      కొరకు (శూన్య *Handle : Handles) {
        ఒకవేళ (శూన్య *Ptr = DLSym(Handle, చిహ్నం))
          తిరిగి Ptr;
      }
    } లేకపోతే {
      కొరకు (శూన్య *Handle : llvm::తిరగేయి(Handles)) {
        ఒకవేళ (శూన్య *Ptr = DLSym(Handle, చిహ్నం))
          తిరిగి Ptr;
      }
    }
    తిరిగి శూన్యసూచిక;
  }

  శూన్య *Lookup(స్థిరరాశి అక్షరం *చిహ్నం, DynamicLibrary::శోధన_Ordering క్రమం) {
    ప్రతిజ్ఞా(!((క్రమం & SO_Loaded_మొదటి) && (క్రమం & SO_Loaded_చివరి)) &&
           "Invalid Ordering");

    ఒకవేళ (Process == &Invalid || (క్రమం & SO_Loaded_మొదటి)) {
      ఒకవేళ (శూన్య *Ptr = LibLookup(చిహ్నం, క్రమం))
        తిరిగి Ptr;
    }
    ఒకవేళ (Process != &Invalid) {
      // Use OS facilities to search the current binary and all loaded libs.
      ఒకవేళ (శూన్య *Ptr = DLSym(Process, చిహ్నం))
        తిరిగి Ptr;

      // Search any libs that might have been skipped because of RTLD_LOCAL.
      ఒకవేళ (క్రమం & SO_Loaded_చివరి) {
        ఒకవేళ (శూన్య *Ptr = LibLookup(చిహ్నం, క్రమం))
          తిరిగి Ptr;
      }
    }
    తిరిగి శూన్యసూచిక;
  }
};

నామాకాశ {

నిర్మాణం Globals {
  // Collection of symbol name/value pairs to be searched prior to any
  // libraries.
  llvm::StringMap<శూన్య *> ExplicitSymbols;
  // Collections of known library handles.
  DynamicLibrary::Handle_సమితి OpenedHandles;
  DynamicLibrary::Handle_సమితి OpenedTemporaryHandles;
  // Lock for ExplicitSymbols, OpenedHandles, and OpenedTemporaryHandles.
  llvm::sys::SmartMutex<నిజం> SymbolsMutex;
};

Globals &getGlobals() {
  స్థిర Globals G;
  తిరిగి G;
}

} // namespace

#నిర్వచితమైతే _WIN32

#అన్తర్భావ "Windows/DynamicLibrary.inc"

#లేకపోతే

#అన్తర్భావ "Unix/DynamicLibrary.inc"

#అయితే_ముగింపు

అక్షరం DynamicLibrary::Invalid;
DynamicLibrary::శోధన_Ordering DynamicLibrary::శోధన_క్రమం =
    DynamicLibrary::SO_Linker;

నామాకాశ llvm {
శూన్య *శోధన_For_చిరునామా_యొక్క_Special_చిహ్నం(స్థిరరాశి అక్షరం *చిహ్నం_Name) {
  తిరిగి Do_శోధన(చిహ్నం_Name); // DynamicLibrary.inc
}
} // namespace llvm

శూన్య DynamicLibrary::కూడు_చిహ్నం(StringRef చిహ్నం_Name, శూన్య *చిహ్నం_విలువ) {
  స్వయంచాలక &G = getGlobals();
  Smart_Scoped_తాళం<నిజం> తాళం_3(G.SymbolsMutex);
  G.ExplicitSymbols[చిహ్నం_Name] = చిహ్నం_విలువ;
}

DynamicLibrary DynamicLibrary::getPermanentLibrary(స్థిరరాశి అక్షరం *FileName,
                                                   std::string *Err) {
  స్వయంచాలక &G = getGlobals();
  శూన్య *Handle = Handle_సమితి::DLOpen(FileName, Err);
  ఒకవేళ (Handle != &Invalid) {
    Smart_Scoped_తాళం<నిజం> తాళం_3(G.SymbolsMutex);
    G.OpenedHandles.కూడు_Library(Handle, /*IsProcess*/ FileName == శూన్యసూచిక);
  }

  తిరిగి DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::కూడు_Permanent_Library(శూన్య *Handle,
                                                   std::string *Err) {
  స్వయంచాలక &G = getGlobals();
  Smart_Scoped_తాళం<నిజం> తాళం_3(G.SymbolsMutex);
  // If we've already loaded this library, tell the caller.
  ఒకవేళ (!G.OpenedHandles.కూడు_Library(Handle, /*IsProcess*/ అబద్ధం,
                                  /*CanClose*/ అబద్ధం))
    *Err = "Library already loaded";

  తిరిగి DynamicLibrary(Handle);
}

DynamicLibrary DynamicLibrary::getLibrary(స్థిరరాశి అక్షరం *FileName,
                                          std::string *Err) {
  ప్రతిజ్ఞా(FileName && "Use getPermanentLibrary() for opening process handle");
  శూన్య *Handle = Handle_సమితి::DLOpen(FileName, Err);
  ఒకవేళ (Handle != &Invalid) {
    స్వయంచాలక &G = getGlobals();
    Smart_Scoped_తాళం<నిజం> తాళం_3(G.SymbolsMutex);
    G.OpenedTemporaryHandles.కూడు_Library(Handle, /*IsProcess*/ అబద్ధం,
                                        /*CanClose*/ అబద్ధం,
                                        /*AllowDuplicates*/ నిజం);
  }
  తిరిగి DynamicLibrary(Handle);
}

శూన్య DynamicLibrary::closeLibrary(DynamicLibrary &Lib) {
  స్వయంచాలక &G = getGlobals();
  Smart_Scoped_తాళం<నిజం> తాళం_3(G.SymbolsMutex);
  ఒకవేళ (Lib.isValid()) {
    G.OpenedTemporaryHandles.CloseLibrary(Lib.దత్తాంశం_2);
    Lib.దత్తాంశం_2 = &Invalid;
  }
}

శూన్య *DynamicLibrary::get_చిరునామా_యొక్క_చిహ్నం(స్థిరరాశి అక్షరం *చిహ్నం_Name) {
  ఒకవేళ (!isValid())
    తిరిగి శూన్యసూచిక;
  తిరిగి Handle_సమితి::DLSym(దత్తాంశం_2, చిహ్నం_Name);
}

శూన్య *DynamicLibrary::శోధన_For_చిరునామా_యొక్క_చిహ్నం(స్థిరరాశి అక్షరం *చిహ్నం_Name) {
  {
    స్వయంచాలక &G = getGlobals();
    Smart_Scoped_తాళం<నిజం> తాళం_3(G.SymbolsMutex);

    // First check symbols added via AddSymbol().
    StringMap<శూన్య *>::iterator i = G.ExplicitSymbols.కనుగొను(చిహ్నం_Name);

    ఒకవేళ (i != G.ExplicitSymbols.చివర())
      తిరిగి i->రెండవ_2;

    // Now search the libraries.
    ఒకవేళ (శూన్య *Ptr = G.OpenedHandles.Lookup(చిహ్నం_Name, శోధన_క్రమం))
      తిరిగి Ptr;
    ఒకవేళ (శూన్య *Ptr = G.OpenedTemporaryHandles.Lookup(చిహ్నం_Name, శోధన_క్రమం))
      తిరిగి Ptr;
  }

  తిరిగి llvm::శోధన_For_చిరునామా_యొక్క_Special_చిహ్నం(చిహ్నం_Name);
}

//===----------------------------------------------------------------------===//
// C API.
//===----------------------------------------------------------------------===//

LLVMBool LLVMLoadLibraryPermanently(స్థిరరాశి అక్షరం *Filename) {
  తిరిగి llvm::sys::DynamicLibrary::ఎక్కించు_Library_Permanently(Filename);
}

శూన్య *LLVMSearch_For_చిరునామా_యొక్క_చిహ్నం(స్థిరరాశి అక్షరం *చిహ్నం_Name_2) {
  తిరిగి llvm::sys::DynamicLibrary::శోధన_For_చిరునామా_యొక్క_చిహ్నం(చిహ్నం_Name_2);
}

శూన్య LLVMAdd_చిహ్నం(స్థిరరాశి అక్షరం *చిహ్నం_Name_2, శూన్య *చిహ్నం_విలువ_2) {
  తిరిగి llvm::sys::DynamicLibrary::కూడు_చిహ్నం(చిహ్నం_Name_2, చిహ్నం_విలువ_2);
}
