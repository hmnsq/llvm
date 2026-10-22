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

#অন্তর্ভাব "llvm/Support/SourceMgr.h"
#অন্তর্ভাব "llvm/ADT/ArrayRef.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/Twine.h"
#অন্তর্ভাব "llvm/Support/ErrorOr.h"
#অন্তর্ভাব "llvm/Support/Locale.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/Path.h"
#অন্তর্ভাব "llvm/Support/SMLoc.h"
#অন্তর্ভাব "llvm/Support/VirtualFileSystem.h"
#অন্তর্ভাব "llvm/Support/WithColor.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <algorithm>
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstddef>
#অন্তর্ভাব <limits>
#অন্তর্ভাব <memory>
#অন্তর্ভাব <string>
#অন্তর্ভাব <utility>

using নামাকাশ llvm;

স্থির ধ্রুবক size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = পূর্বনির্ধারিত;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = পূর্বনির্ধারিত;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = পূর্বনির্ধারিত;
SourceMgr::~SourceMgr() = পূর্বনির্ধারিত;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ধ্রুবক {
  ফেরত FS;
}

শূন্য SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  এই->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(ধ্রুবক std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<Memory_বাফার>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  যদি (!NewBufOrErr)
    ফেরত 0;

  ফেরত Add_New_Source_বাফার(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<Memory_বাফার>>
SourceMgr::OpenIncludeFile(ধ্রুবক std::string &Filename,
                           std::string &IncludedFile,
                           যৌক্তিক RequiresNullTerminator) {
  auto GetFile = [এই, RequiresNullTerminator](StringRef Path) {
    ফেরত FS ? FS->get_বাফার_For_File(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : Memory_বাফার::getFile(Path, /*IsText=*/মিথ্যা,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<Memory_বাফার>> NewBufOrErr = GetFile(Filename);

  SmallString<64> বাফার(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  জন্য (unsigned i = 0, e = IncludeDirectories.আকার_3(); i != e && !NewBufOrErr;
       ++i) {
    বাফার = IncludeDirectories[i];
    sys::path::append(বাফার, Filename);
    NewBufOrErr = GetFile(বাফার);
  }

  যদি (NewBufOrErr)
    IncludedFile = static_cast<std::string>(বাফার);

  ফেরত NewBufOrErr;
}

unsigned SourceMgr::Find_বাফার_Containing_Loc(SMLoc Loc) ধ্রুবক {
  জন্য (unsigned i = 0, e = Buffers.আকার_3(); i != e; ++i)
    যদি (Loc.getPointer() >= Buffers[i].বাফার->get_বাফার_Start() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].বাফার->get_বাফার_End())
      ফেরত i + 1;
  ফেরত 0;
}

template <typename T>
স্থির std::vector<T> &GetOrCreateOffsetCache(শূন্য *&OffsetCache,
                                              Memory_বাফার *বাফার) {
  যদি (OffsetCache)
    ফেরত *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = নতুন std::vector<T>();
  size_t Sz = বাফার->get_বাফার_আকার();
  প্রতিজ্ঞা(Sz <= std::numeric_limits<T>::max());
  StringRef S = বাফার->get_বাফার();
  জন্য (size_t N = 0; N < Sz; ++N) {
    যদি (S[N] == '\n')
      Offsets->ঠেলো_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  ফেরত *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::Src_বাফার::get_সারি_And_Column_Specialized(ধ্রুবক অক্ষর *Ptr) ধ্রুবক {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, বাফার.get());

  ধ্রুবক অক্ষর *BufStart = বাফার->get_বাফার_Start();
  প্রতিজ্ঞা(Ptr >= BufStart && Ptr <= বাফার->get_বাফার_End());
  ptrdiff_t PtrDiff = Ptr - BufStart;
  প্রতিজ্ঞা(PtrDiff >= 0 &&
         static_cast<size_t>(PtrDiff) <= std::numeric_limits<T>::max());
  T PtrOffset = static_cast<T>(PtrDiff);

  // llvm::lower_bound gives the number of EOL before PtrOffset. Add 1 to get
  // the line number.
  auto I = llvm::lower_bound(Offsets, PtrOffset);
  unsigned সারি_No = I - Offsets.begin() + 1;

  // The column number is the distance from the previous EOL (or the start of
  // the buffer) to the pointer.
  T সারি_Start_Offs = (I == Offsets.begin()) ? 0 : (I[-1] + 1);
  unsigned ColNo = PtrOffset - সারি_Start_Offs + 1;
  ফেরত {সারি_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::Src_বাফার::get_সারি_And_Column(ধ্রুবক অক্ষর *Ptr) ধ্রুবক {
  size_t Sz = বাফার->get_বাফার_আকার();
  যদি (Sz <= std::numeric_limits<uint8_t>::max())
    ফেরত get_সারি_And_Column_Specialized<uint8_t>(Ptr);
  যদি (Sz <= std::numeric_limits<uint16_t>::max())
    ফেরত get_সারি_And_Column_Specialized<uint16_t>(Ptr);
  যদি (Sz <= std::numeric_limits<uint32_t>::max())
    ফেরত get_সারি_And_Column_Specialized<uint32_t>(Ptr);
  ফেরত get_সারি_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
ধ্রুবক অক্ষর *SourceMgr::Src_বাফার::get_Pointer_For_সারি_সংখ্যা_Specialized(
    unsigned সারি_No) ধ্রুবক {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, বাফার.get());

  // We start counting line and column numbers from 1.
  যদি (সারি_No != 0)
    --সারি_No;

  ধ্রুবক অক্ষর *BufStart = বাফার->get_বাফার_Start();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  যদি (সারি_No == 0)
    ফেরত BufStart;
  যদি (সারি_No > Offsets.আকার_3())
    ফেরত nullptr;
  ফেরত BufStart + Offsets[সারি_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ধ্রুবক অক্ষর *
SourceMgr::Src_বাফার::get_Pointer_For_সারি_সংখ্যা(unsigned সারি_No) ধ্রুবক {
  size_t Sz = বাফার->get_বাফার_আকার();
  যদি (Sz <= std::numeric_limits<uint8_t>::max())
    ফেরত get_Pointer_For_সারি_সংখ্যা_Specialized<uint8_t>(সারি_No);
  অন্যথা যদি (Sz <= std::numeric_limits<uint16_t>::max())
    ফেরত get_Pointer_For_সারি_সংখ্যা_Specialized<uint16_t>(সারি_No);
  অন্যথা যদি (Sz <= std::numeric_limits<uint32_t>::max())
    ফেরত get_Pointer_For_সারি_সংখ্যা_Specialized<uint32_t>(সারি_No);
  অন্যথা
    ফেরত get_Pointer_For_সারি_সংখ্যা_Specialized<uint64_t>(সারি_No);
}

SourceMgr::Src_বাফার::Src_বাফার(SourceMgr::Src_বাফার &&Other)
    : বাফার(std::move(Other.বাফার)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::Src_বাফার::~Src_বাফার() {
  যদি (OffsetCache) {
    size_t Sz = বাফার->get_বাফার_আকার();
    যদি (Sz <= std::numeric_limits<uint8_t>::max())
      delete static_cast<std::vector<uint8_t> *>(OffsetCache);
    অন্যথা যদি (Sz <= std::numeric_limits<uint16_t>::max())
      delete static_cast<std::vector<uint16_t> *>(OffsetCache);
    অন্যথা যদি (Sz <= std::numeric_limits<uint32_t>::max())
      delete static_cast<std::vector<uint32_t> *>(OffsetCache);
    অন্যথা
      delete static_cast<std::vector<uint64_t> *>(OffsetCache);
    OffsetCache = nullptr;
  }
}

std::pair<unsigned, unsigned>
SourceMgr::get_সারি_And_Column(SMLoc Loc, unsigned বাফার_ID) ধ্রুবক {
  যদি (!বাফার_ID)
    বাফার_ID = Find_বাফার_Containing_Loc(Loc);
  প্রতিজ্ঞা(বাফার_ID && "Invalid location!");

  auto &SB = get_বাফার_Info(বাফার_ID);
  ধ্রুবক অক্ষর *Ptr = Loc.getPointer();

  ফেরত SB.get_সারি_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    যৌক্তিক IncludePath) ধ্রুবক {
  auto বাফার_ID = Find_বাফার_Containing_Loc(Loc);
  প্রতিজ্ঞা(বাফার_ID && "Invalid location!");
  auto FileSpec = get_বাফার_Info(বাফার_ID).বাফার->get_বাফার_Identifier();

  যদি (IncludePath) {
    ফেরত FileSpec.str() + ":" + std::to_string(Find_সারি_সংখ্যা(Loc, বাফার_ID));
  } অন্যথা {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.আকার_3()) ? 0 : (I + 1);
    ফেরত FileSpec.substr(I).str() + ":" +
           std::to_string(Find_সারি_সংখ্যা(Loc, বাফার_ID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_সারি_And_Column(unsigned বাফার_ID, unsigned সারি_No,
                                         unsigned ColNo) {
  auto &SB = get_বাফার_Info(বাফার_ID);
  ধ্রুবক অক্ষর *Ptr = SB.get_Pointer_For_সারি_সংখ্যা(সারি_No);
  যদি (!Ptr)
    ফেরত SMLoc();

  // We start counting line and column numbers from 1.
  যদি (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  যদি (ColNo) {
    // Make sure the location is within the current line.
    যদি (Ptr + ColNo > SB.বাফার->get_বাফার_End())
      ফেরত SMLoc();

    // Make sure there is no newline in the way.
    যদি (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      ফেরত SMLoc();

    Ptr += ColNo;
  }

  ফেরত SMLoc::getFromPointer(Ptr);
}

শূন্য SourceMgr::Print_Include_স্তূপ(SMLoc IncludeLoc, raw_ostream &OS) ধ্রুবক {
  যদি (IncludeLoc == SMLoc())
    ফেরত; // Top of stack.

  unsigned CurBuf = Find_বাফার_Containing_Loc(IncludeLoc);
  প্রতিজ্ঞা(CurBuf && "Invalid or unspecified location!");

  Print_Include_স্তূপ(get_বাফার_Info(CurBuf).IncludeLoc, OS);

  OS << "Included from " << get_বাফার_Info(CurBuf).বাফার->get_বাফার_Identifier()
     << ":" << Find_সারি_সংখ্যা(IncludeLoc, CurBuf) << ":\n";
}

শূন্য SourceMgr::print_Include_স্তূপ_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ধ্রুবক {
  যদি (!Loc.isValid())
    ফেরত;
  unsigned Diag_Cur_বাফার = Find_বাফার_Containing_Loc(Loc);
  যদি (Diag_Cur_বাফার && Diag_Cur_বাফার != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(Diag_Cur_বাফার);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_স্তূপ(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ধ্রুবক Twine &বার্তা_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ধ্রুবক {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> সারি_And_Col;
  StringRef বাফার_ID = "<unknown>";
  StringRef সারি_Str;

  যদি (Loc.isValid()) {
    unsigned CurBuf = Find_বাফার_Containing_Loc(Loc);
    প্রতিজ্ঞা(CurBuf && "Invalid or unspecified location!");

    ধ্রুবক Memory_বাফার *CurMB = get_Memory_বাফার(CurBuf);
    বাফার_ID = CurMB->get_বাফার_Identifier();

    // Scan backward to find the start of the line.
    ধ্রুবক অক্ষর *সারি_Start = Loc.getPointer();
    ধ্রুবক অক্ষর *BufStart = CurMB->get_বাফার_Start();
    যখন (সারি_Start != BufStart && সারি_Start[-1] != '\n' &&
           সারি_Start[-1] != '\r')
      --সারি_Start;

    // Get the end of the line.
    ধ্রুবক অক্ষর *সারি_End = Loc.getPointer();
    ধ্রুবক অক্ষর *BufEnd = CurMB->get_বাফার_End();
    যখন (সারি_End != BufEnd && সারি_End[0] != '\n' && সারি_End[0] != '\r')
      ++সারি_End;
    সারি_Str = StringRef(সারি_Start, সারি_End - সারি_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    জন্য (SMRange R : Ranges) {
      যদি (!R.isValid())
        চালু;

      // If the line doesn't contain any part of the range, then ignore it.
      যদি (R.Start.getPointer() > সারি_End || R.End.getPointer() < সারি_Start)
        চালু;

      // Ignore pieces of the range that go onto other lines.
      যদি (R.Start.getPointer() < সারি_Start)
        R.Start = SMLoc::getFromPointer(সারি_Start);
      যদি (R.End.getPointer() > সারি_End)
        R.End = SMLoc::getFromPointer(সারি_End);

      // Translate from SMLoc ranges to column ranges.
      // FIXME: Handle multibyte characters.
      ColRanges.ঠেলো_back(std::make_pair(R.Start.getPointer() - সারি_Start,
                                         R.End.getPointer() - সারি_Start));
    }

    সারি_And_Col = get_সারি_And_Column(Loc, CurBuf);
  }

  ফেরত SMDiagnostic(*এই, Loc, বাফার_ID, সারি_And_Col.first,
                      সারি_And_Col.second - 1, Kind, বার্তা_2.str(), সারি_Str,
                      ColRanges, FixIts);
}

শূন্য SourceMgr::PrintMessage(raw_ostream &OS, ধ্রুবক SMDiagnostic &Diagnostic,
                             যৌক্তিক ShowColors) ধ্রুবক {
  // Report the message with the diagnostic handler if present.
  যদি (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    ফেরত;
  }

  যদি (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = Find_বাফার_Containing_Loc(Diagnostic.getLoc());
    প্রতিজ্ঞা(CurBuf && "Invalid or unspecified location!");
    Print_Include_স্তূপ(get_বাফার_Info(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

শূন্য SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ধ্রুবক Twine &বার্তা_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             যৌক্তিক ShowColors) ধ্রুবক {
  PrintMessage(OS, GetMessage(Loc, Kind, বার্তা_2, Ranges, FixIts), ShowColors);
}

শূন্য SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ধ্রুবক Twine &বার্তা_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, যৌক্তিক ShowColors) ধ্রুবক {
  PrintMessage(errs(), Loc, Kind, বার্তা_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ধ্রুবক Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  প্রতিজ্ঞা(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ধ্রুবক SourceMgr &sm, SMLoc L, StringRef FN, পূর্ণসংখ্যা সারি,
                           পূর্ণসংখ্যা Col, SourceMgr::DiagKind Kind, StringRef বার্তা_2,
                           StringRef সারি_Str,
                           ArrayRef<std::pair<unsigned, unsigned>> Ranges,
                           ArrayRef<SMFixIt> Hints)
    : SM(&sm), Loc(L), Filename(std::string(FN)), সারি_No(সারি), ColumnNo(Col),
      Kind(Kind), Message(বার্তা_2), সারি_Contents(সারি_Str), Ranges(Ranges.vec()),
      FixIts(Hints) {
  llvm::sort(FixIts);
}

স্থির শূন্য build_Fix_It_সারি(std::string &Caret_সারি, std::string &Fix_It_সারি,
                           ArrayRef<SMFixIt> FixIts,
                           ArrayRef<অক্ষর> Source_সারি) {
  যদি (FixIts.empty())
    ফেরত;

  ধ্রুবক অক্ষর *সারি_Start = Source_সারি.begin();
  ধ্রুবক অক্ষর *সারি_End = Source_সারি.end();

  size_t পূর্ব_Hint_End_Col = 0;

  জন্য (ধ্রুবক llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    যদি (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      চালু;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    যদি (R.Start.getPointer() > সারি_End || R.End.getPointer() < সারি_Start)
      চালু;

    // Translate from SMLoc to column.
    // Ignore pieces of the range that go onto other lines.
    // FIXME: Handle multibyte characters in the source line.
    unsigned FirstCol;
    যদি (R.Start.getPointer() < সারি_Start)
      FirstCol = 0;
    অন্যথা
      FirstCol = R.Start.getPointer() - সারি_Start;

    // If we inserted a long previous hint, push this one forwards, and add
    // an extra space to show that this is not part of the previous
    // completion. This is sort of the best we can do when two hints appear
    // to overlap.
    //
    // Note that if this hint is located immediately after the previous
    // hint, no space will be added, since the location is more important.
    unsigned HintCol = FirstCol;
    যদি (HintCol < পূর্ব_Hint_End_Col)
      HintCol = পূর্ব_Hint_End_Col + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    প্রতিজ্ঞা((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().আকার_3());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().আকার_3();
    যদি (LastColumnModified > Fix_It_সারি.আকার_3())
      Fix_It_সারি.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_সারি.begin() + HintCol);

    পূর্ব_Hint_End_Col = LastColumnModified;

    // For replacements, mark the removal range with '~'.
    // FIXME: Handle multibyte characters in the source line.
    unsigned LastCol;
    যদি (R.End.getPointer() >= সারি_End)
      LastCol = সারি_End - সারি_Start;
    অন্যথা
      LastCol = R.End.getPointer() - সারি_Start;

    std::fill(&Caret_সারি[FirstCol], &Caret_সারি[LastCol], '~');
  }
}

স্থির শূন্য print_Source_সারি(raw_ostream &S, StringRef সারি_Contents) {
  // Print out the source line one character at a time, so we can expand tabs.
  জন্য (unsigned i = 0, e = সারি_Contents.আকার_3(), OutCol = 0; i != e; ++i) {
    size_t পরবর্তী_Tab = সারি_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    যদি (পরবর্তী_Tab == StringRef::npos) {
      S << সারি_Contents.drop_front(i);
      বিরতি;
    }

    // Otherwise, print from i to NextTab.
    S << সারি_Contents.slice(i, পরবর্তী_Tab);
    OutCol += পরবর্তী_Tab - i;
    i = পরবর্তী_Tab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    করো {
      S << ' ';
      ++OutCol;
    } যখন ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

স্থির যৌক্তিক isNonASCII(অক্ষর c) { ফেরত c & 0x80; }

শূন্য SMDiagnostic::print(ধ্রুবক অক্ষর *ProgName, raw_ostream &OS, যৌক্তিক ShowColors,
                         যৌক্তিক ShowKindLabel, যৌক্তিক ShowLocation) ধ্রুবক {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, সত্য, মিথ্যা, Mode);

    যদি (ProgName && ProgName[0])
      S << ProgName << ": ";

    যদি (ShowLocation && !Filename.empty()) {
      যদি (Filename == "-")
        S << "<stdin>";
      অন্যথা
        S << Filename;

      যদি (সারি_No != -1) {
        S << ':' << সারি_No;
        যদি (ColumnNo != -1)
          S << ':' << (ColumnNo + 1);
      }
      S << ": ";
    }
  }

  যদি (ShowKindLabel) {
    নির্বাচন (Kind) {
    ক্ষেত্র SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      বিরতি;
    ক্ষেত্র SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      বিরতি;
    ক্ষেত্র SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      বিরতি;
    ক্ষেত্র SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      বিরতি;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, সত্য, মিথ্যা, Mode) << Message << '\n';

  যদি (সারি_No == -1 || ColumnNo == -1)
    ফেরত;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  যদি (any_of(সারি_Contents, isNonASCII)) {
    print_Source_সারি(OS, সারি_Contents);
    ফেরত;
  }
  size_t অঙ্ক_Columns = সারি_Contents.আকার_3();

  // Build the line with the caret and ranges.
  std::string Caret_সারি(অঙ্ক_Columns + 1, ' ');

  // Expand any ranges.
  জন্য (ধ্রুবক std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_সারি[R.first],
              &Caret_সারি[std::min((size_t)R.second, Caret_সারি.আকার_3())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_সারি;
  build_Fix_It_সারি(Caret_সারি, Fix_It_Insertion_সারি, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, সারি_Contents.আকার_3()));

  // Finally, plop on the caret.
  যদি (unsigned(ColumnNo) <= অঙ্ক_Columns)
    Caret_সারি[ColumnNo] = '^';
  অন্যথা
    Caret_সারি[অঙ্ক_Columns] = '^';

  // ... and remove trailing whitespace so the output doesn't wrap for it.  We
  // know that the line isn't completely empty because it has the caret in it at
  // least.
  Caret_সারি.erase(Caret_সারি.find_last_not_of(' ') + 1);

  print_Source_সারি(OS, সারি_Contents);

  {
    ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;
    WithColor S(OS, raw_ostream::GREEN, সত্য, মিথ্যা, Mode);

    // Print out the caret line, matching tabs in the source line.
    জন্য (unsigned i = 0, e = Caret_সারি.আকার_3(), OutCol = 0; i != e; ++i) {
      যদি (i >= সারি_Contents.আকার_3() || সারি_Contents[i] != '\t') {
        S << Caret_সারি[i];
        ++OutCol;
        চালু;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      করো {
        S << Caret_সারি[i];
        ++OutCol;
      } যখন ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  যদি (Fix_It_Insertion_সারি.empty())
    ফেরত;

  জন্য (size_t i = 0, e = Fix_It_Insertion_সারি.আকার_3(), OutCol = 0; i < e; ++i) {
    যদি (i >= সারি_Contents.আকার_3() || সারি_Contents[i] != '\t') {
      OS << Fix_It_Insertion_সারি[i];
      ++OutCol;
      চালু;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    করো {
      OS << Fix_It_Insertion_সারি[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      যদি (Fix_It_Insertion_সারি[i] != ' ')
        ++i;
      ++OutCol;
    } যখন (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
