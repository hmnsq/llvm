//===- SourceMgr.cpp - Manager for Simple Source Buffers & Diagnostics ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SourceMgr class.  This class is used as a simple
// substrate for diagnostics, #include handling, and other low level things for
// simple parsers.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/SourceMgr.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/Twine.h"
#అన్తర్భావ "llvm/Support/ErrorOr.h"
#అన్తర్భావ "llvm/Support/Locale.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/Support/SMLoc.h"
#అన్తర్భావ "llvm/Support/VirtualFileSystem.h"
#అన్తర్భావ "llvm/Support/WithColor.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <algorithm>
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstddef>
#అన్తర్భావ <limits>
#అన్తర్భావ <memory>
#అన్తర్భావ <string>
#అన్తర్భావ <utility>

ఉపయోగించి నామాకాశ llvm;

స్థిర స్థిరరాశి size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
ఆధారం_Mgr::ఆధారం_Mgr() = అప్రమేయ;
ఆధారం_Mgr::ఆధారం_Mgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
ఆధారం_Mgr::ఆధారం_Mgr(ఆధారం_Mgr &&) = అప్రమేయ;
ఆధారం_Mgr &ఆధారం_Mgr::క్రియాకారకం=(ఆధారం_Mgr &&) = అప్రమేయ;
ఆధారం_Mgr::~ఆధారం_Mgr() = అప్రమేయ;

IntrusiveRefCntPtr<vfs::FileSystem> ఆధారం_Mgr::getVirtualFileSystem() స్థిరరాశి {
  తిరిగి FS;
}

శూన్య ఆధారం_Mgr::సమితి_Virtual_File_System(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  ఇది->FS = std::move(FS);
}

గుర్తులేని ఆధారం_Mgr::కూడు_Include_File(స్థిరరాశి std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  ఒకవేళ (!NewBufOrErr)
    తిరిగి 0;

  తిరిగి కూడు_New_ఆధారం_బఫరు(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>>
ఆధారం_Mgr::OpenIncludeFile(స్థిరరాశి std::string &Filename,
                           std::string &IncludedFile,
                           తార్కిక RequiresNullTerminator) {
  స్వయంచాలక GetFile = [ఇది, RequiresNullTerminator](StringRef మార్గం) {
    తిరిగి FS ? FS->get_బఫరు_For_File(మార్గం, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : జ్ఞాపకం_బఫరు::getFile(మార్గం, /*IsText=*/అబద్ధం,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<జ్ఞాపకం_బఫరు>> NewBufOrErr = GetFile(Filename);

  SmallString<64> బఫరు(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  కొరకు (గుర్తులేని i = 0, e = IncludeDirectories.కొలత_2(); i != e && !NewBufOrErr;
       ++i) {
    బఫరు = IncludeDirectories[i];
    sys::మార్గం_2::append(బఫరు, Filename);
    NewBufOrErr = GetFile(బఫరు);
  }

  ఒకవేళ (NewBufOrErr)
    IncludedFile = static_cast<std::string>(బఫరు);

  తిరిగి NewBufOrErr;
}

గుర్తులేని ఆధారం_Mgr::కనుగొను_బఫరు_Containing_Loc(SMLoc Loc) స్థిరరాశి {
  కొరకు (గుర్తులేని i = 0, e = Buffers.కొలత_2(); i != e; ++i)
    ఒకవేళ (Loc.get_సూచిక() >= Buffers[i].బఫరు->get_బఫరు_ప్రారంభం() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.get_సూచిక() <= Buffers[i].బఫరు->get_బఫరు_చివర())
      తిరిగి i + 1;
  తిరిగి 0;
}

మూస <రకం_పేరు T>
స్థిర std::సదిశ<T> &Get_Or_Create_స్థానభ్రంశం_Cache(శూన్య *&స్థానభ్రంశం_Cache,
                                              జ్ఞాపకం_బఫరు *బఫరు) {
  ఒకవేళ (స్థానభ్రంశం_Cache)
    తిరిగి *static_cast<std::సదిశ<T> *>(స్థానభ్రంశం_Cache);

  // Lazily fill in the offset cache.
  స్వయంచాలక *Offsets = కొత్త std::సదిశ<T>();
  size_t Sz = బఫరు->get_బఫరు_కొలత();
  ప్రతిజ్ఞా(Sz <= std::numeric_limits<T>::max());
  StringRef S = బఫరు->get_బఫరు();
  కొరకు (size_t N = 0; N < Sz; ++N) {
    ఒకవేళ (S[N] == '\n')
      Offsets->నెట్టు_వెనుక(static_cast<T>(N));
  }

  స్థానభ్రంశం_Cache = Offsets;
  తిరిగి *Offsets;
}

మూస <రకం_పేరు T>
std::pair<గుర్తులేని, గుర్తులేని>
ఆధారం_Mgr::Src_బఫరు::get_పంక్తి_And_స్తంభం_Specialized(స్థిరరాశి అక్షరం *Ptr) స్థిరరాశి {
  std::సదిశ<T> &Offsets =
      Get_Or_Create_స్థానభ్రంశం_Cache<T>(స్థానభ్రంశం_Cache, బఫరు.get());

  స్థిరరాశి అక్షరం *Buf_ప్రారంభం = బఫరు->get_బఫరు_ప్రారంభం();
  ప్రతిజ్ఞా(Ptr >= Buf_ప్రారంభం && Ptr <= బఫరు->get_బఫరు_చివర());
  ptrdiff_t PtrDiff = Ptr - Buf_ప్రారంభం;
  ప్రతిజ్ఞా(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T Ptr_స్థానభ్రంశం = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  స్వయంచాలక I = llvm::క్రింది_bound(Offsets, Ptr_స్థానభ్రంశం);
  గుర్తులేని పంక్తి_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T పంక్తి_ప్రారంభం_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  గుర్తులేని నిలువు_No = Ptr_స్థానభ్రంశం - పంక్తి_ప్రారంభం_Offs + 1;
  తిరిగి {పంక్తి_No, నిలువు_No};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<గుర్తులేని, గుర్తులేని>
ఆధారం_Mgr::Src_బఫరు::get_పంక్తి_And_స్తంభం(స్థిరరాశి అక్షరం *Ptr) స్థిరరాశి {
  size_t Sz = బఫరు->get_బఫరు_కొలత();
  ఒకవేళ (Sz <= std::numeric_limits<uint8_t>::max())
    తిరిగి get_పంక్తి_And_స్తంభం_Specialized<uint8_t>(Ptr);
  ఒకవేళ (Sz <= std::numeric_limits<uint16_t>::max())
    తిరిగి get_పంక్తి_And_స్తంభం_Specialized<uint16_t>(Ptr);
  ఒకవేళ (Sz <= std::numeric_limits<uint32_t>::max())
    తిరిగి get_పంక్తి_And_స్తంభం_Specialized<uint32_t>(Ptr);
  తిరిగి get_పంక్తి_And_స్తంభం_Specialized<uint64_t>(Ptr);
}

మూస <రకం_పేరు T>
స్థిరరాశి అక్షరం *ఆధారం_Mgr::Src_బఫరు::get_సూచిక_For_పంక్తి_సంఖ్య_Specialized(
    గుర్తులేని పంక్తి_No) స్థిరరాశి {
  std::సదిశ<T> &Offsets =
      Get_Or_Create_స్థానభ్రంశం_Cache<T>(స్థానభ్రంశం_Cache, బఫరు.get());

  // We start counting line and column numbers from 1.
  ఒకవేళ (పంక్తి_No != 0)
    --పంక్తి_No;

  స్థిరరాశి అక్షరం *Buf_ప్రారంభం = బఫరు->get_బఫరు_ప్రారంభం();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  ఒకవేళ (పంక్తి_No == 0)
    తిరిగి Buf_ప్రారంభం;
  ఒకవేళ (పంక్తి_No > Offsets.కొలత_2())
    తిరిగి శూన్యసూచిక;
  తిరిగి Buf_ప్రారంభం + Offsets[పంక్తి_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
స్థిరరాశి అక్షరం *
ఆధారం_Mgr::Src_బఫరు::get_సూచిక_For_పంక్తి_సంఖ్య(గుర్తులేని పంక్తి_No) స్థిరరాశి {
  size_t Sz = బఫరు->get_బఫరు_కొలత();
  ఒకవేళ (Sz <= std::numeric_limits<uint8_t>::max())
    తిరిగి get_సూచిక_For_పంక్తి_సంఖ్య_Specialized<uint8_t>(పంక్తి_No);
  లేకపోతే ఒకవేళ (Sz <= std::numeric_limits<uint16_t>::max())
    తిరిగి get_సూచిక_For_పంక్తి_సంఖ్య_Specialized<uint16_t>(పంక్తి_No);
  లేకపోతే ఒకవేళ (Sz <= std::numeric_limits<uint32_t>::max())
    తిరిగి get_సూచిక_For_పంక్తి_సంఖ్య_Specialized<uint32_t>(పంక్తి_No);
  లేకపోతే
    తిరిగి get_సూచిక_For_పంక్తి_సంఖ్య_Specialized<uint64_t>(పంక్తి_No);
}

ఆధారం_Mgr::Src_బఫరు::Src_బఫరు(ఆధారం_Mgr::Src_బఫరు &&Other)
    : బఫరు(std::move(Other.బఫరు)), స్థానభ్రంశం_Cache(Other.స్థానభ్రంశం_Cache),
      IncludeLoc(Other.IncludeLoc) {
  Other.స్థానభ్రంశం_Cache = శూన్యసూచిక;
}

ఆధారం_Mgr::Src_బఫరు::~Src_బఫరు() {
  ఒకవేళ (స్థానభ్రంశం_Cache) {
    size_t Sz = బఫరు->get_బఫరు_కొలత();
    ఒకవేళ (Sz <= std::numeric_limits<uint8_t>::max())
      తొలగించు static_cast<std::సదిశ<uint8_t> *>(స్థానభ్రంశం_Cache);
    లేకపోతే ఒకవేళ (Sz <= std::numeric_limits<uint16_t>::max())
      తొలగించు static_cast<std::సదిశ<uint16_t> *>(స్థానభ్రంశం_Cache);
    లేకపోతే ఒకవేళ (Sz <= std::numeric_limits<uint32_t>::max())
      తొలగించు static_cast<std::సదిశ<uint32_t> *>(స్థానభ్రంశం_Cache);
    లేకపోతే
      తొలగించు static_cast<std::సదిశ<uint64_t> *>(స్థానభ్రంశం_Cache);
    స్థానభ్రంశం_Cache = శూన్యసూచిక;
  }
}

std::pair<గుర్తులేని, గుర్తులేని>
ఆధారం_Mgr::get_పంక్తి_And_స్తంభం(SMLoc Loc, గుర్తులేని బఫరు_ID) స్థిరరాశి {
  ఒకవేళ (!బఫరు_ID)
    బఫరు_ID = కనుగొను_బఫరు_Containing_Loc(Loc);
  ప్రతిజ్ఞా(బఫరు_ID && "Invalid location!");

  స్వయంచాలక &SB = get_బఫరు_Info(బఫరు_ID);
  స్థిరరాశి అక్షరం *Ptr = Loc.get_సూచిక();

  తిరిగి SB.get_పంక్తి_And_స్తంభం(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string ఆధారం_Mgr::get_Formatted_Location_No_స్థానభ్రంశం(SMLoc Loc,
                                                    తార్కిక Include_మార్గం) స్థిరరాశి {
  స్వయంచాలక బఫరు_ID = కనుగొను_బఫరు_Containing_Loc(Loc);
  ప్రతిజ్ఞా(బఫరు_ID && "Invalid location!");
  స్వయంచాలక FileSpec = get_బఫరు_Info(బఫరు_ID).బఫరు->get_బఫరు_Identifier();

  ఒకవేళ (Include_మార్గం) {
    తిరిగి FileSpec.str() + ":" + std::గమ్యం_string(కనుగొను_పంక్తి_సంఖ్య(Loc, బఫరు_ID));
  } లేకపోతే {
    స్వయంచాలక I = FileSpec.కనుగొను_చివరి_యొక్క("/\\");
    I = (I == FileSpec.కొలత_2()) ? 0 : (I + 1);
    తిరిగి FileSpec.substr(I).str() + ":" +
           std::గమ్యం_string(కనుగొను_పంక్తి_సంఖ్య(Loc, బఫరు_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc ఆధారం_Mgr::కనుగొను_Loc_For_పంక్తి_And_స్తంభం(గుర్తులేని బఫరు_ID, గుర్తులేని పంక్తి_No,
                                         గుర్తులేని నిలువు_No) {
  స్వయంచాలక &SB = get_బఫరు_Info(బఫరు_ID);
  స్థిరరాశి అక్షరం *Ptr = SB.get_సూచిక_For_పంక్తి_సంఖ్య(పంక్తి_No);
  ఒకవేళ (!Ptr)
    తిరిగి SMLoc();

  // We start counting line and column numbers from 1.
  ఒకవేళ (నిలువు_No != 0)
    --నిలువు_No;

  // If we have a column number, validate it.
  ఒకవేళ (నిలువు_No) {
    // Make sure the location is within the current line.
    ఒకవేళ (Ptr + నిలువు_No > SB.బఫరు->get_బఫరు_చివర())
      తిరిగి SMLoc();

    // Make sure there is no newline in the way.
    ఒకవేళ (StringRef(Ptr, నిలువు_No).కనుగొను_మొదటి_యొక్క("\n\r") != StringRef::npos)
      తిరిగి SMLoc();

    Ptr += నిలువు_No;
  }

  తిరిగి SMLoc::get_From_సూచిక(Ptr);
}

శూన్య ఆధారం_Mgr::Print_Include_దొంతర(SMLoc IncludeLoc, raw_ostream &OS) స్థిరరాశి {
  ఒకవేళ (IncludeLoc == SMLoc())
    తిరిగి; // Top of stack.

  గుర్తులేని CurBuf = కనుగొను_బఫరు_Containing_Loc(IncludeLoc);
  ప్రతిజ్ఞా(CurBuf && "Invalid or unspecified location!");

  Print_Include_దొంతర(get_బఫరు_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_బఫరు_Info(CurBuf).బఫరు->get_బఫరు_Identifier()
     << ":" << కనుగొను_పంక్తి_సంఖ్య(IncludeLoc, CurBuf) << ":\n";
}

శూన్య ఆధారం_Mgr::print_Include_దొంతర_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) స్థిరరాశి {
  ఒకవేళ (!Loc.isValid())
    తిరిగి;
  గుర్తులేని Diag_Cur_బఫరు = కనుగొను_బఫరు_Containing_Loc(Loc);
  ఒకవేళ (Diag_Cur_బఫరు && Diag_Cur_బఫరు != getMainFileID()) {
    SMLoc మాతృ_Include_Loc = get_మాతృ_Include_Loc(Diag_Cur_బఫరు);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_దొంతర(మాతృ_Include_Loc, OS);
  }
}

SMDiagnostic ఆధారం_Mgr::Get_సందేశం(SMLoc Loc, ఆధారం_Mgr::Diag_రకం రకం_3,
                                   స్థిరరాశి Twine &సందేశం_4, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) స్థిరరాశి {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  Small_సదిశ<std::pair<గుర్తులేని, గుర్తులేని>, 4> నిలువు_Ranges;
  std::pair<గుర్తులేని, గుర్తులేని> పంక్తి_And_నిలువు;
  StringRef బఫరు_ID = "<unknown>";
  StringRef పంక్తి_Str;

  ఒకవేళ (Loc.isValid()) {
    గుర్తులేని CurBuf = కనుగొను_బఫరు_Containing_Loc(Loc);
    ప్రతిజ్ఞా(CurBuf && "Invalid or unspecified location!");

    స్థిరరాశి జ్ఞాపకం_బఫరు *CurMB = get_జ్ఞాపకం_బఫరు(CurBuf);
    బఫరు_ID = CurMB->get_బఫరు_Identifier();

    // Scan backward to find the start of the line.
    స్థిరరాశి అక్షరం *పంక్తి_ప్రారంభం = Loc.get_సూచిక();
    స్థిరరాశి అక్షరం *Buf_ప్రారంభం = CurMB->get_బఫరు_ప్రారంభం();
    అయితే (పంక్తి_ప్రారంభం != Buf_ప్రారంభం && పంక్తి_ప్రారంభం[-1] != '\n' &&
           పంక్తి_ప్రారంభం[-1] != '\r')
      --పంక్తి_ప్రారంభం;

    // Get the end of the line.
    స్థిరరాశి అక్షరం *పంక్తి_చివర = Loc.get_సూచిక();
    స్థిరరాశి అక్షరం *Buf_చివర = CurMB->get_బఫరు_చివర();
    అయితే (పంక్తి_చివర != Buf_చివర && పంక్తి_చివర[0] != '\n' && పంక్తి_చివర[0] != '\r')
      ++పంక్తి_చివర;
    పంక్తి_Str = StringRef(పంక్తి_ప్రారంభం, పంక్తి_చివర - పంక్తి_ప్రారంభం);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    కొరకు (SMRange R : Ranges) {
      ఒకవేళ (!R.isValid())
        కొనసాగు;

      // If the line doesn't contain any part of the range, then ignore it.
      ఒకవేళ (R.ప్రారంభం.get_సూచిక() > పంక్తి_చివర || R.చివర_2.get_సూచిక() < పంక్తి_ప్రారంభం)
        కొనసాగు;

      // Ignore pieces of the range that go onto other lines.
      ఒకవేళ (R.ప్రారంభం.get_సూచిక() < పంక్తి_ప్రారంభం)
        R.ప్రారంభం = SMLoc::get_From_సూచిక(పంక్తి_ప్రారంభం);
      ఒకవేళ (R.చివర_2.get_సూచిక() > పంక్తి_చివర)
        R.చివర_2 = SMLoc::get_From_సూచిక(పంక్తి_చివర);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      నిలువు_Ranges.నెట్టు_వెనుక(std::make_pair(R.ప్రారంభం.get_సూచిక() - పంక్తి_ప్రారంభం,
                                         R.చివర_2.get_సూచిక() - పంక్తి_ప్రారంభం));
    }

    పంక్తి_And_నిలువు = get_పంక్తి_And_స్తంభం(Loc, CurBuf);
  }

  తిరిగి SMDiagnostic(*ఇది, Loc, బఫరు_ID, పంక్తి_And_నిలువు.మొదటి_2,
                      పంక్తి_And_నిలువు.రెండవ_2 - 1, రకం_3, సందేశం_4.str(), పంక్తి_Str,
                      నిలువు_Ranges, FixIts);
}

శూన్య ఆధారం_Mgr::Print_సందేశం(raw_ostream &OS, స్థిరరాశి SMDiagnostic &Diagnostic,
                             తార్కిక ShowColors) స్థిరరాశి {
  // Report the message with the diagnostic handler if present.
  ఒకవేళ (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    తిరిగి;
  }

  ఒకవేళ (Diagnostic.getLoc().isValid()) {
    గుర్తులేని CurBuf = కనుగొను_బఫరు_Containing_Loc(Diagnostic.getLoc());
    ప్రతిజ్ఞా(CurBuf && "Invalid or unspecified location!");
    Print_Include_దొంతర(get_బఫరు_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(శూన్యసూచిక, OS, ShowColors);
}

శూన్య ఆధారం_Mgr::Print_సందేశం(raw_ostream &OS, SMLoc Loc,
                             ఆధారం_Mgr::Diag_రకం రకం_3, స్థిరరాశి Twine &సందేశం_4,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             తార్కిక ShowColors) స్థిరరాశి {
  Print_సందేశం(OS, Get_సందేశం(Loc, రకం_3, సందేశం_4, Ranges, FixIts), ShowColors);
}

శూన్య ఆధారం_Mgr::Print_సందేశం(SMLoc Loc, ఆధారం_Mgr::Diag_రకం రకం_3,
                             స్థిరరాశి Twine &సందేశం_4, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, తార్కిక ShowColors) స్థిరరాశి {
  Print_సందేశం(errs(), Loc, రకం_3, సందేశం_4, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, స్థిరరాశి Twine &Replacement)
    : Range(R), పాఠ్యం_2(Replacement.str()) {
  ప్రతిజ్ఞా(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(స్థిరరాశి ఆధారం_Mgr &sm, SMLoc L, StringRef FN, పూర్ణాంకం పంక్తి,
                           పూర్ణాంకం నిలువు, ఆధారం_Mgr::Diag_రకం రకం_3, StringRef సందేశం_4,
                           StringRef పంక్తి_Str,
                           ArrayRef<std::pair<గుర్తులేని, గుర్తులేని>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), పంక్తి_No(పంక్తి), స్తంభం_No(నిలువు),
      రకం_3(రకం_3), సందేశం_3(సందేశం_4), పంక్తి_Contents(పంక్తి_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::క్రమీకరించు(FixIts);
}

స్థిర శూన్య build_Fix_It_పంక్తి(std::string &Caret_పంక్తి, std::string &Fix_It_పంక్తి,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<అక్షరం> ఆధారం_పంక్తి) {
  ఒకవేళ (FixIts.ఖాళీలు())
    తిరిగి;

  స్థిరరాశి అక్షరం *పంక్తి_ప్రారంభం = ఆధారం_పంక్తి.begin();
  స్థిరరాశి అక్షరం *పంక్తి_చివర = ఆధారం_పంక్తి.చివర();

  size_t Prev_Hint_చివర_నిలువు = 0;

  కొరకు (స్థిరరాశి llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    ఒకవేళ (Fixit.get_పాఠ్యం().కనుగొను_మొదటి_యొక్క("\n\r\t") != StringRef::npos)
      కొనసాగు;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    ఒకవేళ (R.ప్రారంభం.get_సూచిక() > పంక్తి_చివర || R.చివర_2.get_సూచిక() < పంక్తి_ప్రారంభం)
      కొనసాగు;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    గుర్తులేని మొదటి_నిలువు;
    ఒకవేళ (R.ప్రారంభం.get_సూచిక() < పంక్తి_ప్రారంభం)
      మొదటి_నిలువు = 0;
    లేకపోతే
      మొదటి_నిలువు = R.ప్రారంభం.get_సూచిక() - పంక్తి_ప్రారంభం;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    గుర్తులేని Hint_నిలువు = మొదటి_నిలువు;
    ఒకవేళ (Hint_నిలువు < Prev_Hint_చివర_నిలువు)
      Hint_నిలువు = Prev_Hint_చివర_నిలువు + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    ప్రతిజ్ఞా((size_t)sys::locale::స్తంభం_వెడల్పు(Fixit.get_పాఠ్యం()) ==
           Fixit.get_పాఠ్యం().కొలత_2());

    // This relies on one byte per column in our fixit hints.
    గుర్తులేని చివరి_స్తంభం_Modified = Hint_నిలువు + Fixit.get_పాఠ్యం().కొలత_2();
    ఒకవేళ (చివరి_స్తంభం_Modified > Fix_It_పంక్తి.కొలత_2())
      Fix_It_పంక్తి.resize(చివరి_స్తంభం_Modified, ' ');

    llvm::copy(Fixit.get_పాఠ్యం(), Fix_It_పంక్తి.begin() + Hint_నిలువు);

    Prev_Hint_చివర_నిలువు = చివరి_స్తంభం_Modified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    గుర్తులేని చివరి_నిలువు;
    ఒకవేళ (R.చివర_2.get_సూచిక() >= పంక్తి_చివర)
      చివరి_నిలువు = పంక్తి_చివర - పంక్తి_ప్రారంభం;
    లేకపోతే
      చివరి_నిలువు = R.చివర_2.get_సూచిక() - పంక్తి_ప్రారంభం;

    std::fill(&Caret_పంక్తి[మొదటి_నిలువు], &Caret_పంక్తి[చివరి_నిలువు], '~');
  }
}

స్థిర శూన్య print_ఆధారం_పంక్తి(raw_ostream &S, StringRef పంక్తి_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  కొరకు (గుర్తులేని i = 0, e = పంక్తి_Contents.కొలత_2(), బహిర్గతం_నిలువు = 0; i != e; ++i) {
    size_t తదుపరి_Tab = పంక్తి_Contents.కనుగొను('\t', i);
    // If there were no tabs left, print the rest, we are done.
    ఒకవేళ (తదుపరి_Tab == StringRef::npos) {
      S << పంక్తి_Contents.drop_front(i);
      ఆపు;
    }

    // Otherwise, print from i to NextTab.
    S << పంక్తి_Contents.ముక్క(i, తదుపరి_Tab);
    బహిర్గతం_నిలువు += తదుపరి_Tab - i;
    i = తదుపరి_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    చేయి {
      S << ' ';
      ++బహిర్గతం_నిలువు;
    } అయితే ((బహిర్గతం_నిలువు % TabStop) != 0);
  }
  S << '\n';
}

స్థిర తార్కిక isNonASCII(అక్షరం c) { తిరిగి c & 0x80; }

శూన్య SMDiagnostic::print(స్థిరరాశి అక్షరం *ProgName, raw_ostream &OS, తార్కిక ShowColors,
                         తార్కిక Show_రకం_Label, తార్కిక ShowLocation) స్థిరరాశి {
  Color_బహుళకం బహుళకం = ShowColors ? Color_బహుళకం::Auto : Color_బహుళకం::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, నిజం, అబద్ధం, బహుళకం);

    ఒకవేళ (ProgName && ProgName[0])
      S << ProgName << ": ";

    ఒకవేళ (ShowLocation && !Filename.ఖాళీలు()) {
      ఒకవేళ (Filename == "-")
        S << "<stdin>";
      లేకపోతే
        S << Filename;

      ఒకవేళ (పంక్తి_No != -1) {
        S << ':' << పంక్తి_No;
        ఒకవేళ (స్తంభం_No != -1)
          S << ':' << (స్తంభం_No + 1);
      }
      S << ": ";
    }
  }

  ఒకవేళ (Show_రకం_Label) {
    ఎంపిక (రకం_3) {
    స్థితి ఆధారం_Mgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      ఆపు;
    స్థితి ఆధారం_Mgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      ఆపు;
    స్థితి ఆధారం_Mgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      ఆపు;
    స్థితి ఆధారం_Mgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      ఆపు;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, నిజం, అబద్ధం, బహుళకం) << సందేశం_3 << '\n';

  ఒకవేళ (పంక్తి_No == -1 || స్తంభం_No == -1)
    తిరిగి;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  ఒకవేళ (any_యొక్క(పంక్తి_Contents, isNonASCII)) {
    print_ఆధారం_పంక్తి(OS, పంక్తి_Contents);
    తిరిగి;
  }
  size_t అంకె_Columns = పంక్తి_Contents.కొలత_2();

  // Build the line with the caret and ranges.
  std::string Caret_పంక్తి(అంకె_Columns + 1, ' ');

  // Expand any ranges.
  కొరకు (స్థిరరాశి std::pair<గుర్తులేని, గుర్తులేని> &R : Ranges)
    std::fill(&Caret_పంక్తి[R.మొదటి_2],
              &Caret_పంక్తి[std::min((size_t)R.రెండవ_2, Caret_పంక్తి.కొలత_2())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_పంక్తి;
  build_Fix_It_పంక్తి(Caret_పంక్తి, Fix_It_Insertion_పంక్తి, FixIts,
                 ArrayRef(Loc.get_సూచిక() - స్తంభం_No, పంక్తి_Contents.కొలత_2()));

  // Finally, plop on the caret.
  ఒకవేళ (గుర్తులేని(స్తంభం_No) <= అంకె_Columns)
    Caret_పంక్తి[స్తంభం_No] = '^';
  లేకపోతే
    Caret_పంక్తి[అంకె_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_పంక్తి.erase(Caret_పంక్తి.కనుగొను_చివరి_not_యొక్క(' ') + 1);

  print_ఆధారం_పంక్తి(OS, పంక్తి_Contents);

  {
    Color_బహుళకం బహుళకం = ShowColors ? Color_బహుళకం::Auto : Color_బహుళకం::Disable;
    WithColor S(OS, raw_ostream::GREEN, నిజం, అబద్ధం, బహుళకం);

    // Print out the caret line, matching tabs in the source line.
    కొరకు (గుర్తులేని i = 0, e = Caret_పంక్తి.కొలత_2(), బహిర్గతం_నిలువు = 0; i != e; ++i) {
      ఒకవేళ (i >= పంక్తి_Contents.కొలత_2() || పంక్తి_Contents[i] != '\t') {
        S << Caret_పంక్తి[i];
        ++బహిర్గతం_నిలువు;
        కొనసాగు;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      చేయి {
        S << Caret_పంక్తి[i];
        ++బహిర్గతం_నిలువు;
      } అయితే ((బహిర్గతం_నిలువు % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  ఒకవేళ (Fix_It_Insertion_పంక్తి.ఖాళీలు())
    తిరిగి;

  కొరకు (size_t i = 0, e = Fix_It_Insertion_పంక్తి.కొలత_2(), బహిర్గతం_నిలువు = 0; i < e; ++i) {
    ఒకవేళ (i >= పంక్తి_Contents.కొలత_2() || పంక్తి_Contents[i] != '\t') {
      OS << Fix_It_Insertion_పంక్తి[i];
      ++బహిర్గతం_నిలువు;
      కొనసాగు;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    చేయి {
      OS << Fix_It_Insertion_పంక్తి[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      ఒకవేళ (Fix_It_Insertion_పంక్తి[i] != ' ')
        ++i;
      ++బహిర్గతం_నిలువు;
    } అయితే (((బహిర్గతం_నిలువు % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
