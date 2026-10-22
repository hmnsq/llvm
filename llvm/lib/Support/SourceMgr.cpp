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

#समावेश "llvm/Support/SourceMgr.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/Twine.h"
#समावेश "llvm/Support/ErrorOr.h"
#समावेश "llvm/Support/Locale.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/SMLoc.h"
#समावेश "llvm/Support/VirtualFileSystem.h"
#समावेश "llvm/Support/WithColor.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <algorithm>
#समावेश <cassert>
#समावेश <cstddef>
#समावेश <limits>
#समावेश <memory>
#समावेश <string>
#समावेश <utility>

using namespace llvm;

अटल स्थिर माप_प्रकार TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = शेष;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = शेष;
SourceMgr &SourceMgr::चालक=(SourceMgr &&) = शेष;
SourceMgr::~SourceMgr() = शेष;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() स्थिर {
  वापस FS;
}

व्योम SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  यह->FS = std::move(FS);
}

अचिन्हित SourceMgr::AddIncludeFile(स्थिर std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<Memory_बफर>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  अगर (!NewBufOrErr)
    वापस 0;

  वापस Add_New_Source_बफर(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<Memory_बफर>>
SourceMgr::OpenIncludeFile(स्थिर std::string &Filename,
                           std::string &IncludedFile,
                           bool RequiresNullTerminator) {
  स्वतः GetFile = [यह, RequiresNullTerminator](StringRef Path) {
    वापस FS ? FS->get_बफर_For_File(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : Memory_बफर::getFile(Path, /*IsText=*/false,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<Memory_बफर>> NewBufOrErr = GetFile(Filename);

  SmallString<64> बफर(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  क्रम (अचिन्हित i = 0, e = IncludeDirectories.आकार_2(); i != e && !NewBufOrErr;
       ++i) {
    बफर = IncludeDirectories[i];
    sys::path::append(बफर, Filename);
    NewBufOrErr = GetFile(बफर);
  }

  अगर (NewBufOrErr)
    IncludedFile = static_cast<std::string>(बफर);

  वापस NewBufOrErr;
}

अचिन्हित SourceMgr::Find_बफर_Containing_Loc(SMLoc Loc) स्थिर {
  क्रम (अचिन्हित i = 0, e = Buffers.आकार_2(); i != e; ++i)
    अगर (Loc.getPointer() >= Buffers[i].बफर->get_बफर_Start() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].बफर->get_बफर_End())
      वापस i + 1;
  वापस 0;
}

ढाँचा <typename T>
अटल std::vector<T> &GetOrCreateOffsetCache(व्योम *&OffsetCache,
                                              Memory_बफर *बफर) {
  अगर (OffsetCache)
    वापस *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  स्वतः *Offsets = नया std::vector<T>();
  माप_प्रकार Sz = बफर->get_बफर_आकार();
  निश्चित(Sz <= std::numeric_limits<T>::max());
  StringRef S = बफर->get_बफर();
  क्रम (माप_प्रकार N = 0; N < Sz; ++N) {
    अगर (S[N] == '\n')
      Offsets->धकेलो_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  वापस *Offsets;
}

ढाँचा <typename T>
std::pair<अचिन्हित, अचिन्हित>
SourceMgr::Src_बफर::get_पंक्ति_And_Column_Specialized(स्थिर अक्षर *Ptr) स्थिर {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, बफर.लो());

  स्थिर अक्षर *BufStart = बफर->get_बफर_Start();
  निश्चित(Ptr >= BufStart && Ptr <= बफर->get_बफर_End());
  सूचक_भेद_प्रकार PtrDiff = Ptr - BufStart;
  निश्चित(PtrDiff >= 0 &&
         static_cast<माप_प्रकार>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  स्वतः I = llvm::lower_bound(Offsets, PtrOffset);
  अचिन्हित पंक्ति_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T पंक्ति_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  अचिन्हित ColNo = PtrOffset - पंक्ति_Start_Offs + 1;
  वापस {पंक्ति_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<अचिन्हित, अचिन्हित>
SourceMgr::Src_बफर::get_पंक्ति_And_Column(स्थिर अक्षर *Ptr) स्थिर {
  माप_प्रकार Sz = बफर->get_बफर_आकार();
  अगर (Sz <= std::numeric_limits<uint8_t>::max())
    वापस get_पंक्ति_And_Column_Specialized<uint8_t>(Ptr);
  अगर (Sz <= std::numeric_limits<uint16_t>::max())
    वापस get_पंक्ति_And_Column_Specialized<uint16_t>(Ptr);
  अगर (Sz <= std::numeric_limits<uint32_t>::max())
    वापस get_पंक्ति_And_Column_Specialized<uint32_t>(Ptr);
  वापस get_पंक्ति_And_Column_Specialized<uint64_t>(Ptr);
}

ढाँचा <typename T>
स्थिर अक्षर *SourceMgr::Src_बफर::get_Pointer_For_पंक्ति_संख्या_Specialized(
    अचिन्हित पंक्ति_No) स्थिर {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, बफर.लो());

  // We start counting line and column numbers from 1.
  अगर (पंक्ति_No != 0)
    --पंक्ति_No;

  स्थिर अक्षर *BufStart = बफर->get_बफर_Start();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  अगर (पंक्ति_No == 0)
    वापस BufStart;
  अगर (पंक्ति_No > Offsets.आकार_2())
    वापस nullptr;
  वापस BufStart + Offsets[पंक्ति_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
स्थिर अक्षर *
SourceMgr::Src_बफर::get_Pointer_For_पंक्ति_संख्या(अचिन्हित पंक्ति_No) स्थिर {
  माप_प्रकार Sz = बफर->get_बफर_आकार();
  अगर (Sz <= std::numeric_limits<uint8_t>::max())
    वापस get_Pointer_For_पंक्ति_संख्या_Specialized<uint8_t>(पंक्ति_No);
  अन्यथा अगर (Sz <= std::numeric_limits<uint16_t>::max())
    वापस get_Pointer_For_पंक्ति_संख्या_Specialized<uint16_t>(पंक्ति_No);
  अन्यथा अगर (Sz <= std::numeric_limits<uint32_t>::max())
    वापस get_Pointer_For_पंक्ति_संख्या_Specialized<uint32_t>(पंक्ति_No);
  अन्यथा
    वापस get_Pointer_For_पंक्ति_संख्या_Specialized<uint64_t>(पंक्ति_No);
}

SourceMgr::Src_बफर::Src_बफर(SourceMgr::Src_बफर &&Other)
    : बफर(std::move(Other.बफर)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::Src_बफर::~Src_बफर() {
  अगर (OffsetCache) {
    माप_प्रकार Sz = बफर->get_बफर_आकार();
    अगर (Sz <= std::numeric_limits<uint8_t>::max())
      मिटाओ static_cast<std::vector<uint8_t> *>(OffsetCache);
    अन्यथा अगर (Sz <= std::numeric_limits<uint16_t>::max())
      मिटाओ static_cast<std::vector<uint16_t> *>(OffsetCache);
    अन्यथा अगर (Sz <= std::numeric_limits<uint32_t>::max())
      मिटाओ static_cast<std::vector<uint32_t> *>(OffsetCache);
    अन्यथा
      मिटाओ static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<अचिन्हित, अचिन्हित>
SourceMgr::get_पंक्ति_And_Column(SMLoc Loc, अचिन्हित बफर_ID) स्थिर {
  अगर (!बफर_ID)
    बफर_ID = Find_बफर_Containing_Loc(Loc);
  निश्चित(बफर_ID && "Invalid location!");

  स्वतः &SB = get_बफर_Info(बफर_ID);
  स्थिर अक्षर *Ptr = Loc.getPointer();

  वापस SB.get_पंक्ति_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    bool IncludePath) स्थिर {
  स्वतः बफर_ID = Find_बफर_Containing_Loc(Loc);
  निश्चित(बफर_ID && "Invalid location!");
  स्वतः FileSpec = get_बफर_Info(बफर_ID).बफर->get_बफर_Identifier();

  अगर (IncludePath) {
    वापस FileSpec.str() + ":" + std::to_string(Find_पंक्ति_संख्या(Loc, बफर_ID));
  } अन्यथा {
    स्वतः I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.आकार_2()) ? 0 : (I + 1);
    वापस FileSpec.substr(I).str() + ":" +
           std::to_string(Find_पंक्ति_संख्या(Loc, बफर_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_पंक्ति_And_Column(अचिन्हित बफर_ID, अचिन्हित पंक्ति_No,
                                         अचिन्हित ColNo) {
  स्वतः &SB = get_बफर_Info(बफर_ID);
  स्थिर अक्षर *Ptr = SB.get_Pointer_For_पंक्ति_संख्या(पंक्ति_No);
  अगर (!Ptr)
    वापस SMLoc();

  // We start counting line and column numbers from 1.
  अगर (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  अगर (ColNo) {
    // Make sure the location is within the current line.
    अगर (Ptr + ColNo > SB.बफर->get_बफर_End())
      वापस SMLoc();

    // Make sure there is no newline in the way.
    अगर (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      वापस SMLoc();

    Ptr += ColNo;
  }

  वापस SMLoc::getFromPointer(Ptr);
}

व्योम SourceMgr::Print_Include_ढेर(SMLoc IncludeLoc, raw_ostream &OS) स्थिर {
  अगर (IncludeLoc == SMLoc())
    वापस; // Top of stack.

  अचिन्हित CurBuf = Find_बफर_Containing_Loc(IncludeLoc);
  निश्चित(CurBuf && "Invalid or unspecified location!");

  Print_Include_ढेर(get_बफर_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_बफर_Info(CurBuf).बफर->get_बफर_Identifier()
     << ":" << Find_पंक्ति_संख्या(IncludeLoc, CurBuf) << ":\n";
}

व्योम SourceMgr::print_Include_ढेर_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) स्थिर {
  अगर (!Loc.isValid())
    वापस;
  अचिन्हित Diag_Cur_बफर = Find_बफर_Containing_Loc(Loc);
  अगर (Diag_Cur_बफर && Diag_Cur_बफर != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(Diag_Cur_बफर);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_ढेर(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   स्थिर Twine &संदेश_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) स्थिर {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<अचिन्हित, अचिन्हित>, 4> ColRanges;
  std::pair<अचिन्हित, अचिन्हित> पंक्ति_And_Col;
  StringRef बफर_ID = "<unknown>";
  StringRef पंक्ति_Str;

  अगर (Loc.isValid()) {
    अचिन्हित CurBuf = Find_बफर_Containing_Loc(Loc);
    निश्चित(CurBuf && "Invalid or unspecified location!");

    स्थिर Memory_बफर *CurMB = get_Memory_बफर(CurBuf);
    बफर_ID = CurMB->get_बफर_Identifier();

    // Scan backward to find the start of the line.
    स्थिर अक्षर *पंक्ति_Start = Loc.getPointer();
    स्थिर अक्षर *BufStart = CurMB->get_बफर_Start();
    जबतक (पंक्ति_Start != BufStart && पंक्ति_Start[-1] != '\n' &&
           पंक्ति_Start[-1] != '\r')
      --पंक्ति_Start;

    // Get the end of the line.
    स्थिर अक्षर *पंक्ति_End = Loc.getPointer();
    स्थिर अक्षर *BufEnd = CurMB->get_बफर_End();
    जबतक (पंक्ति_End != BufEnd && पंक्ति_End[0] != '\n' && पंक्ति_End[0] != '\r')
      ++पंक्ति_End;
    पंक्ति_Str = StringRef(पंक्ति_Start, पंक्ति_End - पंक्ति_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    क्रम (SMRange R : Ranges) {
      अगर (!R.isValid())
        जारी;

      // If the line doesn't contain any part of the range, then ignore it.
      अगर (R.Start.getPointer() > पंक्ति_End || R.End.getPointer() < पंक्ति_Start)
        जारी;

      // Ignore pieces of the range that go onto other lines.
      अगर (R.Start.getPointer() < पंक्ति_Start)
        R.Start = SMLoc::getFromPointer(पंक्ति_Start);
      अगर (R.End.getPointer() > पंक्ति_End)
        R.End = SMLoc::getFromPointer(पंक्ति_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.धकेलो_back(std::make_pair(R.Start.getPointer() - पंक्ति_Start,
                                         R.End.getPointer() - पंक्ति_Start));
    }

    पंक्ति_And_Col = get_पंक्ति_And_Column(Loc, CurBuf);
  }

  वापस SMDiagnostic(*यह, Loc, बफर_ID, पंक्ति_And_Col.first,
                      पंक्ति_And_Col.second - 1, Kind, संदेश_2.str(), पंक्ति_Str,
                      ColRanges, FixIts);
}

व्योम SourceMgr::PrintMessage(raw_ostream &OS, स्थिर SMDiagnostic &Diagnostic,
                             bool ShowColors) स्थिर {
  // Report the message with the diagnostic handler if present.
  अगर (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    वापस;
  }

  अगर (Diagnostic.getLoc().isValid()) {
    अचिन्हित CurBuf = Find_बफर_Containing_Loc(Diagnostic.getLoc());
    निश्चित(CurBuf && "Invalid or unspecified location!");
    Print_Include_ढेर(get_बफर_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

व्योम SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, स्थिर Twine &संदेश_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             bool ShowColors) स्थिर {
  PrintMessage(OS, GetMessage(Loc, Kind, संदेश_2, Ranges, FixIts), ShowColors);
}

व्योम SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             स्थिर Twine &संदेश_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, bool ShowColors) स्थिर {
  PrintMessage(errs(), Loc, Kind, संदेश_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, स्थिर Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  निश्चित(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(स्थिर SourceMgr &sm, SMLoc L, StringRef FN, पूर्णांक पंक्ति_2,
                           पूर्णांक Col, SourceMgr::DiagKind Kind, StringRef संदेश_2,
                           StringRef पंक्ति_Str,
                           ArrayRef<std::pair<अचिन्हित, अचिन्हित>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), पंक्ति_No(पंक्ति_2), ColumnNo(Col),
      Kind(Kind), Message(संदेश_2), पंक्ति_Contents(पंक्ति_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

अटल व्योम build_Fix_It_पंक्ति(std::string &Caret_पंक्ति, std::string &Fix_It_पंक्ति,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<अक्षर> Source_पंक्ति) {
  अगर (FixIts.empty())
    वापस;

  स्थिर अक्षर *पंक्ति_Start = Source_पंक्ति.begin();
  स्थिर अक्षर *पंक्ति_End = Source_पंक्ति.end();

  माप_प्रकार पूर्व_Hint_End_Col = 0;

  क्रम (स्थिर llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    अगर (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      जारी;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    अगर (R.Start.getPointer() > पंक्ति_End || R.End.getPointer() < पंक्ति_Start)
      जारी;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    अचिन्हित FirstCol;
    अगर (R.Start.getPointer() < पंक्ति_Start)
      FirstCol = 0;
    अन्यथा
      FirstCol = R.Start.getPointer() - पंक्ति_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    अचिन्हित HintCol = FirstCol;
    अगर (HintCol < पूर्व_Hint_End_Col)
      HintCol = पूर्व_Hint_End_Col + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    निश्चित((माप_प्रकार)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().आकार_2());

    // This relies on one byte per column in our fixit hints.
    अचिन्हित LastColumnModified = HintCol + Fixit.getText().आकार_2();
    अगर (LastColumnModified > Fix_It_पंक्ति.आकार_2())
      Fix_It_पंक्ति.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_पंक्ति.begin() + HintCol);

    पूर्व_Hint_End_Col = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    अचिन्हित LastCol;
    अगर (R.End.getPointer() >= पंक्ति_End)
      LastCol = पंक्ति_End - पंक्ति_Start;
    अन्यथा
      LastCol = R.End.getPointer() - पंक्ति_Start;

    std::fill(&Caret_पंक्ति[FirstCol], &Caret_पंक्ति[LastCol], '~');
  }
}

अटल व्योम print_Source_पंक्ति(raw_ostream &S, StringRef पंक्ति_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  क्रम (अचिन्हित i = 0, e = पंक्ति_Contents.आकार_2(), OutCol = 0; i != e; ++i) {
    माप_प्रकार अग्र_Tab = पंक्ति_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    अगर (अग्र_Tab == StringRef::npos) {
      S << पंक्ति_Contents.drop_front(i);
      अवरोध;
    }

    // Otherwise, print from i to NextTab.
    S << पंक्ति_Contents.slice(i, अग्र_Tab);
    OutCol += अग्र_Tab - i;
    i = अग्र_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    करो {
      S << ' ';
      ++OutCol;
    } जबतक ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

अटल bool isNonASCII(अक्षर c) { वापस c & 0x80; }

व्योम SMDiagnostic::print(स्थिर अक्षर *ProgName, raw_ostream &OS, bool ShowColors,
                         bool ShowKindLabel, bool ShowLocation) स्थिर {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, true, false, Mode);

    अगर (ProgName && ProgName[0])
      S << ProgName << ": ";

    अगर (ShowLocation && !Filename.empty()) {
      अगर (Filename == "-")
        S << "<stdin>";
      अन्यथा
        S << Filename;

      अगर (पंक्ति_No != -1) {
        S << ':' << पंक्ति_No;
        अगर (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  अगर (ShowKindLabel) {
    चयन (Kind) {
    हाल SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      अवरोध;
    हाल SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      अवरोध;
    हाल SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      अवरोध;
    हाल SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      अवरोध;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, true, false, Mode) << Message << '\n';

  अगर (पंक्ति_No == -1 || ColumnNo == -1)
    वापस;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  अगर (any_of(पंक्ति_Contents, isNonASCII)) {
    print_Source_पंक्ति(OS, पंक्ति_Contents);
    वापस;
  }
  माप_प्रकार अंक_Columns = पंक्ति_Contents.आकार_2();

  // Build the line with the caret and ranges.
  std::string Caret_पंक्ति(अंक_Columns + 1, ' ');

  // Expand any ranges.
  क्रम (स्थिर std::pair<अचिन्हित, अचिन्हित> &R : Ranges)
    std::fill(&Caret_पंक्ति[R.first],
              &Caret_पंक्ति[std::min((माप_प्रकार)R.second, Caret_पंक्ति.आकार_2())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_पंक्ति;
  build_Fix_It_पंक्ति(Caret_पंक्ति, Fix_It_Insertion_पंक्ति, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, पंक्ति_Contents.आकार_2()));

  // Finally, plop on the caret.
  अगर (अचिन्हित(ColumnNo) <= अंक_Columns)
    Caret_पंक्ति[ColumnNo] = '^';
  अन्यथा
    Caret_पंक्ति[अंक_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_पंक्ति.erase(Caret_पंक्ति.find_last_not_of(' ') + 1);

  print_Source_पंक्ति(OS, पंक्ति_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, true, false, Mode);

    // Print out the caret line, matching tabs in the source line.
    क्रम (अचिन्हित i = 0, e = Caret_पंक्ति.आकार_2(), OutCol = 0; i != e; ++i) {
      अगर (i >= पंक्ति_Contents.आकार_2() || पंक्ति_Contents[i] != '\t') {
        S << Caret_पंक्ति[i];
        ++OutCol;
        जारी;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      करो {
        S << Caret_पंक्ति[i];
        ++OutCol;
      } जबतक ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  अगर (Fix_It_Insertion_पंक्ति.empty())
    वापस;

  क्रम (माप_प्रकार i = 0, e = Fix_It_Insertion_पंक्ति.आकार_2(), OutCol = 0; i < e; ++i) {
    अगर (i >= पंक्ति_Contents.आकार_2() || पंक्ति_Contents[i] != '\t') {
      OS << Fix_It_Insertion_पंक्ति[i];
      ++OutCol;
      जारी;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    करो {
      OS << Fix_It_Insertion_पंक्ति[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      अगर (Fix_It_Insertion_पंक्ति[i] != ' ')
        ++i;
      ++OutCol;
    } जबतक (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
