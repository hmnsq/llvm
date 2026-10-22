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

স্থির ধ্রুব size_t TabStop = 8;

// Out of line to avoid needing definition of vfs::FileSystem in header.
SourceMgr::SourceMgr() = স্বভাব;
SourceMgr::SourceMgr(IntrusiveRefCntPtr<vfs::FileSystem> FS)
    : FS(std::move(FS)) {}
SourceMgr::SourceMgr(SourceMgr &&) = স্বভাব;
SourceMgr &SourceMgr::operator=(SourceMgr &&) = স্বভাব;
SourceMgr::~SourceMgr() = স্বভাব;

IntrusiveRefCntPtr<vfs::FileSystem> SourceMgr::getVirtualFileSystem() ধ্রুব {
  প্রত্যাগম FS;
}

শূন্য SourceMgr::setVirtualFileSystem(IntrusiveRefCntPtr<vfs::FileSystem> FS) {
  এতত্->FS = std::move(FS);
}

unsigned SourceMgr::AddIncludeFile(ধ্রুব std::string &Filename,
                                   SMLoc IncludeLoc,
                                   std::string &IncludedFile) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr =
      OpenIncludeFile(Filename, IncludedFile);
  যদি (!NewBufOrErr)
    প্রত্যাগম 0;

  প্রত্যাগম AddNewSourceBuffer(std::move(*NewBufOrErr), IncludeLoc);
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
SourceMgr::OpenIncludeFile(ধ্রুব std::string &Filename,
                           std::string &IncludedFile,
                           তার্কিক RequiresNullTerminator) {
  auto GetFile = [এতত্, RequiresNullTerminator](StringRef Path) {
    প্রত্যাগম FS ? FS->getBufferForFile(Path, /*FileSize=*/-1,
                                     RequiresNullTerminator)
              : MemoryBuffer::getFile(Path, /*IsText=*/অসত্য,
                                      RequiresNullTerminator);
  };

  ErrorOr<std::unique_ptr<MemoryBuffer>> NewBufOrErr = GetFile(Filename);

  SmallString<64> Buffer(Filename);
  // If the file didn't exist directly, see if it's in an include path.
  প্রতি (unsigned i = 0, e = IncludeDirectories.size(); i != e && !NewBufOrErr;
       ++i) {
    Buffer = IncludeDirectories[i];
    sys::path::append(Buffer, Filename);
    NewBufOrErr = GetFile(Buffer);
  }

  যদি (NewBufOrErr)
    IncludedFile = static_cast<std::string>(Buffer);

  প্রত্যাগম NewBufOrErr;
}

unsigned SourceMgr::FindBufferContainingLoc(SMLoc Loc) ধ্রুব {
  প্রতি (unsigned i = 0, e = Buffers.size(); i != e; ++i)
    যদি (Loc.getPointer() >= Buffers[i].Buffer->getBufferStart() &&
        // Use <= here so that a pointer to the null at the end of the buffer
        // is included as part of the buffer.
        Loc.getPointer() <= Buffers[i].Buffer->getBufferEnd())
      প্রত্যাগম i + 1;
  প্রত্যাগম 0;
}

template <typename T>
স্থির std::vector<T> &GetOrCreateOffsetCache(শূন্য *&OffsetCache,
                                              MemoryBuffer *Buffer) {
  যদি (OffsetCache)
    প্রত্যাগম *static_cast<std::vector<T> *>(OffsetCache);

  // Lazily fill in the offset cache.
  auto *Offsets = নূতন std::vector<T>();
  size_t Sz = Buffer->getBufferSize();
  প্রতিজ্ঞা(Sz <= std::numeric_limits<T>::max());
  StringRef S = Buffer->getBuffer();
  প্রতি (size_t N = 0; N < Sz; ++N) {
    যদি (S[N] == '\n')
      Offsets->ঠেলো_back(static_cast<T>(N));
  }

  OffsetCache = Offsets;
  প্রত্যাগম *Offsets;
}

template <typename T>
std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_সারি_And_Column_Specialized(ধ্রুব বর্ণ *Ptr) ধ্রুব {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  ধ্রুব বর্ণ *BufStart = Buffer->getBufferStart();
  প্রতিজ্ঞা(Ptr >= BufStart && Ptr <= Buffer->getBufferEnd());
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
  প্রত্যাগম {সারি_No, ColNo};
}

/// Look up a given \p Ptr in the buffer, determining which line and column
/// it came from.
LLVM_ABI std::pair<unsigned, unsigned>
SourceMgr::SrcBuffer::get_সারি_And_Column(ধ্রুব বর্ণ *Ptr) ধ্রুব {
  size_t Sz = Buffer->getBufferSize();
  যদি (Sz <= std::numeric_limits<uint8_t>::max())
    প্রত্যাগম get_সারি_And_Column_Specialized<uint8_t>(Ptr);
  যদি (Sz <= std::numeric_limits<uint16_t>::max())
    প্রত্যাগম get_সারি_And_Column_Specialized<uint16_t>(Ptr);
  যদি (Sz <= std::numeric_limits<uint32_t>::max())
    প্রত্যাগম get_সারি_And_Column_Specialized<uint32_t>(Ptr);
  প্রত্যাগম get_সারি_And_Column_Specialized<uint64_t>(Ptr);
}

template <typename T>
ধ্রুব বর্ণ *SourceMgr::SrcBuffer::get_Pointer_For_সারি_সংখ্যা_Specialized(
    unsigned সারি_No) ধ্রুব {
  std::vector<T> &Offsets =
      GetOrCreateOffsetCache<T>(OffsetCache, Buffer.get());

  // We start counting line and column numbers from 1.
  যদি (সারি_No != 0)
    --সারি_No;

  ধ্রুব বর্ণ *BufStart = Buffer->getBufferStart();

  // The offset cache contains the location of the \n for the specified line,
  // we want the start of the line.  As such, we look for the previous entry.
  যদি (সারি_No == 0)
    প্রত্যাগম BufStart;
  যদি (সারি_No > Offsets.size())
    প্রত্যাগম nullptr;
  প্রত্যাগম BufStart + Offsets[সারি_No - 1] + 1;
}

/// Return a pointer to the first character of the specified line number or
/// null if the line number is invalid.
ধ্রুব বর্ণ *
SourceMgr::SrcBuffer::get_Pointer_For_সারি_সংখ্যা(unsigned সারি_No) ধ্রুব {
  size_t Sz = Buffer->getBufferSize();
  যদি (Sz <= std::numeric_limits<uint8_t>::max())
    প্রত্যাগম get_Pointer_For_সারি_সংখ্যা_Specialized<uint8_t>(সারি_No);
  অন্যথা যদি (Sz <= std::numeric_limits<uint16_t>::max())
    প্রত্যাগম get_Pointer_For_সারি_সংখ্যা_Specialized<uint16_t>(সারি_No);
  অন্যথা যদি (Sz <= std::numeric_limits<uint32_t>::max())
    প্রত্যাগম get_Pointer_For_সারি_সংখ্যা_Specialized<uint32_t>(সারি_No);
  অন্যথা
    প্রত্যাগম get_Pointer_For_সারি_সংখ্যা_Specialized<uint64_t>(সারি_No);
}

SourceMgr::SrcBuffer::SrcBuffer(SourceMgr::SrcBuffer &&Other)
    : Buffer(std::move(Other.Buffer)), OffsetCache(Other.OffsetCache),
      IncludeLoc(Other.IncludeLoc) {
  Other.OffsetCache = nullptr;
}

SourceMgr::SrcBuffer::~SrcBuffer() {
  যদি (OffsetCache) {
    size_t Sz = Buffer->getBufferSize();
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
SourceMgr::get_সারি_And_Column(SMLoc Loc, unsigned BufferID) ধ্রুব {
  যদি (!BufferID)
    BufferID = FindBufferContainingLoc(Loc);
  প্রতিজ্ঞা(BufferID && "Invalid location!");

  auto &SB = getBufferInfo(BufferID);
  ধ্রুব বর্ণ *Ptr = Loc.getPointer();

  প্রত্যাগম SB.get_সারি_And_Column(Ptr);
}

// FIXME: Note that the formatting of source locations is spread between
// multiple functions, some in SourceMgr and some in SMDiagnostic. A better
// solution would be a general-purpose source location formatter
// in one of those two classes, or possibly in SMLoc.

/// Get a string with the source location formatted in the standard
/// style, but without the line offset. If \p IncludePath is true, the path
/// is included. If false, only the file name and extension are included.
std::string SourceMgr::getFormattedLocationNoOffset(SMLoc Loc,
                                                    তার্কিক IncludePath) ধ্রুব {
  auto BufferID = FindBufferContainingLoc(Loc);
  প্রতিজ্ঞা(BufferID && "Invalid location!");
  auto FileSpec = getBufferInfo(BufferID).Buffer->getBufferIdentifier();

  যদি (IncludePath) {
    প্রত্যাগম FileSpec.str() + ":" + std::to_string(Find_সারি_সংখ্যা(Loc, BufferID));
  } অন্যথা {
    auto I = FileSpec.find_last_of("/\\");
    I = (I == FileSpec.size()) ? 0 : (I + 1);
    প্রত্যাগম FileSpec.substr(I).str() + ":" +
           std::to_string(Find_সারি_সংখ্যা(Loc, BufferID));
  }
}

/// Given a line and column number in a mapped buffer, turn it into an SMLoc.
/// This will return a null SMLoc if the line/column location is invalid.
SMLoc SourceMgr::Find_Loc_For_সারি_And_Column(unsigned BufferID, unsigned সারি_No,
                                         unsigned ColNo) {
  auto &SB = getBufferInfo(BufferID);
  ধ্রুব বর্ণ *Ptr = SB.get_Pointer_For_সারি_সংখ্যা(সারি_No);
  যদি (!Ptr)
    প্রত্যাগম SMLoc();

  // We start counting line and column numbers from 1.
  যদি (ColNo != 0)
    --ColNo;

  // If we have a column number, validate it.
  যদি (ColNo) {
    // Make sure the location is within the current line.
    যদি (Ptr + ColNo > SB.Buffer->getBufferEnd())
      প্রত্যাগম SMLoc();

    // Make sure there is no newline in the way.
    যদি (StringRef(Ptr, ColNo).find_first_of("\n\r") != StringRef::npos)
      প্রত্যাগম SMLoc();

    Ptr += ColNo;
  }

  প্রত্যাগম SMLoc::getFromPointer(Ptr);
}

শূন্য SourceMgr::Print_Include_স্তূপ(SMLoc IncludeLoc, raw_ostream &OS) ধ্রুব {
  যদি (IncludeLoc == SMLoc())
    প্রত্যাগম; // Top of stack.

  unsigned CurBuf = FindBufferContainingLoc(IncludeLoc);
  প্রতিজ্ঞা(CurBuf && "Invalid or unspecified location!");

  Print_Include_স্তূপ(getBufferInfo(CurBuf).IncludeLoc, OS);

  OS << "Included from " << getBufferInfo(CurBuf).Buffer->getBufferIdentifier()
     << ":" << Find_সারি_সংখ্যা(IncludeLoc, CurBuf) << ":\n";
}

শূন্য SourceMgr::print_Include_স্তূপ_For_Diagnostic(SMLoc Loc,
                                               raw_ostream &OS) ধ্রুব {
  যদি (!Loc.isValid())
    প্রত্যাগম;
  unsigned DiagCurBuffer = FindBufferContainingLoc(Loc);
  যদি (DiagCurBuffer && DiagCurBuffer != getMainFileID()) {
    SMLoc ParentIncludeLoc = getParentIncludeLoc(DiagCurBuffer);
    // Ignore macro instantiation buffers to avoid redundant include stacks.
    Print_Include_স্তূপ(ParentIncludeLoc, OS);
  }
}

SMDiagnostic SourceMgr::GetMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                   ধ্রুব Twine &বার্তা_2, ArrayRef<SMRange> Ranges,
                                   ArrayRef<SMFixIt> FixIts) ধ্রুব {
  // First thing to do: find the current buffer containing the specified
  // location to pull out the source line.
  SmallVector<std::pair<unsigned, unsigned>, 4> ColRanges;
  std::pair<unsigned, unsigned> সারি_And_Col;
  StringRef BufferID = "<unknown>";
  StringRef সারি_Str;

  যদি (Loc.isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Loc);
    প্রতিজ্ঞা(CurBuf && "Invalid or unspecified location!");

    ধ্রুব MemoryBuffer *CurMB = getMemoryBuffer(CurBuf);
    BufferID = CurMB->getBufferIdentifier();

    // Scan backward to find the start of the line.
    ধ্রুব বর্ণ *সারি_Start = Loc.getPointer();
    ধ্রুব বর্ণ *BufStart = CurMB->getBufferStart();
    যাবত্ (সারি_Start != BufStart && সারি_Start[-1] != '\n' &&
           সারি_Start[-1] != '\r')
      --সারি_Start;

    // Get the end of the line.
    ধ্রুব বর্ণ *সারি_End = Loc.getPointer();
    ধ্রুব বর্ণ *BufEnd = CurMB->getBufferEnd();
    যাবত্ (সারি_End != BufEnd && সারি_End[0] != '\n' && সারি_End[0] != '\r')
      ++সারি_End;
    সারি_Str = StringRef(সারি_Start, সারি_End - সারি_Start);

    // Convert any ranges to column ranges that only intersect the line of the
    // location.
    প্রতি (SMRange R : Ranges) {
      যদি (!R.isValid())
        অনুবর্ত;

      // If the line doesn't contain any part of the range, then ignore it.
      যদি (R.Start.getPointer() > সারি_End || R.End.getPointer() < সারি_Start)
        অনুবর্ত;

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

  প্রত্যাগম SMDiagnostic(*এতত্, Loc, BufferID, সারি_And_Col.first,
                      সারি_And_Col.second - 1, Kind, বার্তা_2.str(), সারি_Str,
                      ColRanges, FixIts);
}

শূন্য SourceMgr::PrintMessage(raw_ostream &OS, ধ্রুব SMDiagnostic &Diagnostic,
                             তার্কিক ShowColors) ধ্রুব {
  // Report the message with the diagnostic handler if present.
  যদি (DiagHandler) {
    DiagHandler(Diagnostic, DiagContext);
    প্রত্যাগম;
  }

  যদি (Diagnostic.getLoc().isValid()) {
    unsigned CurBuf = FindBufferContainingLoc(Diagnostic.getLoc());
    প্রতিজ্ঞা(CurBuf && "Invalid or unspecified location!");
    Print_Include_স্তূপ(getBufferInfo(CurBuf).IncludeLoc, OS);
  }

  Diagnostic.print(nullptr, OS, ShowColors);
}

শূন্য SourceMgr::PrintMessage(raw_ostream &OS, SMLoc Loc,
                             SourceMgr::DiagKind Kind, ধ্রুব Twine &বার্তা_2,
                             ArrayRef<SMRange> Ranges, ArrayRef<SMFixIt> FixIts,
                             তার্কিক ShowColors) ধ্রুব {
  PrintMessage(OS, GetMessage(Loc, Kind, বার্তা_2, Ranges, FixIts), ShowColors);
}

শূন্য SourceMgr::PrintMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                             ধ্রুব Twine &বার্তা_2, ArrayRef<SMRange> Ranges,
                             ArrayRef<SMFixIt> FixIts, তার্কিক ShowColors) ধ্রুব {
  PrintMessage(errs(), Loc, Kind, বার্তা_2, Ranges, FixIts, ShowColors);
}

//===----------------------------------------------------------------------===//
// SMFixIt Implementation
//===----------------------------------------------------------------------===//

SMFixIt::SMFixIt(SMRange R, ধ্রুব Twine &Replacement)
    : Range(R), Text(Replacement.str()) {
  প্রতিজ্ঞা(R.isValid());
}

//===----------------------------------------------------------------------===//
// SMDiagnostic Implementation
//===----------------------------------------------------------------------===//

SMDiagnostic::SMDiagnostic(ধ্রুব SourceMgr &sm, SMLoc L, StringRef FN, পূর্ণাঙ্ক সারি,
                           পূর্ণাঙ্ক Col, SourceMgr::DiagKind Kind, StringRef বার্তা_2,
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
                           ArrayRef<বর্ণ> Source_সারি) {
  যদি (FixIts.empty())
    প্রত্যাগম;

  ধ্রুব বর্ণ *সারি_Start = Source_সারি.begin();
  ধ্রুব বর্ণ *সারি_End = Source_সারি.end();

  size_t PrevHintEndCol = 0;

  প্রতি (ধ্রুব llvm::SMFixIt &Fixit : FixIts) {
    // If the fixit contains a newline or tab, ignore it.
    যদি (Fixit.getText().find_first_of("\n\r\t") != StringRef::npos)
      অনুবর্ত;

    SMRange R = Fixit.getRange();

    // If the line doesn't contain any part of the range, then ignore it.
    যদি (R.Start.getPointer() > সারি_End || R.End.getPointer() < সারি_Start)
      অনুবর্ত;

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
    যদি (HintCol < PrevHintEndCol)
      HintCol = PrevHintEndCol + 1;

    // FIXME: This assertion is intended to catch unintended use of multibyte
    // characters in fixits. If we decide to do this, we'll have to track
    // separate byte widths for the source and fixit lines.
    প্রতিজ্ঞা((size_t)sys::locale::columnWidth(Fixit.getText()) ==
           Fixit.getText().size());

    // This relies on one byte per column in our fixit hints.
    unsigned LastColumnModified = HintCol + Fixit.getText().size();
    যদি (LastColumnModified > Fix_It_সারি.size())
      Fix_It_সারি.resize(LastColumnModified, ' ');

    llvm::copy(Fixit.getText(), Fix_It_সারি.begin() + HintCol);

    PrevHintEndCol = LastColumnModified;

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
  প্রতি (unsigned i = 0, e = সারি_Contents.size(), OutCol = 0; i != e; ++i) {
    size_t NextTab = সারি_Contents.find('\t', i);
    // If there were no tabs left, print the rest, we are done.
    যদি (NextTab == StringRef::npos) {
      S << সারি_Contents.drop_front(i);
      ভঙ্গ;
    }

    // Otherwise, print from i to NextTab.
    S << সারি_Contents.slice(i, NextTab);
    OutCol += NextTab - i;
    i = NextTab;

    // If we have a tab, emit at least one space, then round up to 8 columns.
    কুরু {
      S << ' ';
      ++OutCol;
    } যাবত্ ((OutCol % TabStop) != 0);
  }
  S << '\n';
}

স্থির তার্কিক isNonASCII(বর্ণ c) { প্রত্যাগম c & 0x80; }

শূন্য SMDiagnostic::print(ধ্রুব বর্ণ *ProgName, raw_ostream &OS, তার্কিক ShowColors,
                         তার্কিক ShowKindLabel, তার্কিক ShowLocation) ধ্রুব {
  ColorMode Mode = ShowColors ? ColorMode::Auto : ColorMode::Disable;

  {
    WithColor S(OS, raw_ostream::SAVEDCOLOR, সত্য, অসত্য, Mode);

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
    বিকল্প (Kind) {
    পক্ষ SourceMgr::DK_Error:
      WithColor::error(OS, "", !ShowColors);
      ভঙ্গ;
    পক্ষ SourceMgr::DK_Warning:
      WithColor::warning(OS, "", !ShowColors);
      ভঙ্গ;
    পক্ষ SourceMgr::DK_Note:
      WithColor::note(OS, "", !ShowColors);
      ভঙ্গ;
    পক্ষ SourceMgr::DK_Remark:
      WithColor::remark(OS, "", !ShowColors);
      ভঙ্গ;
    }
  }

  WithColor(OS, raw_ostream::SAVEDCOLOR, সত্য, অসত্য, Mode) << Message << '\n';

  যদি (সারি_No == -1 || ColumnNo == -1)
    প্রত্যাগম;

  // FIXME: If there are multibyte or multi-column characters in the source, all
  // our ranges will be wrong. To do this properly, we'll need a byte-to-column
  // map like Clang's TextDiagnostic. For now, we'll just handle tabs by
  // expanding them later, and bail out rather than show incorrect ranges and
  // misaligned fixits for any other odd characters.
  যদি (any_of(সারি_Contents, isNonASCII)) {
    print_Source_সারি(OS, সারি_Contents);
    প্রত্যাগম;
  }
  size_t অঙ্ক_Columns = সারি_Contents.size();

  // Build the line with the caret and ranges.
  std::string Caret_সারি(অঙ্ক_Columns + 1, ' ');

  // Expand any ranges.
  প্রতি (ধ্রুব std::pair<unsigned, unsigned> &R : Ranges)
    std::fill(&Caret_সারি[R.first],
              &Caret_সারি[std::min((size_t)R.second, Caret_সারি.size())], '~');

  // Add any fix-its.
  // FIXME: Find the beginning of the line properly for multibyte characters.
  std::string Fix_It_Insertion_সারি;
  build_Fix_It_সারি(Caret_সারি, Fix_It_Insertion_সারি, FixIts,
                 ArrayRef(Loc.getPointer() - ColumnNo, সারি_Contents.size()));

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
    WithColor S(OS, raw_ostream::GREEN, সত্য, অসত্য, Mode);

    // Print out the caret line, matching tabs in the source line.
    প্রতি (unsigned i = 0, e = Caret_সারি.size(), OutCol = 0; i != e; ++i) {
      যদি (i >= সারি_Contents.size() || সারি_Contents[i] != '\t') {
        S << Caret_সারি[i];
        ++OutCol;
        অনুবর্ত;
      }

      // Okay, we have a tab.  Insert the appropriate number of characters.
      কুরু {
        S << Caret_সারি[i];
        ++OutCol;
      } যাবত্ ((OutCol % TabStop) != 0);
    }
    S << '\n';
  }

  // Print out the replacement line, matching tabs in the source line.
  যদি (Fix_It_Insertion_সারি.empty())
    প্রত্যাগম;

  প্রতি (size_t i = 0, e = Fix_It_Insertion_সারি.size(), OutCol = 0; i < e; ++i) {
    যদি (i >= সারি_Contents.size() || সারি_Contents[i] != '\t') {
      OS << Fix_It_Insertion_সারি[i];
      ++OutCol;
      অনুবর্ত;
    }

    // Okay, we have a tab.  Insert the appropriate number of characters.
    কুরু {
      OS << Fix_It_Insertion_সারি[i];
      // FIXME: This is trying not to break up replacements, but then to re-sync
      // with the tabs between replacements. This will fail, though, if two
      // fix-it replacements are exactly adjacent, or if a fix-it contains a
      // space. Really we should be precomputing column widths, which we'll
      // need anyway for multibyte chars.
      যদি (Fix_It_Insertion_সারি[i] != ' ')
        ++i;
      ++OutCol;
    } যাবত্ (((OutCol % TabStop) != 0) && i != e);
  }
  OS << '\n';
}
